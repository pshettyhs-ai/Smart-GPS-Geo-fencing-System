# System Architecture

## Scope of this implementation

The original mini-project report mixes two different architectural ideas in
places: (1) a standalone **embedded hardware** geofencing device (Arduino +
GPS + GSM, sending SMS alerts directly with no internet dependency), and (2)
descriptions of a **cloud/web-backend** geofencing architecture (Node.js /
Python APIs, mobile/web frontend, cloud database) more typical of a
commercial LBS (location-based services) platform.

**This repository documents and implements only the embedded hardware
system that was actually built** — Arduino Nano + NEO-6M GPS + SIM800L GSM,
communicating entirely over SMS with no server, app, or cloud component.
The cloud/app-based description in the literature survey section represents
the broader category of geofencing systems surveyed in research (and is
preserved in `Literature_Survey.md` for academic completeness), but it is
**not** part of this project's actual deliverable and is intentionally
excluded from the hardware/firmware documentation to avoid misrepresenting
the build.

## High-level architecture

```text
GPS Satellites
      |
      v
NEO-6M GPS Module  ---- NMEA over UART ---->  Arduino Nano
                                                   |
                                                   |  Haversine distance calc
                                                   |  vs. geofence boundary
                                                   v
                                          SIM800L GSM/GPRS Module
                                                   |
                                                   | SMS over 2G cellular network
                                                   v
                                          Registered Mobile Number
```

See `../hardware/Block_Diagram.png` for the full diagram including the
corrected power subsystem, and `../hardware/Circuit_Diagram.png` for the
wiring-level view.

## Why this architecture

| Design choice | Reasoning |
|---|---|
| SMS instead of internet/app | No dependency on Wi-Fi/mobile data coverage at the tracked location — only 2G cellular signal is required, which has wider rural coverage than data networks in many regions |
| Arduino Nano (vs. ESP32/ESP8266) | Project requirement was a minimal, low-power, breadboard-friendly microcontroller; no Wi-Fi/cloud connectivity was needed since alerts are SMS-based |
| Local geofence calculation (on-device) | No server round-trip latency; the device can alert even with zero internet connectivity, anywhere it has GSM signal |
| State-machine SMS triggering | Prevents SMS/SIM-credit spam — see `Working_Principle.md` |

## Layered view

1. **Sensing layer** — NEO-6M GPS module acquires satellite fixes and streams
   NMEA sentences.
2. **Processing layer** — Arduino Nano parses NMEA data (TinyGPS++),
   computes distance from the geofence centre (Haversine formula), and runs
   the entry/exit state machine.
3. **Communication layer** — SIM800L sends SMS alerts over the GSM network
   using AT commands.
4. **Indication layer** — onboard green/red LEDs and a buzzer give immediate
   local visual/audible feedback without needing a phone nearby.
5. **Power layer** — Li-ion battery + protection + dual regulated rails (5V
   for logic, dedicated ~4V/2A for the GSM module) — see
   `../hardware/Connection_Details.md`.
