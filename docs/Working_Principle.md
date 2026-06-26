# Working Principle

## Step-by-step operation

1. **Power-up & initialization**
   The Arduino initializes its two SoftwareSerial ports (GPS and GSM), sets
   pin modes for the LEDs/buzzer, and sends initial `AT` setup commands to
   the SIM800L (echo off, text-mode SMS).

2. **Continuous GPS acquisition**
   The NEO-6M module continuously streams NMEA sentences (e.g., `$GPGGA`,
   `$GPRMC`) over UART. The Arduino feeds every incoming byte to the
   **TinyGPS++** parser, which extracts latitude, longitude, and fix
   validity.

3. **Geofence distance calculation (Haversine formula)**
   Every sampling interval (default: 2 seconds), if a valid fix is
   available, the firmware computes the great-circle distance between the
   device's current position and the configured geofence centre using the
   **Haversine formula**:

   ```text
   a = sin²(Δlat/2) + cos(lat1) · cos(lat2) · sin²(Δlon/2)
   c = 2 · atan2(√a, √(1−a))
   distance = R · c          (R = Earth's mean radius ≈ 6,371,000 m)
   ```

   This is significantly more accurate over real-world distances than a
   flat Euclidean approximation on raw latitude/longitude degrees, especially
   as the geofence radius grows.

4. **Boundary classification**
   If `distance ≤ GEOFENCE_RADIUS_M` → state = **INSIDE**; otherwise →
   state = **OUTSIDE**.

5. **State-change detection (anti-spam logic)**
   The firmware keeps track of the **last state for which an SMS was
   actually sent** (`lastAlertedAs`). A new SMS is only triggered when the
   newly computed state **differs** from `lastAlertedAs` — i.e., exactly once
   per genuine entry or exit event. If the device stays inside (or outside)
   across many consecutive sampling cycles, no repeated SMS is sent. This
   directly fixes a gap in the original report, whose flowchart implied an
   SMS could be sent on every loop iteration.

6. **SMS alert dispatch**
   On a genuine transition, the firmware sends one SMS via SIM800L AT
   commands (`AT+CMGS`), waits for the module's prompt/acknowledgement, and
   retries up to `SMS_MAX_RETRIES` times if the module does not respond —
   this handles transient GSM network errors instead of silently failing.

7. **Local indication**
   - **Inside geofence:** green LED ON, red LED OFF, buzzer OFF.
   - **Outside geofence:** red LED ON, green LED OFF, buzzer ON.

8. **GPS signal-loss handling**
   If no valid fix is received for longer than `GPS_FIX_TIMEOUT_MS`, the
   system logs a warning over Serial and **holds the last known state**
   rather than guessing or falsely declaring a boundary crossing.

9. **Loop**
   Steps 2–8 repeat continuously for the lifetime of the device.

## Why a state machine instead of a simple if/else?

A naive implementation re-evaluates "inside vs outside" every loop and sends
an SMS whenever the condition is true (or whenever it's false). At a 2-second
sampling interval, that would generate **thousands of SMS messages per day**
while the device sits on either side of the boundary — quickly exhausting
SIM balance and overwhelming the recipient. Tracking `lastAlertedAs` and only
alerting on a genuine transition is the standard, correct pattern for
event-driven geofencing and is the core correction this repository makes to
the original report's logic.
