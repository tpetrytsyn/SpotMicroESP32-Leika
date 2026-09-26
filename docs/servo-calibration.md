# Servo calibration — setup and parameters

Bench setup for calibrating the twelve leg servos against
[`ServoController`](../esp32/include/peripherals/servo_controller.h).

**Calibration runs on the robot's final wiring**, specified in [wiring.md](wiring.md). Only
one or two servos are powered at a time and the robot is not assembled, but nothing in the
harness is bench-specific — so nothing is rebuilt at assembly, and the harness itself gets
tested now rather than being a fresh unknown later. The only bench values are the
[bench supply](#bench-power) standing in for the battery and the
[CC limit](#current-limit-during-calibration) starting low; both are called out as
temporary.

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

## What calibration needs

Everything electrical — converters, fuses, PCA9685, servo distribution board, ground star,
I2C, monitoring — is the robot's harness, specified in [wiring.md](wiring.md). On top of it:

| # | Item | Why |
| --- | --- | --- |
| 1 | **The harness, commissioned** | Converters set and metered first — [Commissioning](wiring.md#commissioning) |
| 2 | **Bench supply**, 7.4 V, current-limited, on an XT60 | Stands in for the [2S 1500 mAh LiPo](components/ZOP-Power-2S-1500mAh-LiPo.md); see [Bench power](#bench-power) |
| 3 | Multimeter | Verify 3.3 V logic and rail voltages *before* connecting |
| 4 | Servo horns + mounting screws | Fitted at calibrated centre |
| 5 | A vice, clamp or jig | Hold the servo while it moves — a 35 kg·cm servo will throw itself around |
| 6 | CH343P USB-serial adapter | Only for reflashing / serial log |

## Bench power

The bench supply stands in for the battery, at **7.4 V** as the working default. Sweep
6.6 – 8.4 V when testing, because a 2S pack does not sit still — see
[Power architecture](wiring.md#power-architecture) for what each voltage means for the
converters. Give the supply its own XT60, so the connector and the 15 A fuse are in
circuit from the first session.

There are **three** current limits in series: the bench supply's, the SZBK07's CC pot on
the servo rail, and the mechanical one of a servo that simply stalls. Set the first two
deliberately.

### Current, by phase

| Phase | 6 V rail draw | 7.4 V pack draw **[inferred]** |
| --- | --- | --- |
| I2C only, no servos | ~0 | < 0.2 A |
| One servo, unloaded | 0.15 – 0.3 A | < 0.3 A |
| One servo stalled | 2.6 – 3.4 A | ~2.9 A |
| Twelve servos, walking | several A | several A |
| Twelve servos, all stalled | up to ~36 A | **~31 A — far beyond a bench supply** |

At 2S the pack current is close to the rail current, since the step-down ratio is small.

### Current limit during calibration

**CC is the one setting that is deliberately temporary.** Everything else in the harness is
specified for the finished robot; the current limit is not, because it should sit a little
above whatever phase you are in so a jam folds back instead of feeding stall current
indefinitely.

**Set CC to ~1.5 A while calibrating one servo.** The assembled value, ~12 A, and the
reasoning behind it are in [wiring.md](wiring.md#current-limit-cc).

**Your bench supply almost certainly cannot deliver the bottom row** of the table above.
That is fine for calibration and worth knowing before you conclude the robot has a
firmware fault when it browns out under twelve-servo load.

## During calibration

Only one or two servos are connected at a time, but the topology is already final:

- Servo plugs into **the channel under test** on the distribution board, stock connector
- That header's signal pin already runs to the PCA9685 channel

Unused headers stay empty. Nothing changes when the remaining ten arrive.

> **The shortcut, for reference.** Servo power can instead run through the PCA9685's `V+`
> terminal and out via its own 3-pin headers, skipping the distribution board entirely.
> That is fine below ~2 A, so it works for calibration, but the board's `V+` trace and its
> ground return become the limit at twelve servos. Since the distribution board plugs
> servos in exactly the same way, the shortcut saves an afternoon and costs the rework.
> See [Servo rail](wiring.md#servo-rail).

## Cautions

Wiring cautions — setting the converters before connecting anything, 3.3 V logic supplies,
the balance lead — are in [wiring.md](wiring.md#cautions). The ones specific to
calibration:

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

1. **Build the [servo distribution board](wiring.md#building-the-servo-board) first**, then
   the [ground star cluster](wiring.md#the-ground-star-point). Doing it before anything is
   powered means the soldering happens once, on the bench.
2. Wire **I2C only** — no servos, board not energised. Confirm `0x40` appears on the I2C
   scan page (a WebSocket correlation request, not a REST endpoint), and `0x48` once the
   ADS1115 is fitted. The `i2c.master: I2C transaction unexpected nack` logging stops and
   the control loop returns to a full 100 Hz — that is the independent confirmation.
3. [Commission](wiring.md#commissioning) the converters unloaded, then energise the board
   with **no servos plugged in**. Verify 6.0 V across a header's centre and ground pins, and
   5.0 V at the ESP32. Check the ground row for polarity here, while a mistake still costs
   nothing.
4. Plug in **one servo, horn removed**, stock connector into the channel under test. Use
   the per-channel PWM slider to find mechanical centre; that value is `center_pwm`. Raw
   PWM mode bypasses the angle maths entirely **[code]**, which is what you want here.
5. Fit the horn at the neutral position, then resolve `direction`, `conversion` and
   `center_angle` in angle mode.
6. Repeat per channel. Save via `/api/servo/config`, which persists to LittleFS. Nothing
   about the wiring changes as servos are added — only how many headers are occupied.

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

Wiring and power questions — the board's `3V3` pin, rail voltage, walking draw — are in
[wiring.md](wiring.md#open-questions).

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
