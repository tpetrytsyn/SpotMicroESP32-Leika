# ASW-07D-2 illuminated toggle switch

Main power switch candidate. Sourced as
["Тумблер 3 контакти ON-OFF з зеленим з підсвічуванням 12VDC ASW-07D-2 Daier"](https://www.rcscomponents.kiev.ua/product/tumbler-3-kontakty-on-off-z-zelenym-z-pidsvichuvanniam-12vdc-asw-07d-2_207255.html)
(RCS Components), 85 ₴.

> **Not the documented choice.** The main switch is the
> [KCD2-201N-B rocker](../KCD2-201N-B-rocker-switch.md). This toggle is kept as the
> alternative because it is the **better part electrically** — it carries a DC rating, which
> the rocker does not — and its round hole may suit the shell better. Revisit it if the
> rocker's cutout does not fit, or if the pack ever moves to 3S, where an AC-only rating
> stops being acceptable.

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](../ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[listing]** = stated by the seller, **[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Configuration | SPST ON-OFF toggle, 3 pins | **[listing]** |
| Rating | **30 A / 12 V DC** | **[listing]** — optimistic for the size, but a DC figure |
| Illumination | green LED, 12 V | **[listing]** |
| Mounting hole | **12.2 mm round** | **[listing]** |
| Body | 28.3 × 16 × 29.7 mm | **[listing]** |
| Terminals | tabs with holes | **[listing]** |
| Manufacturer | Daier | **[listing]** |

## Against the rocker

| | [KCD2-201N-B rocker](../KCD2-201N-B-rocker-switch.md) | ASW-07D-2 toggle |
| --- | --- | --- |
| Rating | 30 A / 250 V **AC**, no DC figure | **30 A / 12 V DC** |
| Hole | rectangular 22 × 30.8 mm | round 12.2 mm |
| Light at 8.4 V | probably neon — dark | 12 V LED — dimmer, but lights **[inferred]** |
| Exposure | flush rocker | lever can be knocked in a fall |
| Price | 35 ₴ | 85 ₴ |

### Shell fit

Leika's assembly points at Kubina's SpotMicroESP32 ([2_assembly.md](../../2_assembly.md)),
whose rear cover (`Rear_Cover_Shell_Long`, assembly step 024) has a **round hole for a
19 mm pushbutton**. The toggle's 12.2 mm bushing fits that hole with a printed 19 → 12.2 mm
reducer ring or a large washer under its nut; the rocker's rectangle does not fit it at all
without reworking the cover. Clearance for the toggle's ~30 mm depth beside the cover's
TFT pocket is **unchecked**.

## If it replaces the rocker

The three pins are usually marked **POWER**, **ACC** and **GND** **[inferred]**:

| Pin | To |
| --- | --- |
| POWER | from the F114-C fuse |
| ACC | to the ACS712, and on to the rest of the robot |
| GND | star point — a spare light-block position; LED current only |

**Do not swap POWER and ACC.** The LED sits between ACC and GND. Wired correctly it lights
only when on and draws nothing when off; swapped, it stays lit permanently and drains the
pack while the robot is off.

Mount it with the lever sheltered, or fit a lever guard, so a fall cannot flip it off.
