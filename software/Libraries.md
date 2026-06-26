# Required Libraries

Install these via **Arduino IDE → Sketch → Include Library → Manage
Libraries...** (or Library Manager search) before compiling
`Smart_GPS_Geofencing.ino`.

| Library | Author | Purpose | Install via |
|---|---|---|---|
| **TinyGPS++** | Mikal Hart | Parses raw NMEA sentences from the NEO-6M into usable latitude/longitude/fix-quality data | Library Manager — search "TinyGPSPlus" |
| **SoftwareSerial** | Arduino (built-in) | Creates two extra virtual UART ports on digital pins, since the Nano's single hardware UART (D0/D1) is reserved for USB/Serial Monitor debugging | Bundled with Arduino IDE — no install needed |

## Why two SoftwareSerial ports?

The Arduino Nano has only **one** hardware UART (pins D0/D1), which is also
used by the USB-to-serial debug connection to your computer. Since this
project needs to talk to **two** other serial devices (the GPS module and the
GSM module) simultaneously, the firmware creates two independent
`SoftwareSerial` instances on spare digital pins:

```cpp
SoftwareSerial gpsSerial(8, 9);   // GPS  : RX=D8, TX=D9
SoftwareSerial gsmSerial(2, 3);   // GSM  : RX=D2, TX=D3
```

> **Limitation to be aware of:** `SoftwareSerial` can only actively listen on
> one port at a time on most AVR boards if both ports receive data
> simultaneously at high baud rates. At the 9600 baud used here, and given
> that the firmware polls `gpsSerial` every loop iteration, this is not an
> issue in practice for this project's update rate. If you need guaranteed
> simultaneous high-throughput reception from both modules, consider an
> Arduino board with multiple hardware UARTs (e.g., Arduino Mega).

## Version notes

This project was built and tested against the Arduino IDE's standard AVR
core for Arduino Nano (ATmega328P, old bootloader). If you are using a Nano
with the new bootloader, select **Tools → Processor → ATmega328P (New
Bootloader)** in the IDE before uploading.
