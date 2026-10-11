# CN3903 fixed 5 V / 3 A buck module

The ESP32's 5 V supply. Sourced as
["Понижуючий перетворювач CN3903 5V 3A"](https://myproject.com.ua/ponyzhuiuchyi-peretvoriuvach-cn3903-5v-3a.html)
(myproject.com.ua).

**Fixed output, nothing to adjust** — chosen over the adjustable
[LM2596S](alternatives/LM2596S-CC-CV-module.md) because a trimmer drifts on a vibrating robot (a
quarter turn could put 8 V into the ESP32), and that module's CC pot browns out the load.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Output | **5 V, fixed** | listing |
| Input | 5.5–30 V (≤ 28 V recommended) | listing |
| Current | 3 A peak, **1.6–1.8 A continuous** | listing |
| Efficiency | up to 96 % | listing |
| Topology | synchronous buck | listing + inferred |
| Temperature | −40…+85 °C | listing |
| Size | 17.5 × 12 × 4.5 mm | listing |
| Connections | solder pads `IN+`, `IN−`, `OUT+`, `OUT−` | listing + photo |

SO-8 regulator, 4.7 µH shielded inductor, all-ceramic capacitors: a high switching frequency and
low ripple **[inferred]**. The chip's markings (`GKE0E`/`GMC2X`) are lot codes, so nothing confirms
it's really a CN3903 — the figures are vendor claims; don't design against a datasheet.

## Use in this project

| | |
| --- | --- |
| Input | 6.0–8.4 V (2S) through the 2 A fuse — inside 5.5–30 V throughout |
| Output | 5.0 V → ESP32 `VCC` and ACS712 `VCC` |
| Load | a few hundred mA, ~¼ of the conservative rating; ~0.1 W dissipated |
| Discharge | holds 5 V down to ~5.5 V in, outlasting the servo rail |

`IN+` ← F117-C; `IN−` and `OUT−` → star (LIGHT 1); `OUT+` → ESP32 and ACS712. Solder the pads and
strain-relieve the wires so the pads carry no load.

**Meter 5.0 V unloaded before the ESP32 sees it** — a fixed module built with the wrong divider
can't be corrected ([build step 8](../pre-calibration-build.md#8-logic-branch-f117-c-and-cn3903)).

## Open items

- [ ] Confirm 5.0 V on the bench
- [ ] The actual regulator part
- [ ] Ripple under Wi-Fi transmit bursts, if brownouts appear
