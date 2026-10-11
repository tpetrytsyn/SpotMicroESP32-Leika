# ASW-07D-2 illuminated toggle switch

> **Not used.** The main switch is the [KCD2-201N-B rocker](../KCD2-201N-B-rocker-switch.md). This
> is the better part electrically — it carries a DC rating — kept as the swap if the rocker's
> cutout doesn't fit the shell or the pack moves to 3S.

Sourced as
["Тумблер 3 контакти ON-OFF з зеленим з підсвічуванням 12VDC ASW-07D-2 Daier"](https://www.rcscomponents.kiev.ua/product/tumbler-3-kontakty-on-off-z-zelenym-z-pidsvichuvanniam-12vdc-asw-07d-2_207255.html)
(RCS Components), 85 ₴.

| Item | ASW-07D-2 | [KCD2 rocker](../KCD2-201N-B-rocker-switch.md) |
| --- | --- | --- |
| Configuration | SPST ON-OFF toggle, 3 pins | DPST ON-OFF rocker |
| Rating | **30 A / 12 V DC** — optimistic for the size, but DC | 30 A / 250 V AC, no DC figure |
| Lamp | green LED, 12 V — dimmer at 8.4 V, but lit **[inferred]** | probably neon — dark |
| Mounting | **12.2 mm round hole** | 22 × 30.8 mm rectangle |
| Body | 28.3 × 16 × 29.7 mm | 32 × 25 × 36 mm |
| Exposure | the lever can be knocked in a fall | flush |
| Price | 85 ₴ | 35 ₴ |

**Shell fit:** Leika's assembly points at Kubina's SpotMicroESP32 ([2_assembly.md](../../2_assembly.md)),
whose rear cover (`Rear_Cover_Shell_Long`, step 024) has a 19 mm pushbutton hole. The 12.2 mm
bushing fits it with a printed reducer ring or a large washer; the rocker doesn't fit without
reworking the cover. Clearance for the toggle's ~30 mm depth beside the TFT pocket is unchecked.
Shelter the lever, or fit a guard, so a fall can't flip it.

**Wiring, if fitted** — pins usually marked POWER, ACC, GND **[inferred]**:

| Pin | To |
| --- | --- |
| POWER | from the F114-C |
| ACC | to the ACS712 and the rest of the robot |
| GND | star — a spare LIGHT position; LED current only |

**Don't swap POWER and ACC:** the LED sits between ACC and GND, so swapped it stays lit and drains
the pack while the robot is off.
