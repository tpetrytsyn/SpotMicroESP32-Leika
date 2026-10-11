# SZBK07 300 W CC/CV buck converter

The servo-rail converter. Sourced as
["Регульований понижуючий 20A перетворювач 300W 6-40V 1.2-36V"](https://www.rcscomponents.kiev.ua/product/rehulovanyi-ponyzhuiuchyi-20a-peretvoriuvach-300w-6-40v-1-2-36v_174587.html)
(rcscomponents.kiev.ua); the PCB reads `SZBK07`.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Input | 6–40 V | listing |
| Output | 1.2–36 V, adjustable | listing + photo |
| Current | **15 A or 20 A** — the listing states both | listing |
| Power | 300 W, assuming forced air | listing |
| Regulation | constant current **and** constant voltage, separate pots | photo |
| Switch | `ON/OFF` slide | photo |
| Cooling | two bolt-on heatsinks, no fan | photo |
| Size | 60 × 53 × 27 mm | listing |

Treat **15 A** as dependable and 20 A as a peak at best — neither without airflow.

Screw terminals: `+IN`/`−IN` on one edge, `OUT+`/`OUT−` on the opposite. The `CC` and `CV` pots
sit side by side near the output, marked only by tiny silkscreen — check the vendor image before
turning either.

## Current limit (CC)

A hard ceiling on the rail, independent of the input. It is **constant-current, not foldback**:
past the limit the output voltage drops and the set current keeps flowing — into any load,
including a short.

- **Covers:** the trunk and the converter, and a jam of several servos.
- **Doesn't cover:** a single servo — one stall (~3 A) is far below the assembled 12 A
  ([wiring.md](../wiring.md#servo-distribution)).
- Settings: 1.5 A for calibration, ~12 A assembled — [wiring.md](../wiring.md#current-limit-cc).

## Headroom

A buck: output must stay ~1.5–2 V below input **[inferred]**, so 6 V out wants 7.5–8 V in. Below
that, the output follows the input down
([wiring.md](../wiring.md#battery-and-headroom)).

## Setting it up

Both pots are multi-turn, and the module ships at an arbitrary setting.

1. Before power, turn **CC** and **CV** fully counter-clockwise.
2. **CC:** meter on its 10 A range across the output, power on, raise CC to the target, power off,
   remove the meter. With CV still low, making and breaking the short doesn't spark.
3. **CV:** meter on DC volts, power on, raise CV to the rail voltage.
4. **Lock CV** with nail varnish: a drifted pot on a vibrating robot puts up to the pack's 8.4 V on
   7.4 V servos.

Setting ~12 A this way exceeds most handheld meters' 10 A range — use a 20 A range within its time
limit, a DC clamp meter, or raise CC from low until the rail stops limiting under real load. This
robot's sequence is [build step 7](../pre-calibration-build.md#7-szbk07-servo-rail-converter).

## Thermal

Cold under 1 A. Above a few amps continuous, give it air, keep it away from the LiPo, and feel the
heatsinks after a few minutes of load.

## Open items

- [ ] Real continuous rating with the stock heatsinks, no fan
- [ ] Dropout at 6 V out — the 1.5–2 V is **[inferred]**;
      [test 12](../pre-calibration-build.md#12-servo-rail-to-the-board) measures it
- [ ] What `ON/OFF` gates: output enable or soft-start only — build step 7 checks it
- [ ] Output ripple at servo load
