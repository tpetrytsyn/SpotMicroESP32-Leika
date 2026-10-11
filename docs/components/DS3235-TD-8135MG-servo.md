# DS3235 / TD-8135MG 35 kg digital servo

The twelve leg servos. Bought as
["Цифровий металевий сервопривід 180° DS3235 робоче зусилля 35 кг"](https://prom.ua/ua/p2230486878-tsifrovoj-metallicheskij-servoprivod.html)
(prom.ua), also listed as **TD-8135MG**.

The delivered case reads **`TD-8135MG`**, `35 Kg`, `4.8-7.4V`, `DIGITAL SERVO`, so the TIANKONGRC
TD-8135MG datasheet applies.

- The listing's photos show the same housing unmarked. It's shared by a 20/25/30/35 kg family
  (red for 20/25, blue for 30/35), so stock images don't show what arrives.
- TD-8135MG is also sold as 360° continuous rotation, which can't hold a joint. These are
  **180° positional** — check when reordering.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Operating voltage | **4.8–7.4 V** | case — the seller says 3.7–7.2 V, the datasheet 4.8–7.2 V (8.4 V in some listings). 6 V is inside all of them |
| Stall torque | 32.7 kg·cm @ 4.8 V, 35.2 kg·cm @ 8.4 V | datasheet (seller: 35 kg·cm) |
| Stall current | 2.6–3.4 A (±10 %); up to 3.8 A @ 8.4 V | datasheet |
| Running current | 140–200 mA | datasheet |
| Pulse width | 500–2500 µs | datasheet |
| Speed | 0.32 s/60° @ 4.8 V, 0.22 s/60° @ 8.4 V | datasheet — the seller's 0.15 s/60° @ 5 V is a 20 kg-class figure |
| Dead band | 3 µs | seller, who writes "3 seconds" |
| Motor | coreless | datasheet |
| Gears | full metal: steel, brass output gear | photo |
| Output spline | 25T | owner |
| Size, weight | 46 × 40 × 20 mm, 68 g | seller + photo |
| Temperature | −20…+60 °C | seller |
| Sealing | none — plain bottom cover, not the waterproof variant | photo |

The seller gives no pulse range, stall current or no-load current; the datasheet does, but reseller
figures vary — planning numbers, not limits.

In the box: 1-arm, 4-arm, 6-arm and disc horns; 4 grommets, 4 brass eyelets, 4 mounting screws, a
horn screw.

## Cable

JR order: **orange** signal, **red** V+ (middle pin), **brown** GND — brown goes on the servo board's
marked GND row. A reversed plug swaps signal and ground; V+ stays in the middle, so the servo simply
won't run, and its return current is limited by the PCA9685's series resistors.

## Firmware interaction

| Firmware value | With this servo |
| --- | --- |
| `center_pwm` = 306 | ≈1616 µs at the real ≈5.28 µs/count — in range |
| `conversion` = 2.0 | ±90° → ≈665–2566 µs **[inferred]**, ~66 µs past 2500 at the top |
| clamp 125–600 | ≈660–3170 µs — wider than the servo at the top |
| UI slider 80–600 | ≈420–3170 µs — outside the range at both ends |
| `FACTORY_SERVO_PWM_FREQUENCY` = 50 | Accepted by digital servos; real output ≈46 Hz |

The slider and the clamp can drive the servo into its stops — see
[servo-calibration.md](../servo-calibration.md#cautions).

## To measure

- [ ] Usable pulse range — sweep from centre; replaces the assumed 500–2500 µs clamp
- [ ] Stall current at 6 V, with the bench limit as protection
- [ ] Travel for a commanded 180° — calibrates `conversion`
- [ ] Real speed, 0.15 or 0.32 s/60°, before tuning gait timing
- [ ] Limp or holding with no PWM signal — decides whether deactivating ends a stall
- [ ] Accepts the PCA9685's 3.3 V pulses — [test 13](../pre-calibration-build.md#13-first-servo)
