# LM2596S CC/CV buck module (3-potentiometer charger variant)

Step-down converter for the ESP32's 5 V rail. Sourced as
["Понижуючий перетворювач LM2596S"](https://prom.ua/ua/m1440220616950486746-ponizhayuschij-preobrazovatel-lm2596s.html?p=2826593351)
(prom.ua). Regulator is an **LM2596S-ADJ** **[photo]**.

> **Not the documented choice.** The 5 V rail uses the fixed-output
> [CN3903](../CN3903-5V-buck-module.md) instead — nothing to misadjust and nothing to knock
> out of alignment on a moving robot. This module is kept as the adjustable alternative,
> and remains useful wherever a non-5 V rail is needed.

This is the **CC/CV charger variant** with three trimmers and three LEDs — not the common
single-pot LM2596 board. The extra controls change how it is set up, and one of them can
brown out the load if set wrongly.

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](../ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[photo]** = read off the product images, **[listing]** = stated by the seller,
**[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Regulator | LM2596S-ADJ | **[photo]** |
| Input voltage | 5 – 35 V, must exceed output | **[listing]** |
| Output voltage | 1.25 – 30 V, adjustable | **[listing]** |
| Max output current | 3 A — heatsink advised above 15 W | **[listing]** |
| Efficiency | up to 92% | **[listing]** |
| Regulation | Constant current **and** constant voltage | **[listing]** + **[photo]** |
| Protection | Overcurrent | **[listing]** |
| Operating temperature | −40 … +85 °C | **[listing]** |
| Dimensions | 48 × 23 mm | **[listing]** |

Passive components **[photo]**: 330 µH shielded inductor, 2 × 220 µF 35 V, SK54 Schottky.

## Board layout

| Marking | Function |
| --- | --- |
| `IN+` / `IN−` | Input screw terminal, one end of the board |
| *(opposite end)* | Output screw terminal — **verify polarity with a meter**, the silkscreen is not legible in the vendor photos |
| `OK` LED | Charge complete / threshold reached |
| `CH` LED | Charging |
| `CC/CV` LED | Indicates which regulation mode is active |
| 3 × blue trimmers | Voltage, current, and LED threshold — **see below** |

### The pots are not labelled by function

The three trimmers are marked only with resistance codes — `103`, `104` and `W103`
(10 kΩ, 100 kΩ, 10 kΩ) **[photo]**. Nothing on the board says which is CV, which is CC,
and which sets the indicator threshold. Vendor listings for this design disagree about the
ordering, so **identify them empirically** rather than trusting any diagram:

1. Apply input only. No load. Meter on the output.
2. Turn each trimmer a few turns in turn. **The one that moves the unloaded output voltage
   is CV.** Set it aside — that is the one you want at 5.0 V.
3. The other two do nothing without a load. To find **CC**, connect a modest load and
   adjust each: the one that makes the output voltage droop as you turn it down is the
   current limit.
4. Whichever remains is the **LED threshold**. It affects only the indicator and can be
   left alone.

All three are multi-turn — expect several turns before anything moves.

## The CC pot can brown out the ESP32

This is the one behaviour that matters and that a plain LM2596 board does not have.

In constant-current mode the module behaves as a **current source**: once the load draws
more than the CC setting, the output voltage drops to whatever keeps current at the limit.
That is correct for charging a battery and wrong for powering a microcontroller.

If CC is left near minimum, the 5 V rail sags the moment the ESP32 draws normal current —
Wi-Fi transmit bursts and camera initialisation are the obvious triggers — and the board
browns out for reasons that look nothing like a power-supply fault.

**Set CC comfortably above the load**, around **1 A** for an ESP32-S3 with camera and
Wi-Fi (peaks are a few hundred mA). The `CC/CV` LED shows which mode it is in: it should
sit in CV during normal operation, never CC.

## If used for the 5 V rail

Should you fall back to this module instead of the CN3903, from the shared 2S battery rail
([servo-calibration.md](../../servo-calibration.md)):

| | |
| --- | --- |
| Input | 6.0 – 8.4 V (2S) — inside 5–35 V, but only ~1–3.4 V of headroom over 5 V |
| Output | 5.0 V |
| Load | ESP32-S3 with camera, a few hundred mA |
| Power | ~2.5 W against a 3 A / 15 W rating — **no heatsink needed** |

The module is running at a small fraction of its capability here, which is the right place
for it to be.

## Open items

- [ ] **Which trimmer is which** — identify on the bench by the procedure above and
      label the board with a marker so it need not be rediscovered.
- [ ] **Output terminal polarity**, not legible in the vendor photos. Meter it before
      connecting the ESP32.
- [ ] **Whether `OK`/`CH` LEDs are meaningful outside charging use**, or simply reflect
      the threshold pot. Cosmetic either way.
