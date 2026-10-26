# ACS712 30 A current sensor module

Hall-effect sensor in pack `+`, read by the [ADS1115](ADS1115-16bit-ADC.md).

> **Deferred — not in the initial build.** It monitors, it doesn't protect: nothing acts on its
> reading, and the fuses and CC cover faults without it ([wiring.md](../wiring.md#not-fitted)). The
> wiring below is kept for fitting it later, which means cutting the trunk between the rocker and
> the + split; ADS1115 `A1` stays free for it.

Sourced as
["Модуль датчика струму ACS712 30 A"](https://www.rcscomponents.kiev.ua/product/modul-datchyka-strumu-acs712-30-a_103100.html)
(RCS Components), 70 ₴. Chosen over re-shunting the INA226 on hand — no board modification, and
the ADS1115 was already owned. The trade is accuracy.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Range | ±30 A | listing |
| Supply | 5 V (4.5–5.5 V) | listing + datasheet |
| Sensitivity | **66 mV/A** | datasheet — the seller doesn't state it |
| Zero | `VCC / 2`, **ratiometric** — 2.50 V at 5.00 V | datasheet |
| Conductor resistance | ~1.2 mΩ | datasheet |
| Isolation | current path isolated from the signal side | datasheet |
| Connections | 2-pole screw terminal; header `VCC`, `OUT`, `GND` | listing + inferred |

The 30 A variant is inferred from the name: read the chip marking (ends `30A`) before trusting
66 mV/A — the 5 A and 20 A versions use 185 and 100 mV/A.

## If fitted

After the rocker, before the split, so it sees the whole robot's draw.

| Pin | To |
| --- | --- |
| `IP−` | from the rocker — **reversed**, below |
| `IP+` | to the + split |
| `VCC` | CN3903 5 V |
| `GND` | ADS1115 `GND` (sensor ground) |
| `OUT` | ADS1115 `A1`, **through 10 kΩ** |

**Reversed:** forward, the output would reach 3.29 V at 12 A — the ADS1115's limit. Reversed, it
falls: 2.50 V at 0 A, 1.71 V at 12 A, 0.52 V at 30 A. Firmware negates the sign; Kubina's build
does the same.

**10 kΩ in series:** the sensor runs on 5 V and can drive past 3.3 V — reverse current, or the
ADS1115 unpowered while it is. The resistor limits the input-protection current **[inferred]**, at
negligible error against the ADC's megohm input.

**Terminal:** crimped ferrules, not tinned ends (solder creeps under a screw); strain-relieve both
wires.

## Accuracy

±0.1–0.2 A in practice **[inferred]** — enough for "the robot draws 4 A" or spotting a stall, not
for coulomb counting.

- **The zero is ratiometric:** 4.95 V instead of 5.00 V moves it 25 mV — 0.4 A. The 5 V rail
  through a ÷2 divider (2 × 10 kΩ) on `A3` reads `V5 / 2`, the zero, directly — never undivided:
  5 V exceeds the ADS1115's input limit.
- **No true zero in service** — the ESP32 always draws. Calibrate offset and gain once against a
  meter in series, and store them.
- Average several samples per reading.

## Open items

- [ ] Read the chip marking — the 30 A variant
- [ ] Bench calibration of offset and gain
- [ ] Direction under load: a ~1 A load must make `A1` **fall** ~66 mV per amp; if it rises, swap
      the current wires
