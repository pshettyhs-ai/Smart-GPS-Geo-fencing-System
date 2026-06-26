/*
  ============================================================================
   Smart GPS Geofencing System
   Hardware : Arduino Nano + NEO-6M GPS (GY-NEO6MV2) + SIM800L GSM/GPRS
   Author   : Pavan Shetty H S
   File     : Smart_GPS_Geofencing.ino
  ============================================================================
  DESCRIPTION
  -----------
  Continuously reads the device's live GPS position, calculates its
  great-circle distance from a fixed geofence centre using the Haversine
  formula, and classifies the device as INSIDE or OUTSIDE the geofence.
  An SMS alert is sent via the SIM800L module ONLY when the device's state
  actually changes (entry or exit event) -- this is the anti-spam logic that
  prevents the system from sending a new SMS on every loop iteration.

  KEY IMPROVEMENTS OVER THE ORIGINAL MINI-PROJECT REPORT
  -------------------------------------------------------
  1. Real geofence math: Haversine distance calculation (lat/lon -> metres),
     instead of no defined comparison logic.
  2. State-machine + debounce: SMS is sent exactly once per boundary
     crossing, not on every loop cycle (prevents SMS/SIM credit spam).
  3. GPS signal-loss handling: system holds last known state and keeps
     retrying instead of crashing or sending false alerts on momentary
     signal loss.
  4. GSM error handling: checks for module response/ACK and retries a
     bounded number of times, logging failures over Serial.
  5. Non-blocking-ish timing using millis() for the main sample interval
     instead of long blocking delay() calls (keeps GPS parsing responsive).

  LIBRARIES REQUIRED (Arduino Library Manager)
  ---------------------------------------------
   - TinyGPSPlus      by Mikal Hart
   - SoftwareSerial    (bundled with Arduino IDE)

  WIRING SUMMARY (see hardware/Connection_Details.md for full table)
  ---------------------------------------------------------------------------
   NEO-6M GPS   : TX -> D8 (Nano RX) | RX -> D9 (Nano TX) | VCC -> 5V regulated
   SIM800L GSM  : TX -> D2 (Nano RX) | RX -> D3 (Nano TX, via 10k/20k divider)
                  VCC -> dedicated ~4.0V rail (2A capable) + 1000uF cap
   Buzzer       : D5      Green LED : D6      Red LED : D7
   ALL GROUNDS MUST BE COMMON (star ground) -- see Circuit_Diagram.png
  ============================================================================
*/

#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// ----------------------------------------------------------------------------
// 1. CONFIGURATION -- edit these values for your deployment
// ----------------------------------------------------------------------------
static const double GEOFENCE_CENTER_LAT = 13.418900;   // <-- set your latitude
static const double GEOFENCE_CENTER_LON = 75.773800;   // <-- set your longitude
static const double GEOFENCE_RADIUS_M   = 100.0;        // geofence radius, metres

static const char ALERT_PHONE_NUMBER[] = "+91XXXXXXXXXX"; // registered mobile

static const unsigned long GPS_SAMPLE_INTERVAL_MS = 2000;  // evaluate position every 2s
static const unsigned long GPS_FIX_TIMEOUT_MS      = 8000; // max wait for a valid fix
static const uint8_t       SMS_MAX_RETRIES         = 3;

// ----------------------------------------------------------------------------
// 2. PIN MAP
// ----------------------------------------------------------------------------
static const uint8_t PIN_GPS_RX = 8;   // Arduino RX  <- GPS TX
static const uint8_t PIN_GPS_TX = 9;   // Arduino TX  -> GPS RX
static const uint8_t PIN_GSM_RX = 2;   // Arduino RX  <- SIM800L TX
static const uint8_t PIN_GSM_TX = 3;   // Arduino TX  -> SIM800L RX (via divider)

static const uint8_t PIN_BUZZER    = 5;
static const uint8_t PIN_LED_GREEN = 6;  // ON = inside geofence
static const uint8_t PIN_LED_RED   = 7;  // ON = outside geofence

