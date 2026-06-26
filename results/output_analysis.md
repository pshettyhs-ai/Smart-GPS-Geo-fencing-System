# Output Analysis

> **Note:** the sample outputs below illustrate the firmware's expected
> Serial Monitor log format (see `screenshots/` and `../images/output_1.jpg`,
> `output_2.jpg`). They are mock-ups generated for documentation purposes,
> not captured logs from a verified field test. Before using this section as
> evidence of working hardware (e.g., in an academic submission, internship
> portfolio, or resume claim), replace these images with screenshots captured
> from your own Serial Monitor during an actual hardware run.

## 1. GPS Lock Output (sample format)

```text
=============================================
 Smart GPS Geofencing System - booting up...
=============================================
Initializing SIM800L module...
SIM800L initialization sequence complete.
Geofence centre : 13.418900, 75.773800
Geofence radius : 100.00 m
Waiting for valid GPS fix...
Waiting for valid GPS fix...
Lat: 13.419042  Lon: 75.773655  Distance from centre: 18.7 m
```

## 2. Inside-Geofence Output (sample format)

```text
Lat: 13.419042  Lon: 75.773655  Distance from centre: 18.7 m
>>> ENTRY event detected. Sending SMS...
SMS attempt 1 of 3
SMS sent and acknowledged successfully.
```

Status: **INSIDE** — Green LED ON, buzzer OFF.

## 3. Outside-Geofence Output (sample format)

```text
Lat: 13.420510  Lon: 75.775820  Distance from centre: 287.3 m
>>> EXIT event detected. Sending SMS...
SMS attempt 1 of 3
SMS sent and acknowledged successfully.
```

Status: **OUTSIDE** — Red LED ON, buzzer ON.
SMS sent: **YES** (single SMS for this transition only — see anti-spam logic
in `../docs/Working_Principle.md`).

## 4. Anti-spam Suppression Output (sample format)

```text
Lat: 13.419050  Lon: 75.773661  Distance from centre: 19.4 m
Status unchanged. SMS suppressed (anti-spam).
```

## Analysis

- **Distance accuracy** depends primarily on the NEO-6M's raw GPS accuracy
  (typically 2.5–5 m open-sky) and is computed using the Haversine
  great-circle formula rather than a flat-Earth approximation, so accuracy
  scales correctly even for larger geofence radii.
- **SMS volume** is bounded to one message per genuine boundary crossing,
  directly addressing the spam risk of a naive per-loop implementation.
- **Failure recovery**: GPS signal loss and GSM send failures are logged and
  retried/held gracefully rather than causing silent failures or false
  alerts (see `test_cases.md`, TC-08 and TC-09).
