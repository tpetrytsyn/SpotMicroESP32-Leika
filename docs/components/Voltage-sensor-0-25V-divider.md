# Voltage sensor module, 0–25 V (÷5 divider)

A plain resistor divider on a board, scaling pack voltage for the
[ADS1115](ADS1115-16bit-ADC.md). One is used, for the whole pack. **Chosen, not yet
bought.** Sourced as
["Датчик напруги 0-25 В Voltage Sensor"](https://prom.ua/ua/p2909441539-datchik-naprugi-voltage.html)
(prom.ua), 25 ₴.

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[listing]** = stated by the seller, **[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Input range | 0 – 25 V | **[listing]** |
| Ratio | 1 : 5 | **[listing]** |
| Resistors | 30 kΩ + 7.5 kΩ | **[inferred]** — the usual values; measure |
| Output impedance | ~6 kΩ | **[inferred]**, from the above |
| Connections | Input screw terminal `VCC` / `GND`; output header `S`, `+`, `−` | **[listing]** + **[inferred]** |

**There are no active parts.** The input `GND` and the output `−` are the same copper, and
the output `+` is usually not connected to anything **[inferred]** — only `S` and `−` are
used. That makes the module a divider that **only works on a common ground**, which this
build has.

The 0–25 V range is an Arduino-at-5 V figure. On a 3.3 V ADC the usable range is
0–16.5 V — still double a charged 2S pack.

## Use in this project

| | |
| --- | --- |
| Input `VCC` | the CN3903 branch, **after the F117-C 2 A fuse** |
| Input at full charge | 8.4 V |
| `S` → ADS1115 | `A0`, 1.68 V |
| `−` | sensor ground |
| Draw from the pack | ~0.22 mA — ~9 months to drain 1500 mAh |

### Why it taps after the 2 A fuse

A thin sense wire tapped straight off the 14 AWG trunk sits behind the 15 A fuse only, and
would burn before that fuse opens. Tapping after the F117-C puts it behind 2 A, like the
rest of the CN3903 branch. The cost is the fuse's own drop at ~0.5 A — tens of millivolts,
under 1 % of the reading **[inferred]**, and removed by calibration.

### Calibrate it once

The resistor tolerance is unknown, so the ratio is not exactly 5.000. Measure the input
with a multimeter, read the ADS1115 count, and store the real ratio. The ADS1115's input
impedance (megohms) against the module's ~6 kΩ costs ~0.1 % **[inferred]** — also
absorbed by calibration.

## Per-cell monitoring — not fitted

Pack voltage alone is enough for this build. The balance charger equalises both cells on
every charge, and the 7.0 V cutoff leaves margin for drift: even 0.2 V of imbalance is
3.4 + 3.6 V, above the 3.3 V floor. The charger's per-cell display is the check — cells
more than ~0.1 V apart after a full balance charge mean the pack is ageing.

**Nothing on the robot connects to the balance lead**; it stays for the charger.

If per-cell readings are wanted later — an ageing pack, say — a second module adds them:

- **Input from the balance lead's middle pin only** (cell 1 `+`), to ADS1115 `A2`. Cell 2
  is then `A0 − A2`. The ADS1115 cannot take the middle pin directly: 4.2 V exceeds its
  3.3 V input limit.
- **Leave balance pins 1 and 3 unconnected.** Pin 1 is the pack's `−`, already on the star
  through the XT60; wiring it too puts the balance lead's thin wire in parallel with the
  main `−` lead, carrying a share of the ~11 A return.
- **Put 1 kΩ in the middle-pin wire, right at the connector.** That wire is unfused; the
  resistor limits a pinched-wire short of cell 1 to milliamps **[inferred]**. The ~3 %
  ratio shift calibrates out.
- Needs a JST-XH 3-pin mating connector, and the balance lead must stay unpluggable for
  the charger.

## Open items

- [ ] **Measure the resistor values** and the module's real ratio.
- [ ] **Confirm the output `+` pin is unconnected** on the delivered board.