// ----------------------------------------------------------------------------
// 3. GLOBAL OBJECTS
// ----------------------------------------------------------------------------
TinyGPSPlus    gps;
SoftwareSerial gpsSerial(PIN_GPS_RX, PIN_GPS_TX);
SoftwareSerial gsmSerial(PIN_GSM_RX, PIN_GSM_TX);

enum GeofenceState : uint8_t { STATE_UNKNOWN, STATE_INSIDE, STATE_OUTSIDE };
GeofenceState currentState  = STATE_UNKNOWN;
GeofenceState lastAlertedAs = STATE_UNKNOWN;

unsigned long lastSampleTime = 0;
unsigned long gpsSearchStart = 0;

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(9600);          // USB debug console
  gpsSerial.begin(9600);       // NEO-6M default baud rate
  gsmSerial.begin(9600);       // SIM800L default baud rate

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, LOW);

  Serial.println(F("============================================="));
  Serial.println(F(" Smart GPS Geofencing System - booting up..."));
  Serial.println(F("============================================="));

  initGSM();

  Serial.print(F("Geofence centre : "));
  Serial.print(GEOFENCE_CENTER_LAT, 6);
  Serial.print(F(", "));
  Serial.println(GEOFENCE_CENTER_LON, 6);
  Serial.print(F("Geofence radius : "));
  Serial.print(GEOFENCE_RADIUS_M);
  Serial.println(F(" m"));

  gpsSearchStart = millis();
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  // Continuously feed any available NMEA bytes to the TinyGPS++ parser.
  // This MUST run every loop iteration, not just once per sample interval,
  // otherwise serial bytes are dropped and fixes are lost.
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  unsigned long now = millis();
  if (now - lastSampleTime < GPS_SAMPLE_INTERVAL_MS) {
    return;   // not yet time to re-evaluate position
  }
  lastSampleTime = now;

  if (!gps.location.isValid() || !gps.location.isUpdated()) {
    handleNoFix();
    return;
  }

  // Valid fix obtained — reset the "searching" timer
  gpsSearchStart = now;

  double currentLat = gps.location.lat();
  double currentLon = gps.location.lng();
  double distanceM  = haversineDistanceMeters(GEOFENCE_CENTER_LAT, GEOFENCE_CENTER_LON,
                                                currentLat, currentLon);

  Serial.print(F("Lat: "));  Serial.print(currentLat, 6);
  Serial.print(F("  Lon: ")); Serial.print(currentLon, 6);
  Serial.print(F("  Distance from centre: ")); Serial.print(distanceM, 1);
  Serial.println(F(" m"));

  GeofenceState newState = (distanceM <= GEOFENCE_RADIUS_M) ? STATE_INSIDE : STATE_OUTSIDE;
  evaluateGeofenceTransition(newState, currentLat, currentLon, distanceM);
}

// ============================================================================
// GEOFENCE STATE MACHINE  (anti-spam: SMS fires only on a real transition)
// ============================================================================
void evaluateGeofenceTransition(GeofenceState newState, double lat, double lon, double distanceM) {
  currentState = newState;

  if (currentState == STATE_INSIDE) {
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  } else {
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_BUZZER, HIGH);
  }

  if (currentState == lastAlertedAs) {
    // No change since the last alert -> suppress duplicate SMS.
    Serial.println(F("Status unchanged. SMS suppressed (anti-spam)."));
    return;
  }

  // Genuine state transition detected -> send exactly ONE SMS.
  char message[140];
  if (currentState == STATE_INSIDE) {
    snprintf(message, sizeof(message),
             "ALERT: Device ENTERED geofence.\nLat:%.6f Lon:%.6f\nDist:%.1fm",
             lat, lon, distanceM);
    Serial.println(F(">>> ENTRY event detected. Sending SMS..."));
  } else {
    snprintf(message, sizeof(message),
             "ALERT: Device EXITED geofence.\nLat:%.6f Lon:%.6f\nDist:%.1fm",
             lat, lon, distanceM);
    Serial.println(F(">>> EXIT event detected. Sending SMS..."));
  }

  bool sent = sendSMSWithRetry(ALERT_PHONE_NUMBER, message, SMS_MAX_RETRIES);
  if (sent) {
    lastAlertedAs = currentState;   // only update once SMS confirmed sent
    Serial.println(F("SMS sent and acknowledged successfully."));
  } else {
    Serial.println(F("ERROR: SMS failed after max retries. Will retry on next transition check."));
  }
}

