# Servo calibration — components and wiring

Bench setup for calibrating the twelve leg servos against
[`ServoController`](../esp32/include/peripherals/servo_controller.h). This covers the
**calibration step only** — one or two servos powered at a time, robot not assembled.
Powering all twelve for walking is a different problem, see [Power](#power).

Confidence is marked throughout, following
[ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[code]** = read from this repo, **[datasheet]** = from the part's spec,
**[inferred]** = reasoned and needs confirmation on hardware.

## Servos

DS3235 or equivalent — digital, metal gear, 25T spline. **[user-supplied]**

| Item | Value | Note |
| --- | --- | --- |
| Operating voltage | 4.8 – 6.8 V | **[datasheet]** |
| Stall torque | ~35 kg·cm @ 6.8 V | **[datasheet]** |
| Stall current | ~2.8 – 3.5 A each | **[datasheet]**, varies by source |
| No-load current | ~100 – 200 mA | **[datasheet]** |
| Pulse width range | 500 – 2500 µs | **[datasheet]** |
| Rotation | **180°** | **[user-confirmed]**; a 270° variant also exists, not used here |

> These figures are for a genuine DSSERVO DS3235. Clones vary, sometimes a lot,
> particularly on stall current. Confirm against whatever your supplier states.

## Components

| # | Item | Why |
| --- | --- | --- |
| 1 | **PCA9685** 16-channel PWM board, I2C | The driver targets `0x40` **[code]** |
| 2 | **Bench supply**, 6 V, current-limited | Set ~1.5 A for single-servo work; see [Power](#power) |
| 3 | **5 V supply for the ESP32** — or the CH343P's USB 5 V | The board's `VCC` is a 5 V input |
| 4 | 3 × jumper wires (SDA, SCL, GND) | ESP32 ↔ PCA9685 logic |
| 5 | 1 × jumper (3V3) | PCA9685 logic supply — **must be 3.3 V**, see [Cautions](#cautions) |
| 6 | 2 × wires for the servo rail, 18 AWG or heavier | `V+` / `GND` screw terminal |
| 7 | **1000 – 2200 µF** electrolytic, ≥ 10 V | Across `V+`/`GND` at the PCA9685; most boards have the footprint |
| 8 | Multimeter | Verify 3.3 V logic and rail voltage *before* connecting |
| 9 | Servo horns + mounting screws | Fitted at calibrated centre |
| 10 | A vice, clamp or jig | Hold the servo while it moves — a 35 kg·cm servo will throw itself around |
| 11 | CH343P USB-serial adapter | Only for reflashing / serial log |

The bench supply's current limit is the main safety device in this setup. Set it to
~1.5 A while working on one servo: normal motion stays well under that, and a miswire,
a stalled horn or a short trips the limit instead of cooking something. Raise it only
when you move to multiple servos.

## Wiring

```
  ESP32-S3-CAM (HW-679)              PCA9685                Servo supply 6 V
  ─────────────────────              ───────                ────────────────
  IO47  (SDA) ───────────────────── SDA
  IO41  (SCL) ───────────────────── SCL
  3V3         ───────────────────── VCC   (logic, 3.3 V)
  GND         ───────────────────── GND ──────────────────── −  (common ground)
                                     V+ ──────────────────── +
                                                             │
                                            1000–2200 µF ────┤
                                                             │
                                    CH0..CH11 ── servo signal/V+/GND
  VCC (5 V)   ◄──── separate 5 V (CH343P USB, or a buck from the 6 V rail)
```

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

| From | To |
| --- | --- |
| Servo supply **+** | PCA9685 `V+` screw terminal |
| Servo supply **−** | PCA9685 `GND` screw terminal |

`V+` (servo rail) and `VCC` (logic) are **separate supplies on the PCA9685 — never bridge
them.** `V+` at 6 V into a 3.3 V logic pin destroys the ESP32.

Servos plug into channels `CH0`–`CH11` as standard 3-pin (signal / V+ / GND).

## Power

**For calibration** — one or two servos moving at a time — the bench supply at 6 V with
the limit at ~1.5 A is right. A single DS3235 draws 100–200 mA unloaded and only
approaches its ~3 A stall figure when jammed, which is exactly the case the limit should
catch.

**For the assembled robot** the budget is completely different. Twelve DS3235 at ~3 A
stall is ~36 A in the pathological case. Real walking loads are far lower, but inrush
when twelve servos energise simultaneously is not: an undersized supply browns out, and a
brownout reset mid-calibration loses whatever you had not saved. Size for 6 V at
20–30 A **[inferred]** and confirm against measured draw.

The ESP32 board needs its own 5 V on `VCC`, not the 6 V servo rail. During bench work the
CH343P's USB 5 V is sufficient — the camera is the current-hungry part and you are not
streaming while calibrating.

## Cautions

**VCC must be 3.3 V, not 5 V.** The breakout's I2C pull-ups go to `VCC`, so a 5 V logic
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

1. Wire **I2C only** — no servos, no servo rail. Confirm `0x40` appears on the I2C scan
   page (a WebSocket correlation request, not a REST endpoint). The
   `i2c.master: I2C transaction unexpected nack` logging stops and the control loop
   returns to a full 100 Hz — that is the independent confirmation.
2. Add the servo rail with **no servos attached**. Verify `V+` at the terminal with a
   meter.
3. Attach **one servo, horn removed**. Use the per-channel PWM slider to find mechanical
   centre — that value is `center_pwm`. Raw PWM mode bypasses the angle maths entirely
   **[code]**, which is what you want here.
4. Fit the horn at the neutral position, then resolve `direction`, `conversion` and
   `center_angle` in angle mode.
5. Repeat per channel. Save via `/api/servo/config`, which persists to LittleFS.

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
      [the board doc](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md). Matters here because the
      PCA9685's logic side is being fed from it.
- [ ] **Actual stall current of your specific servos** — clone figures vary, and the
      supply sizing above depends on it.
