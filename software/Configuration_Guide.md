# Configuration Guide

Before uploading `Smart_GPS_Geofencing.ino`, edit the configuration block at
the top of the file:

```cpp
static const double GEOFENCE_CENTER_LAT = 13.418900;   // <-- set your latitude
static const double GEOFENCE_CENTER_LON = 75.773800;   // <-- set your longitude
static const double GEOFENCE_RADIUS_M   = 100.0;       // geofence radius, metres

static const char ALERT_PHONE_NUMBER[] = "+91XXXXXXXXXX"; // registered mobile
```

## 1. Finding your geofence centre coordinates

1. Open Google Maps (or any mapping app) on your phone or browser.
2. Long-press / right-click the exact spot you want as the geofence centre.
3. Copy the latitude and longitude shown (e.g., `13.4189, 75.7738`).
4. Paste them into `GEOFENCE_CENTER_LAT` and `GEOFENCE_CENTER_LON`.

## 2. Choosing a geofence radius

- Start with **50–100 m** for testing — consumer GPS modules like the
  NEO-6M typically have a horizontal accuracy of **2.5–5 m** under open sky,
  but accuracy degrades significantly indoors or near tall buildings/trees.
- A radius that is too small relative to GPS accuracy will cause false
  entry/exit toggling near the boundary. If you notice repeated alerts while
  stationary near the edge, increase the radius or add hysteresis (see
  "Future Enhancements" in the main README).

## 3. Setting the alert phone number

- Use the international format with country code, e.g. `+919876543210` for
  India.
- Make sure the SIM inserted in the SIM800L is active, has SMS balance/plan,
  and has **2G network coverage** in your deployment area (SIM800L is a
  2G/GPRS-only module — it will not register on a 3G/4G/5G-only SIM or in an
  area with no 2G coverage).

## 4. Adjusting timing parameters

```cpp
static const unsigned long GPS_SAMPLE_INTERVAL_MS = 2000;  // evaluate every 2s
static const unsigned long GPS_FIX_TIMEOUT_MS      = 8000; // max wait for a fix
static const uint8_t       SMS_MAX_RETRIES         = 3;
```

- `GPS_SAMPLE_INTERVAL_MS`: how often the firmware evaluates the geofence
  condition. Lower values give faster detection but increase processing/SMS
  overhead if near the boundary.
- `GPS_FIX_TIMEOUT_MS`: how long to wait without a valid fix before logging a
  "no fix" warning over Serial.
- `SMS_MAX_RETRIES`: number of attempts before giving up on a single SMS send
  (handles transient GSM network errors).

## 5. Uploading the code

1. Connect the Arduino Nano to your computer via USB.
2. Open `Smart_GPS_Geofencing.ino` in the Arduino IDE.
3. Install the libraries listed in `Libraries.md`.
4. Select **Tools → Board → Arduino Nano** and the correct **Processor**
   variant (old/new bootloader) and **Port**.
5. **Important:** disconnect the SIM800L's RX/TX wires from the Arduino
   before uploading — they share pins that can interfere with the upload
   process; reconnect them after a successful upload. (Since this firmware
   uses SoftwareSerial pins D2/D3/D8/D9 rather than the hardware UART, this
   is usually not required, but is good practice if you experience upload
   failures.)
6. Click **Upload**.
7. Open **Tools → Serial Monitor** at **9600 baud** to view live GPS
   coordinates, distance-from-centre readings, and SMS status logs.