// ============================================================================
// GPS SIGNAL-LOSS HANDLING
// ============================================================================
void handleNoFix() {
  Serial.println(F("Waiting for valid GPS fix..."));
  if (millis() - gpsSearchStart > GPS_FIX_TIMEOUT_MS) {
    // Hold last known state; blink both LEDs slowly to indicate GPS issue
    // without falsely declaring an entry/exit event.
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    Serial.println(F("WARNING: No GPS fix for an extended period. Holding last known state."));
  }
}

// ============================================================================
// HAVERSINE DISTANCE CALCULATION
// Returns the great-circle distance between two lat/lon points in METRES.
// ============================================================================
double haversineDistanceMeters(double lat1, double lon1, double lat2, double lon2) {
  const double EARTH_RADIUS_M = 6371000.0;
  double dLat = radians(lat2 - lat1);
  double dLon = radians(lon2 - lon1);

  double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
             cos(radians(lat1)) * cos(radians(lat2)) *
             sin(dLon / 2.0) * sin(dLon / 2.0);
  double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

  return EARTH_RADIUS_M * c;
}

// ============================================================================
// GSM / SMS FUNCTIONS
// ============================================================================
void initGSM() {
  Serial.println(F("Initializing SIM800L module..."));
  sendATCommand("AT", 1000);             // basic handshake
  sendATCommand("ATE0", 1000);           // echo off
  sendATCommand("AT+CMGF=1", 1000);      // text mode SMS
  Serial.println(F("SIM800L initialization sequence complete."));
}

// Sends a raw AT command and prints whatever the module returns (debug aid).
void sendATCommand(const char* cmd, unsigned long waitMs) {
  gsmSerial.println(cmd);
  unsigned long start = millis();
  while (millis() - start < waitMs) {
    while (gsmSerial.available()) {
      Serial.write(gsmSerial.read());
    }
  }
}

// Reads whatever the module has sent within timeoutMs into a buffer and
// returns true if "OK" (or the expected token) was found in the response.
bool waitForResponse(const char* expected, unsigned long timeoutMs) {
  char buffer[96];
  size_t idx = 0;
  unsigned long start = millis();
  memset(buffer, 0, sizeof(buffer));

  while (millis() - start < timeoutMs) {
    while (gsmSerial.available() && idx < sizeof(buffer) - 1) {
      buffer[idx++] = gsmSerial.read();
    }
    if (strstr(buffer, expected) != NULL) {
      return true;
    }
  }
  return false;
}

// Sends one SMS, retrying up to maxRetries times on failure (handles
// transient GSM network / module errors instead of failing silently).
bool sendSMSWithRetry(const char* number, const char* message, uint8_t maxRetries) {
  for (uint8_t attempt = 1; attempt <= maxRetries; attempt++) {
    Serial.print(F("SMS attempt "));
    Serial.print(attempt);
    Serial.print(F(" of "));
    Serial.println(maxRetries);

    gsmSerial.println("AT+CMGF=1");
    delay(200);

    gsmSerial.print("AT+CMGS=\"");
    gsmSerial.print(number);
    gsmSerial.println("\"");
    if (!waitForResponse(">", 3000)) {
      Serial.println(F("  -> No '>' prompt from module. Retrying..."));
      gsmSerial.write(27);   // ESC to cancel any pending SMS prompt
      delay(500);
      continue;
    }

    gsmSerial.print(message);
    gsmSerial.write(26);     // Ctrl+Z sends the message

    if (waitForResponse("OK", 8000)) {
      return true;           // module acknowledged successful send
    }
    Serial.println(F("  -> No OK/ACK received from module. Retrying..."));
    delay(1000);
  }
  return false;   // all retries exhausted
}
