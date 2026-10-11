# Servo calibration

Calibrating the twelve leg servos against
[`ServoController`](../esp32/include/peripherals/servo_controller.h). It runs on the finished
harness from [pre-calibration-build.md](pre-calibration-build.md), from the bench supply or the
battery; the wiring itself is in [wiring.md](wiring.md).

## Servos

**DS3235 / TD-8135MG**, 35 kg·cm, **180° positional** —
[full reference](components/DS3235-TD-8135MG-servo.md). When ordering more, check the variant:
the same name is sold as 360° continuous rotation, which can't hold a joint.

| Item | Value |
| --- | --- |
| Operating voltage | 4.8–7.4 V (case); 6.0 V here |
| Stall current | 2.6–3.4 A **[datasheet]** |
| Running current | 140–200 mA **[datasheet]** |
| Pulse width | 500–2500 µs **[datasheet]** |
| Cable | orange signal, red V+, brown GND |

## Power

- **Bench supply:** 7.4 V, limit 2–3 A. Sweep 6.6–8.4 V when testing — a pack doesn't sit still.
- **Battery:** as in [Running from the battery](pre-calibration-build.md#running-from-the-battery);
  nothing in the firmware watches the pack yet.
- **CC stays at 1.5 A** throughout ([build step 7](pre-calibration-build.md#7-szbk07-servo-rail-converter)),
  so a jam is held at 1.5 A instead of full stall current. The assembled value is in
  [wiring.md](wiring.md#current-limit-cc).

| Phase | 6 V rail | Pack **[inferred]** |
| --- | --- | --- |
| No servos | ~0 | < 0.2 A |
| One servo, unloaded | 0.15–0.3 A | < 0.3 A |
| One servo stalled | 2.6–3.4 A | ~2.9 A |
| Twelve walking | several A | several A |
| Twelve stalled | up to ~36 A | ~31 A — beyond a bench supply |

At 2S the pack current is close to the rail current, since the step-down ratio is small. A bench
supply can't feed the bottom rows: brownouts there are the supply, not the firmware.

## Cautions

- **The PWM slider can command out-of-range pulses.** It spans 80–600 counts
  ([servos.svelte](../app/src/routes/peripherals/servo/servos.svelte)) and `setServoPWM()` doesn't
  clamp **[code]** — ≈420–3170 µs **[inferred]** against the servo's 500–2500 µs. Both ends drive it
  into its stops at stall current. Move it gently; never park it at an extreme.
- **"All servos" moves all twelve at once.** Leave it off.
- **Active first drives every channel to the current pose angle** **[code]**; the slider takes
  over the selected channel once moved. Keep the horn off until you know where centre is.
- **Deactivate before touching a horn.** `deactivate()` puts the PCA9685 to sleep, cutting the
  outputs **[code]**. The firmware boots deactivated.

## Order of work

1. Harness built and tested through [build step 13](pre-calibration-build.md#13-first-servo): one
   servo on its header, clamped, horn off.
2. **Centre:** with the per-channel PWM slider, find mechanical centre — that value is
   `center_pwm`. Raw PWM mode bypasses the angle maths **[code]**.
3. **Horn:** fit it at the neutral position, then resolve `direction`, `conversion` and
   `center_angle` in angle mode.
4. Repeat per channel; the harness doesn't change as servos are added. Save via
   `/api/servo/config`, which persists to LittleFS.

## Calibration parameters

Each tick **[code]** ([servo_controller.h](../esp32/include/peripherals/servo_controller.h)):

```cpp
angle = servo.direction * angles[i] + servo.center_angle;
pwm   = angle * servo.conversion + servo.center_pwm;
pwm   = clamp(pwm, 125, 600);
```

| Field | Default | Meaning |
| --- | --- | --- |
| `center_pwm` | 306 | Counts at the joint's neutral position |
| `direction` | ±1 | Joint sense against the kinematics convention |
| `center_angle` | 0, ±45, ±90 | Offset from mechanical to kinematic zero |
| `conversion` | 2.0 | Counts per degree |

Rest pose is `{0, 90, −145}` per leg, so the joint order within a leg is coxa → femur → tibia.
Channels map to joints per [2_assembly.md](2_assembly.md#circuit-diagram): CH0–2 front left,
CH3–5 front right, CH6–8 rear left, CH9–11 rear right.

## Open questions

- [ ] **Oscillator mismatch.** `FACTORY_SERVO_OSCILLATOR_FREQUENCY` is 27 MHz
      ([factory_settings.ini](../esp32/factory_settings.ini)); most PCA9685s run 25 MHz. With
      prescale 131 **[code]** the output is ≈46 Hz and one count ≈5.28 µs, so `center_pwm` 306 is
      ≈1616 µs, not 1494 µs. Find centre empirically; a scope would allow a proper fix.
- [ ] **`conversion` overshoots at full deflection:** 2.0 maps ±90° to ≈665–2566 µs **[inferred]**,
      ~66 µs past spec at the top. ≈1.85, or re-centring, fits the sweep — measure first.
- [ ] **The 125–600 clamp** is ≈660–3170 µs, wider than the servo's range at the top: the last line
      of defence currently permits driving into the stops.
- [ ] **Channel mapping** from 2_assembly.md, not yet confirmed against
      [kinematics.h](../esp32/include/kinematics.h).
