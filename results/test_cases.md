# Test Cases

> **Note on this document:** the table below lists the test cases used to
> validate firmware logic during development. The "Expected Result" column
> describes designed/expected behaviour. Treat the "Sample Serial Output"
> images in `screenshots/` and `../images/` as **illustrative mock-ups**
> showing the expected log format — replace them with your own captured
> Serial Monitor screenshots from real hardware runs before submitting this
> as a verified results section (e.g., for an academic submission or
> portfolio claim).

| TC # | Test Case | Procedure | Expected Result |
|------|-----------|-----------|------------------|
| TC-01 | Cold boot / module init | Power on the full circuit | Serial Monitor prints boot banner; SIM800L responds `OK` to initial `AT` handshake |
| TC-02 | GPS cold-start fix acquisition | Power on outdoors with clear sky view | `gps.location.isValid()` becomes true within ~30–60s typical NEO-6M cold-start time |
| TC-03 | Inside-geofence detection | Position device within configured radius of geofence centre | Distance ≤ radius; green LED ON; state = INSIDE |
| TC-04 | Outside-geofence detection | Move device beyond configured radius | Distance > radius; red LED ON, buzzer ON; state = OUTSIDE |
| TC-05 | Entry alert (state transition) | Move device from outside → inside the boundary | Exactly **one** SMS sent: "ENTERED geofence" with lat/lon/distance |
| TC-06 | Exit alert (state transition) | Move device from inside → outside the boundary | Exactly **one** SMS sent: "EXITED geofence" with lat/lon/distance |
| TC-07 | Anti-spam suppression | Hold device stationary inside the geofence for several sampling cycles | No repeat SMS sent after the first entry alert; Serial logs "SMS suppressed (anti-spam)" |
| TC-08 | GPS signal loss handling | Block antenna / move indoors mid-test | After `GPS_FIX_TIMEOUT_MS`, Serial logs a "no fix" warning; last known state is held; no false alert sent |
| TC-09 | GSM send failure & retry | Remove SIM / disable network mid-test, trigger a transition | Firmware retries up to `SMS_MAX_RETRIES`, logs each attempt, then logs a failure message without crashing |
| TC-10 | Boundary edge jitter | Position device very close to the radius boundary | Some toggling may occur if GPS accuracy < distance-to-boundary; documented as a known limitation (see Future Enhancements) |
| TC-11 | Power-supply stability under SMS burst | Trigger several SMS sends in sequence | No Arduino reset / brown-out, due to dedicated SIM800L power rail + 1000µF capacitor |

## Recommended real-world test procedure

1. Bench-test indoors near a window first to confirm serial output format and
   SMS delivery, using a large geofence radius (e.g., 500 m) so you stay
   "inside" throughout.
2. Move outdoors with a clear sky view and re-test with your intended
   production radius.
3. Physically walk/drive across the boundary in both directions to confirm
   TC-05 and TC-06.
4. Leave the device powered on, stationary, for at least 10–15 minutes to
   confirm TC-07 (no SMS spam).
