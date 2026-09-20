# SZBK07 300 W CC/CV buck converter

Step-down converter for the servo rail. Sourced as
["Регульований понижуючий 20A перетворювач 300W 6-40V 1.2-36V"](https://www.rcscomponents.kiev.ua/product/rehulovanyi-ponyzhuiuchyi-20a-peretvoriuvach-300w-6-40v-1-2-36v_174587.html)
(rcscomponents.kiev.ua). The PCB silkscreen reads **`SZBK07`** **[photo]**.

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[photo]** = read off the product images, **[listing]** = stated by the seller,
**[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Input voltage | 6 – 40 V | **[listing]** |
| Output voltage | 1.2 – 36 V, adjustable | **[listing]** + **[photo]** |
| Max output current | **15 A or 20 A — the listing states both** | **[listing]**, see below |
| Max power | 300 W | **[listing]** |
| Regulation | **Constant current *and* constant voltage** | **[photo]** — separate CC and CV pots |
| Soft start | Switch, marked `ON/OFF` | **[photo]** |
| Cooling | Two bolt-on aluminium heatsinks | **[photo]** |
| Dimensions | 60 × 53 × 27 mm | **[listing]** |

The seller's title says **20 A** while its own spec table says **15 A**. Treat 15 A as the
dependable figure and 20 A as a peak at best. Neither is reachable without forced airflow.

## Board layout

All **[photo]**, from the vendor's annotated image.

| Terminal / control | Function |
| --- | --- |
| `+IN` / `−IN` | Input, screw terminals (one edge) |
| `OUT +` / `OUT −` | Output, screw terminals (opposite edge) |
| **CC** pot | Current limit |
| **CV** pot | Output voltage |
| `ON/OFF` slide switch | Soft start / output enable |

The two pots sit side by side near the output terminals and are **not labelled on the
board itself** beyond tiny `CC` / `CV` silkscreen — confirm which is which against the
vendor image before turning anything.

## Why CC matters here

This is the feature that makes the module worth using for a servo rail. The CC pot sets a
hard current ceiling **at the rail**, independent of whatever supplies the input.

Twelve TD-8135MG servos can draw 2.6–3.4 A each when stalled
([servo spec](DS3235-TD-8135MG-servo.md)). A mechanical jam, a leg driven into the
chassis, or a bad calibration value that parks a servo against its stop will pull stall
current indefinitely. With CC set, the rail current folds back instead of feeding it.

Set CC deliberately rather than leaving it at maximum. See
[servo-calibration.md](../servo-calibration.md) for bench values.

## Headroom

It is a buck — **output must be below input**, with roughly **1–2 V** of margin
**[inferred]** for the switch and inductor drop. For a 6 V servo rail, the input wants to
be **8 V or more**. There is no boost capability: if the input sags to the output
voltage, regulation is lost and the output follows the input down.

## Setting it up

**Set both pots before connecting any load.** Wire input only, power up, and adjust with
a meter on the unloaded output.

1. Turn **CC fully down** (counter-clockwise) and **CV** to minimum.
2. Apply input, switch the module on.
3. Raise **CV** until the meter reads the target rail voltage.
4. Set **CC** to the current ceiling you want. With no load connected this cannot be read
   directly — the usual method is to short the output *through an ammeter* at low CV and
   adjust until the ammeter shows the target, then restore CV. If that is uncomfortable,
   leave CC near minimum and raise it until the rail stops folding back under real load.
5. Power down, connect the load, power up.

A module delivered with CV near its maximum will put 36 V into whatever is connected.
Assume nothing about the shipped setting.

## Thermal

Two heatsinks, no fan **[photo]**. 300 W is a headline figure that assumes forced air.
For continuous operation above a few amps, give it airflow and check the heatsinks by
hand after a few minutes of load.

At the calibration currents in [servo-calibration.md](../servo-calibration.md) — under
1 A — it runs cold.

## Open items

- [ ] **Real continuous current rating** with the supplied heatsinks and no fan. The
      15 A / 20 A discrepancy is unresolved and neither figure is credible passively.
- [ ] **Exact dropout at 6 V out**, which sets the minimum usable input voltage. The
      1–2 V figure above is **[inferred]** from this class of module.
- [ ] **What the `ON/OFF` switch actually gates** — output enable, or soft-start ramp
      only. Worth knowing before relying on it to isolate the servo rail.
- [ ] **Output ripple** at servo load. Matters because the PCA9685 and servos share this
      rail with their own transients.
