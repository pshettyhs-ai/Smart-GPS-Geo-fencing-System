# Bill of Materials (BOM)

> Costs are approximate (India, hobbyist-grade components) and will vary by
> vendor and quantity. Verify current pricing with your supplier before
> ordering — these figures are for budgeting reference only.

| # | Component | Specification | Qty | Approx. Cost (INR) | Notes |
|---|-----------|----------------|-----|---------------------|-------|
| 1 | Arduino Nano | ATmega328P, USB Mini-B (clone) | 1 | ₹250 – ₹350 | Original Nano costs more |
| 2 | GPS Module | NEO-6M (GY-NEO6MV2 breakout) | 1 | ₹350 – ₹500 | Includes onboard ceramic antenna pad |
| 3 | GSM/GPRS Module | SIM800L (v2, with onboard level/voltage regulator board preferred) | 1 | ₹300 – ₹450 | Choose a board with a built-in 1000µF cap if available |
| 4 | Li-ion Battery | 18650, 3.7V, 2200 mAh | 1 | ₹150 – ₹250 | Use a cell with built-in protection, or pair with TP4056 |
| 5 | Battery Charger/Protection Module | TP4056 with protection (over-charge/over-discharge/short-circuit) | 1 | ₹30 – ₹60 | Critical fix — see Connection_Details.md |
| 6 | Boost Converter | MT3608 (or similar), set to 5.0V output | 1 | ₹35 – ₹60 | Powers Arduino Nano + GPS |
| 7 | Buck/LDO Module | Adjustable buck (e.g., MP1584) set to ~4.0V, 2A-rated | 1 | ₹40 – ₹70 | Dedicated SIM800L supply — critical fix |
| 8 | Electrolytic Capacitor | 1000 µF, 16V, low-ESR | 1 | ₹15 – ₹30 | Placed directly across SIM800L VCC–GND |
| 9 | Battery Holder | 18650 single-cell holder with leads | 1 | ₹20 – ₹40 | |
| 10 | Buzzer | 5V active buzzer | 1 | ₹10 – ₹20 | Outside-geofence audible alert |
| 11 | LED – Green | 5mm, with 220Ω resistor | 1 | ₹5 – ₹10 | Inside-geofence indicator |
| 12 | LED – Red | 5mm, with 220Ω resistor | 1 | ₹5 – ₹10 | Outside-geofence indicator |
| 13 | Resistors | 10kΩ and 20kΩ (voltage divider), 220Ω x2 (LEDs) | 4 | ₹5 – ₹10 | RX/TX level-shifting fix |
| 14 | Breadboard | Half+ size, 830/400 points | 1 | ₹80 – ₹150 | Or use a perfboard for a permanent build |
| 15 | Jumper Wires | Male–Male, assorted | ~20 | ₹40 – ₹60 | |
| 16 | GSM SIM Card | Any 2G-supported network SIM | 1 | (Carrier dependent) | Must support 2G/GPRS — confirm with operator |
| 17 | GPS Active Antenna (optional) | External, u.FL/IPEX connector | 1 | ₹150 – ₹250 | Improves fix time/accuracy indoors |

**Estimated total (excluding SIM card): ₹1,300 – ₹2,000**

## Why the extra power components matter

The original report's design powered the SIM800L from the same rail as the
Arduino, with no dedicated regulation or bulk capacitance. This is one of the
most common failure points in SIM800L-based projects: the module draws current
spikes of up to **~2A** for a few milliseconds while transmitting, and a
shared/under-specified supply causes brown-outs that reset the Arduino or
silently drop the GSM connection. Items 5–8 above specifically address this.
