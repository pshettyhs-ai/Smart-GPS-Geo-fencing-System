# Hardware Connection Details

This document supersedes the power-supply approach shown in the original
mini-project report. It documents the **corrected** wiring used by this
repository — see `Circuit_Diagram.png` and `Block_Diagram.png` for the visual
versions, and `../docs/Architecture.md` for the reasoning behind each fix.

## 1. Pin Connection Table — Signal Lines

| Arduino Nano Pin | Connects To | Module Pin | Direction | Purpose |
|---|---|---|---|---|
| D8 | NEO-6M GPS | TX | GPS → Arduino | Receive NMEA sentences |
| D9 | NEO-6M GPS | RX | Arduino → GPS | Send config commands (optional) |
| D2 | SIM800L | TX | GSM → Arduino | Receive AT command responses |
| D3 | SIM800L (via divider) | RX | Arduino → GSM | Send AT commands / SMS body |
| D5 | Buzzer (+) | — | Arduino → Buzzer | Outside-geofence audible alert |
| D6 | Green LED (anode, via 220Ω) | — | Arduino → LED | Inside-geofence indicator |
| D7 | Red LED (anode, via 220Ω) | — | Arduino → LED | Outside-geofence indicator |
| GND | Common ground bus | GND | — | **Star ground** — all module grounds must meet at one point |

> **Note on D9 → GPS RX:** the NEO-6M continuously streams NMEA sentences on
> its TX pin without needing commands, so wiring D9 is optional for a basic
> build. It is included so the firmware can be extended later (e.g., to send
> `PUBX` configuration commands to change the update rate).

## 2. Pin Connection Table — Power Lines

| Source | Destination | Voltage | Current Capability | Notes |
|---|---|---|---|---|
| 18650 Li-ion battery (+) | TP4056 module IN+ | 3.0 – 4.2V | — | Raw battery, unregulated |
| TP4056 OUT+ | Boost converter IN | 3.0 – 4.2V | — | Protected output |
| TP4056 OUT+ | Buck/LDO module IN | 3.0 – 4.2V | — | Protected output |
| Boost converter OUT (set to 5.0V) | Arduino Nano 5V pin | 5.0V regulated | ≥ 600mA | Powers Nano logic |
| Boost converter OUT (set to 5.0V) | NEO-6M VCC | 5.0V regulated | ≥ 100mA | GPS module has onboard 3.3V LDO |
| Buck/LDO OUT (set to ~4.0V) | SIM800L VCC | 3.7 – 4.2V regulated | **≥ 2A peak** | Dedicated rail — see critical fix #1 below |
| — | 1000µF/16V capacitor | — | — | Placed directly across SIM800L VCC–GND pins (shortest possible leads) |

## 3. Critical Fixes Applied (vs. the original report)

### Fix 1 — SIM800L cannot share the Arduino's 5V rail
**Problem in original report:** the SIM800L was implied to be powered from
the same supply rail as the Arduino. The SIM800L's recommended supply window
is **3.4V – 4.4V**, and it draws current bursts of up to **~2A** during
network transmission. A 5V rail exceeds its absolute maximum rating, and a
rail shared with the microcontroller cannot supply the burst current without
the whole system browning out or resetting.

**Fix applied:** SIM800L is powered from its own dedicated buck/LDO output
regulated to ~4.0V, sized for ≥2A continuous capability, completely separate
from the Arduino/GPS 5V rail.

### Fix 2 — Missing bulk decoupling capacitor
**Problem:** without local energy storage right at the module, the ~2A,
millisecond-scale current spikes during GSM transmit bursts cause voltage
sag that the regulator cannot respond to fast enough.

**Fix applied:** a **1000µF, 16V, low-ESR electrolytic capacitor** is wired
directly across the SIM800L's VCC and GND pins with the shortest possible
lead length.

### Fix 3 — No level-shifting between 5V Arduino and 3V-logic SIM800L
**Problem:** the SIM800L's UART operates at ~2.8V logic. Driving its RX pin
directly from the Arduino's 5V TX output exceeds the module's input voltage
tolerance and can degrade or damage the module over time.

**Fix applied:** a simple resistive voltage divider (10kΩ / 20kΩ) steps the
Arduino's 5V TX signal down to ~2.8–3V before it reaches the SIM800L RX pin.
(SIM800L TX → Arduino RX needs no divider, since ~2.8V is still read
reliably as a HIGH by the Arduino's input pins.)

### Fix 4 — No battery protection
**Problem:** a bare 18650 cell with no protection circuitry is at risk of
over-discharge, over-charge, or short-circuit damage, and is not safe for
unattended/continuous operation.

**Fix applied:** a **TP4056 module with integrated protection IC** handles
charging and cuts off the cell on over-voltage, under-voltage, and
over-current/short-circuit conditions.

### Fix 5 — Mislabelled module in the original report
**Problem:** Section 2.1 of the original report describes the SIM800L's
functionality but mistakenly refers to it at one point as a "Bluetooth
module."

**Fix applied:** all documentation in this repository consistently and
correctly identifies the SIM800L as a **GSM/GPRS cellular module** — it has
no Bluetooth capability whatsoever; communication with it is over UART using
AT commands, and with the outside world over the 2G cellular network.

## 4. Common Ground Requirement

Every module (Arduino, GPS, GSM, battery negative terminal, regulator
modules) **must share a single common ground reference**. Use a star-ground
layout on the breadboard/perfboard — connect all GND pins to one continuous
rail rather than chaining grounds module-to-module, to avoid ground-loop
noise affecting the GPS and GSM UART lines.
