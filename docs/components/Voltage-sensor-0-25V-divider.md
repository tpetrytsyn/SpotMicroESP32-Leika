# Voltage sensor module, 0–25 V (÷5 divider)

A resistor divider scaling pack voltage for the [ADS1115](ADS1115-16bit-ADC.md). **Decided, not
bought.** Sourced as
["Датчик напруги 0-25 В Voltage Sensor"](https://prom.ua/ua/p2909441539-datchik-naprugi-voltage.html)
(prom.ua), 25 ₴.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Input range | 0–25 V — an Arduino-at-5 V figure; 0–16.5 V on a 3.3 V ADC | listing |
| Ratio | 1 : 5 | listing |
| Resistors | 30 kΩ + 7.5 kΩ | inferred — measure |
| Output impedance | ~6 kΩ | inferred |
| Connections | input screw terminal `VCC`/`GND`; output header `S`, `+`, `−` | listing + inferred |

No active parts: the input `GND` and output `−` are the same copper, and output `+` is usually not
connected **[inferred]** — only `S` and `−` are used. It works only on a common ground, which this
build has.

## Use in this project

| | |
| --- | --- |
| Input `VCC` | the logic branch, **after the 2 A fuse**, in 22 AWG |
| `S` | ADS1115 `A0` — 1.68 V at 8.4 V |
| `−` | sensor ground (ADS1115 `GND`) |
| Draw | ~0.22 mA, behind the rocker |

**Why after the 2 A fuse:** a thin sense wire tapped off the trunk would sit behind 15 A and burn
before that fuse opened. The 2 A fuse's own drop is tens of millivolts, under 1 %, and calibrated
out **[inferred]**. The trunk's drop under load isn't: ~0.1–0.2 V at 12 A
([wiring.md](../wiring.md#monitoring)).

**Calibrate once:** meter the input, read the ADS1115, store the real ratio — it absorbs resistor
tolerance and the ADC's ~0.1 % loading **[inferred]**.

## Per-cell monitoring — not fitted

Pack voltage is enough: the balance charger equalises the cells each charge, and even 0.2 V of
imbalance at the 7.0 V cutoff is 3.4 + 3.6 V, above the 3.3 V floor. Cells more than ~0.1 V apart
after a balance charge mean the pack is ageing.

If ever added: a second module from the balance lead's **middle pin only** (cell 1 `+`) to `A2`,
cell 2 being `A0 − A2`; 1 kΩ in that wire at the connector, since it's unfused. Pins 1 and 3 stay
unconnected — they duplicate the main leads through a thin wire.

## Open items

- [ ] Measure the resistors and the real ratio
- [ ] Confirm output `+` is unconnected
