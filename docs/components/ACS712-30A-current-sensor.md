# ACS712 30 A current sensor module

Hall-effect sensor in the pack `+` wire, read by the [ADS1115](ADS1115-16bit-ADC.md).
**Chosen, not yet bought.** Sourced as
["Модуль датчика струму ACS712 30 A"](https://www.rcscomponents.kiev.ua/product/modul-datchyka-strumu-acs712-30-a_103100.html)
(RCS Components), 70 ₴.

Chosen over re-shunting the INA226 already on hand: it needs no board modification and the
ADS1115 was already owned. The trade is accuracy — see [Accuracy](#accuracy).

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[listing]** = stated by the seller, **[datasheet]** = Allegro ACS712 datasheet for the
`ELCTR-30A-T` variant, **[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Range | ±30 A | **[listing]** |
| Supply | 5 V (4.5 – 5.5 V) | **[listing]** + **[datasheet]** |
| Sensitivity | **66 mV/A** | **[datasheet]** — the seller does not state it |
| Zero-current output | `VCC / 2` — **ratiometric**, 2.50 V at 5.00 V | **[datasheet]** |
| Conductor resistance | ~1.2 mΩ | **[datasheet]** |
| Isolation | Current path isolated from the signal side | **[datasheet]** |
| Connections | 2-pole screw terminal (current); header `VCC`, `OUT`, `GND` | **[listing]** + **[inferred]** |

> **The variant is inferred from "30 A".** Read the chip marking — it should end in
> `30A` — before trusting 66 mV/A; the 5 A and 20 A versions use 185 and 100 mV/A.

## Use in this project

**In the pack `+` wire, after the F114-C fuse, before the split** to the two converters, so
it sees the whole robot's draw. The [rocker switch](KCD2-201N-B-rocker-switch.md) sits
between the fuse and this sensor.

| Pin | To |
| --- | --- |
| `IP−` | fuse side (from the pack) — **reversed**, see below |
| `IP+` | split to SZBK07 and the F117-C |
| `VCC` | CN3903 5 V |
| `GND` | sensor ground (the ADS1115's `GND`) |
| `OUT` | ADS1115 `A1`, **through 10 kΩ** |

### Mount it reversed

Forward, the output rises from 2.5 V at 66 mV/A and reaches **3.29 V at 12 A** — the
ADS1115's input limit on a 3.3 V supply. Reversed, pack current reads negative and the
output falls instead:

| Pack current | Output, reversed |
| --- | --- |
| 0 A | 2.50 V |
| 12 A — the CC ceiling | 1.71 V |
| 30 A — sensor full scale | 0.52 V |

Firmware negates the sign. This is the orientation Kubina's SpotMicroESP32 uses for the
same reason.

### The 10 kΩ series resistor

The sensor runs on 5 V and can drive its output well above 3.3 V — reversed-direction
current, or the ADS1115 unpowered while the ACS712 is not. 10 kΩ in series limits the
current into the ADS1115's input protection to well under its limit **[inferred]**, at a
negligible error against its megohm input.

### Terminals

The screw terminal takes 14 AWG, but a screw is the connection that loosens under
vibration. Use **crimped ferrules**, not tinned ends — solder creeps under screw pressure
and the joint goes loose. Strain-relieve both wires so the terminal carries no load.

## Accuracy

Expect **±0.1 – 0.2 A** in practice **[inferred]** — fine for "the robot draws 4 A" and for
spotting a stall, not for coulomb counting.

- **The zero point is ratiometric.** If the CN3903 delivers 4.95 V rather than 5.00 V, zero
  moves by 25 mV — **0.4 A**. Measuring the 5 V rail on ADS1115 `A3` lets firmware take
  zero as `V5 / 2`.
- **There is no true zero in service.** The sensor sits before the ESP32's own branch, so
  it never sees 0 A while the firmware is running. Calibrate offset and gain once on the
  bench against a meter in series, and store them.
- **Noise:** average several samples per reading.

## Open items

- [ ] **Read the chip marking** to confirm the 30 A variant.
- [ ] **Bench calibration** of offset and gain against a reference meter.
- [ ] **Walking draw** — the measurement the SZBK07's CC setting is waiting for.
