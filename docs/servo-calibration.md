# Servo calibration — components and wiring

Bench setup for calibrating the twelve leg servos against
[`ServoController`](../esp32/include/peripherals/servo_controller.h). This covers the
**calibration step only** — one or two servos powered at a time, robot not assembled.
Powering all twelve for walking is a different problem, see [Power architecture](#power-architecture).

Confidence is marked throughout, following
[ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[code]** = read from this repo, **[datasheet]** = from the part's spec,
**[inferred]** = reasoned and needs confirmation on hardware.

## Servos

**DS3235 / TD-8135MG** 35 kg digital metal-gear servo, **180° positional**. Full
specifications, sourcing and measurement gaps are in
[DS3235-TD-8135MG-servo.md](components/DS3235-TD-8135MG-servo.md).

The figures this document depends on:

| Item | Value | Note |
| --- | --- | --- |
| Operating voltage | 4.8 – 7.4 V | **[case marking]**; 6 V used throughout |
| Stall current | 2.6 – 3.4 A each | **[datasheet]**, not stated by the seller |
| Running current | 140 – 200 mA | **[datasheet]** |
| Pulse width range | 500 – 2500 µs | **[datasheet]**, not stated by the seller |
| Cable | orange = signal, red = V+, brown = GND | **[photo]** |

> The delivered part is marked `TD-8135MG` on the case, so the datasheet applies directly.
> Reseller figures for it still disagree, and the seller stated none of the electrical
> limits, so pulse range and stall current remain on the measurement list.

> **When ordering more, check the variant.** The same model number is also sold as 360°
> continuous rotation, which has no position feedback and cannot hold a joint. The servos
> in this build are the 180° positional version.

## Components

| # | Item | Why |
| --- | --- | --- |
| 1 | **PCA9685** 16-channel PWM board, I2C | The driver targets `0x40` **[code]** |
| 2 | **Bench supply**, 7.4 V, current-limited | Stands in for a 2S battery; see [Power architecture](#power-architecture) |
| 3 | **[SZBK07](components/SZBK07-buck-converter.md)** 300 W CC/CV buck | battery → 6 V servo rail, with its own current limit |
| 4 | **[CN3903](components/CN3903-5V-buck-module.md)** fixed 5 V buck | battery → 5 V for the ESP32; no adjustment |
| 5 | 3 × jumper wires (SDA, SCL, GND) | ESP32 ↔ PCA9685 logic |
| 6 | 1 × jumper (3V3) | PCA9685 logic supply — **must be 3.3 V**, see [Cautions](#cautions) |
| 7 | **Distribution bus** — terminal block or bus bar, ×2 | 6 V servo bus and the ground star point |
| 8 | **12 × servo extension cables** | Red conductor broken out to the bus; see [Building the harness](#building-the-harness) |
| 9 | Wire for the bus, 18 AWG or heavier | Converter → bus, and bus → star point |
| 10 | **1000 – 2200 µF** electrolytic, ≥ 10 V | Across the 6 V bus, close to the servos |
| 11 | Multimeter | Verify 3.3 V logic and rail voltage *before* connecting |
| 12 | Servo horns + mounting screws | Fitted at calibrated centre |
| 13 | A vice, clamp or jig | Hold the servo while it moves — a 35 kg·cm servo will throw itself around |
| 14 | CH343P USB-serial adapter | Only for reflashing / serial log |

There are now **three** current limits in series: the bench supply's, the SZBK07's CC pot
on the servo rail, and the mechanical one of a servo that simply stalls. Set the first two
deliberately — see [Power architecture](#power-architecture).

## Wiring

**Wire it as the robot will stay wired.** Servo power and ground come from a distribution
bus and never touch the PCA9685, which carries signal only. Calibration then happens on
the final harness, so nothing is rebuilt later and the wiring itself gets tested early.

The bench supply stands in for a **2S pack** — the configuration this project's BOM
specifies ([1_components.md](1_components.md)).

```
   Bench supply 7.4 V   (stands in for 2S: 6.6 – 8.4 V)
   ──────────────────
    + ──────┬──────────────────────────────┐
            │                              │
        SZBK07                          CN3903
        CV = 6.0 V                      fixed 5 V
        CC = set limit                     │
            │                              └──► ESP32 VCC
            │
            │   6 V SERVO BUS
            ├──────┬──────┬────── … ──┐
            │      │      │           │
          servo  servo  servo       servo     ← red  ×12, direct
            │      │      │           │
       1000–2200 µF across the bus, near the servos

    − ──────┬────────────────────────────────────────────┐
            │                                            │
       ★ GROUND STAR POINT                               │
            ├──► servo brown ×12          (heavy return) │
            ├──► PCA9685 GND              (signal ref)   │
            ├──► ESP32 GND                               │
            └──► both converter OUT− ────────────────────┘

   PCA9685                                 ESP32-S3-CAM
   ───────                                 ───────────────
    VCC ◄── 3V3 ───────────────────────────  3V3
    SDA ◄── IO47 ──────────────────────────  IO47
    SCL ◄── IO41 ──────────────────────────  IO41
    GND ◄── star point
    V+  ── NOT CONNECTED
    CH0..CH11 signal ──► servo orange ×12    (signal only)
```

### The three rules that make it work

1. **`V+` stays unconnected.** The PCA9685 IC needs only `VCC`, `GND`, `SDA` and `SCL` —
   it never carries servo current. Leaving `V+` off keeps every amp off the board's thin
   copper, which is the whole point of this topology.
2. **Servo ground goes to the star point, not to the board.** Returning it through the
   PCA9685 would move the bottleneck to the ground trace and achieve nothing.
3. **One wire from the star point to `PCA9685 GND`.** PWM is referenced to ground; without
   it the servos have no valid reference and will jitter or ignore commands entirely. That
   wire carries reference current only, so it can be thin.

### Building the harness

The servos' red wires must leave before the 3-pin connector. Make twelve **split
extension cables** once — extension lead with the red conductor cut and run out to the
bus — and each servo still plugs in with a single connector at its end. This is what the
BOM's **"4x Servo extension cables"** line anticipates.

Doing it now means the fiddly work happens once, on the bench, before anything is
mounted in a chassis.

### During calibration

Only one or two servos are connected at a time, but the topology is already final:

- Servo **orange** → the channel under test on the PCA9685
- Servo **red** → the 6 V bus
- Servo **brown** → the ground star point

Unused bus taps stay unconnected. Nothing changes when the remaining ten arrive.

> **The simpler alternative, for reference.** Servo power can instead run through the
> PCA9685's `V+` terminal and out via its 3-pin headers — one plug per servo, no harness
> to build. That is fine below ~2 A, so it works for calibration, but the board's `V+`
> trace and its ground return become the limit at twelve servos. Choosing the bus now
> avoids that conversation entirely. See [Servo rail](#servo-rail).

### Logic and I2C

| ESP32 pin | PCA9685 pin | Note |
| --- | --- | --- |
| `IO47` | SDA | `SDA_PIN=47` in the `s3cam` env **[code]** |
| `IO41` | SCL | `SCL_PIN=41` in the `s3cam` env **[code]** |
| `3V3` | VCC | **3.3 V only** — see [Cautions](#cautions) |
| `GND` | GND | Required; also the common ground for the servo supply |

Both pins are free on this board and touch neither the camera DVP bus nor the memory
pins. `IO14` is reserved for the WS2812 and stays clear.

Most PCA9685 breakouts carry their own I2C pull-ups, so no external resistors are needed.

### Servo rail

`V+` (servo rail) and `VCC` (logic) are **separate supplies on the PCA9685 — never bridge
them.** `V+` at 6 V into a 3.3 V logic pin destroys the ESP32. In this build `V+` is left
unconnected, so the only thing that must not reach `VCC` is the bus.

**The PCA9685 IC is not in the servo current path.** `V+` runs from the terminal straight
to the middle pin of each 3-pin header; the chip drives only the signal pins. That is why
leaving `V+` off costs nothing functionally, and why the board's limit was never the
silicon.

#### If you ever revert to powering servos through the board

The stock `V+` trace is comfortable with roughly 2–3 A **[inferred]**. Twelve
TD-8135MG servos draw ~2 A idling and several amps walking, all through one trace and one
screw terminal — which is why the project's BOM lists the PCA9685 with the note **"Add
thicker solder traces"** ([1_components.md](1_components.md), [readme.md](../readme.md)).

The failure mode is **voltage drop, not smoke**: servos far from the terminal see less
than 6 V and run weaker and slower than the near ones. That reads as an asymmetric limp
and sends you hunting for a kinematics bug. Reinforce `GND` as well as `V+` — the return
carries the same current, and a ground drop also shifts the servos' signal reference.

Mitigations, if needed: flow solder along both traces, lay 18 AWG wire along them, or feed
`V+` from both ends of the board to halve the copper any current traverses.

**None of this applies to the wiring above**, which keeps servo current off the board
entirely.

## Power architecture

### The battery: 2S

This project's BOM specifies a **7.6–8.4 V** pack — "4x 18650 in 2s2p configuration, but
other people have 2s LiPos" ([1_components.md](1_components.md)) — and
[spot.md](spot.md) records a maximum battery voltage of 8.4 V. So: **2S**.

Set the bench supply to **7.4 V** as the working default, and sweep the range when
testing, because a 2S pack does not sit still:

| State | Pack voltage | SZBK07 headroom over 6 V | CN3903 headroom over 5 V |
| --- | --- | --- | --- |
| Fully charged | 8.4 V | 2.4 V — fine | 3.4 V — fine |
| Nominal | 7.4 V | 1.4 V — marginal | 2.4 V — fine |
| Low (3.5 V/cell) | 7.0 V | 1.0 V — below spec | 2.0 V — fine |
| Flat (3.0 V/cell) | 6.0 V | none | 1.0 V — marginal |

### What that means

**The SZBK07 will stop regulating before the battery is empty.** Both converters are
bucks needing roughly 1.5–2 V of headroom, and a discharging 2S pack crosses that
threshold around 7.5–8 V.

This degrades gracefully rather than failing: once it drops out, the converter becomes
roughly a pass-through and the servo rail follows the pack down. The servos are rated
**4.8–7.4 V**, so a rail sagging from 6.0 V toward 5.5 V stays in spec — they get weaker
and slower, not damaged. Budget for less torque near the end of a charge.

**The SZBK07 is still required, because of the top of the range, not the bottom.** A
charged 2S is **8.4 V**, above the servos' 7.4 V rating. Feeding them from the battery
directly — which the BOM offers as an option — would overrun them whenever the pack is
full. Keep the converter and hold the rail at 6 V.

**The CN3903 is comfortable throughout.** It needs only 5.5 V in, so the ESP32's rail
stays solid across the whole discharge curve and for a while after the servos start
fading. That is the right way round: the MCU should outlive the actuators, not the
reverse.

> **If this becomes limiting, 3S is the alternative.** 11.1 V nominal gives both
> converters headroom across the entire discharge, at the cost of diverging from the
> reference design and the 8.4 V figure in `spot.md` — worth checking whether that is a
> hard limit somewhere before switching.

### Setting it up, in order

Both converters ship at arbitrary settings, and the SZBK07 can put out **36 V**. Set them
with **nothing connected downstream**.

1. Bench supply to 7.4 V, current limit low (~0.5 A) for the first power-up.
2. **SZBK07 alone**: CC pot to minimum, then raise **CV until the unloaded output reads
   6.0 V**. Confirm the polarity of `OUT+`/`OUT−` with the meter before trusting the
   silkscreen.
3. **CN3903 alone**: nothing to adjust, but **meter the unloaded output and confirm
   5.0 V** before it goes anywhere near the ESP32 — a fixed module built with the wrong
   feedback divider cannot be corrected.
4. Power down. Connect the PCA9685's `V+`/`GND` to the SZBK07, the ESP32's `VCC`/`GND` to
   the CN3903, and the logic wiring.
5. Raise the bench limit to suit the phase below, and set the SZBK07's **CC**.

### Current, by phase

| Phase | 6 V rail draw | 7.4 V pack draw **[inferred]** |
| --- | --- | --- |
| I2C only, no servos | ~0 | < 0.2 A |
| One servo, unloaded | 0.15 – 0.3 A | < 0.3 A |
| One servo stalled | 2.6 – 3.4 A | ~2.9 A |
| Twelve servos, walking | several A | several A |
| Twelve servos, all stalled | up to ~36 A | **~31 A — far beyond a bench supply** |

At 2S the pack current is close to the rail current, since the step-down ratio is small.
That is the flip side of the good headroom a higher pack would give: **3S would draw
roughly half the amps for the same power**, which matters for wiring gauge and connector
choice on the assembled robot.

Set the SZBK07's **CC** to a little above the phase you are in — around **1.5 A** while
calibrating one servo. That is the limit that matters, because it sits on the servo rail
itself and folds back before a jammed servo can sit at stall current indefinitely.

**Your bench supply almost certainly cannot deliver the bottom row.** That is fine for
calibration and worth knowing before you conclude the robot has a firmware fault when it
browns out under twelve-servo load. Size the pack, wiring and connectors against
[measured](#gaps-and-open-questions) draw rather than that worst case, which assumes every
servo stalls simultaneously — but note that 2S puts the whole current burden on the pack
at close to rail amperage, so the 18650 cells' own discharge rating becomes a real
constraint.

### Thermal

At calibration currents the SZBK07 runs cold. Its 300 W rating assumes forced air, and
the seller quotes both 15 A and 20 A without resolving which — see
[the component doc](components/SZBK07-buck-converter.md). Check the heatsinks by hand
after sustained multi-servo load.

## Cautions

**Set the SZBK07's output before connecting anything downstream.** Its CV pot can deliver
**36 V** and it ships at an arbitrary setting. Measure 6.0 V on the unloaded output first
— see [Setting it up](#setting-it-up-in-order).

**Meter the CN3903 too.** It is fixed at 5 V with nothing to adjust, which is why it is
the documented choice, but confirm 5.0 V unloaded before connecting the ESP32. There is no
pot to correct a module built wrong.

**The PCA9685's VCC must be 3.3 V, not 5 V.** The breakout's I2C pull-ups go to `VCC`, so a 5 V logic
supply idles SDA/SCL at 5 V. The ESP32-S3 is **not 5 V tolerant on GPIO** — this damages
the chip. Either feed `VCC` from `3V3`, or use a level shifter.

**The UI's PWM slider can command out-of-range pulses.** It spans **80 – 600 counts**
([servos.svelte:41-46](../app/src/routes/peripherals/servo/servos.svelte#L41-L46)) and
`setServoPWM()` does not clamp **[code]**. At the configured timing that is roughly
420 – 3170 µs **[inferred]**, against the servo's rated 500 – 2500 µs. **Both ends of the
slider drive the servo past its limits**, where it stalls against a mechanical stop and
draws stall current continuously. Move it gently, and never park it at an extreme.

**"All servos" moves all twelve at once.** Combined with the above, that is twelve stalled
servos on one rail. Leave it off during calibration.

**Deactivate before touching a horn.** `deactivate()` calls `_pca.sleep()`, which genuinely
cuts the outputs **[code]**. The firmware boots deactivated, so outputs are off until you
enable them.

**Horn off for the first sweep of each channel.** Until you know where centre is, the horn
can drive linkage into a hard stop.

## Order of work

1. **Build the harness first** — the 6 V bus, the ground star point, and the split
   extension cables. Doing it before anything is powered means the fiddly work happens
   once, on the bench.
2. Wire **I2C only** — no servos, no bus connection. Confirm `0x40` appears on the I2C
   scan page (a WebSocket correlation request, not a REST endpoint). The
   `i2c.master: I2C transaction unexpected nack` logging stops and the control loop
   returns to a full 100 Hz — that is the independent confirmation.
3. Set both converters unloaded, then energise the bus with **no servos attached**.
   Verify 6.0 V at a bus tap and 5.0 V at the ESP32 with a meter.
4. Attach **one servo, horn removed** — orange to the channel under test, red to the bus,
   brown to the star point. Use the per-channel PWM slider to find mechanical centre; that
   value is `center_pwm`. Raw PWM mode bypasses the angle maths entirely **[code]**, which
   is what you want here.
5. Fit the horn at the neutral position, then resolve `direction`, `conversion` and
   `center_angle` in angle mode.
6. Repeat per channel. Save via `/api/servo/config`, which persists to LittleFS. Nothing
   about the wiring changes as servos are added — only how many bus taps are in use.

## Calibration parameters

Each tick computes **[code]**
([servo_controller.h:104-111](../esp32/include/peripherals/servo_controller.h#L104-L111)):

```cpp
angle = servo.direction * angles[i] + servo.center_angle;
pwm   = angle * servo.conversion + servo.center_pwm;
pwm   = clamp(pwm, 125, 600);
```

| Field | Default | Meaning |
| --- | --- | --- |
| `center_pwm` | 306 | Counts at the joint's neutral position |
| `direction` | ±1 | Matches the joint's sense to the kinematics convention |
| `center_angle` | 0, ±45, ±90 | Offset from mechanical zero to kinematic zero |
| `conversion` | 2.0 | Counts per degree |

Rest pose is `{0, 90, -145}` per leg, repeated four times, so joint order within a leg is
**coxa → femur → tibia** **[inferred]**.

## Gaps and open questions

- [ ] **The `conversion` default slightly overshoots at full deflection.** With 180°
      servos and the real ≈5.28 µs count below, `conversion = 2.0` maps ±90° to
      ≈665–2566 µs **[inferred]** against a rated 500–2500 µs. The bottom end is fine;
      the top is ~66 µs past spec, so a joint commanded to its extreme may sit against
      its stop. Trimming `conversion` to ≈1.85, or re-centring, would fit the full sweep
      inside the rated range. Measure before deciding — it depends on the real oscillator.
- [ ] **The oscillator setting does not match the hardware.**
      `FACTORY_SERVO_OSCILLATOR_FREQUENCY` is **27 MHz**
      ([factory_settings.ini](../esp32/factory_settings.ini)) while most PCA9685 parts run
      a **25 MHz** oscillator. With prescale 131 **[code]** the real output is ≈46 Hz, not
      the nominal 50 Hz, and one count is ≈5.28 µs rather than 4.88 µs. So `center_pwm`
      306 is ≈1616 µs, not the 1494 µs the numbers suggest. **Do not trust computed
      microseconds — find centre empirically.** Measuring the real frequency with a scope
      would let this be corrected properly.
- [ ] **Channel → leg mapping is not documented.** Servos are named only `Servo1..12`
      **[code]**. Confirm against [kinematics.h](../esp32/include/kinematics.h) before
      trusting it; a wrong mapping produces a plausible-looking stance that walks into
      itself.
- [ ] **Is the 125–600 clamp right for these servos?** At ≈5.28 µs/count that is
      660–3170 µs, wider than the rated 500–2500 µs. The clamp is the last line of
      defence and currently permits driving past the mechanical stops.
- [ ] **Is the board's `3V3` pin a regulator output or an input?** Carried over from
      [the board doc](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md). Matters here because the
      PCA9685's logic side is being fed from it.
- [ ] **Rail voltage choice.** 6 V is safe and is what this document assumes. The part is
      rated to 7.2 V (8.4 V by some listings), where torque rises from 32.7 to
      35.2 kg·cm at the cost of higher stall current. Decide before sizing the robot's
      supply, since it changes the amperage budget.
