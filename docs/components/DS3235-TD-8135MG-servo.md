# DS3235 / TD-8135MG 35 kg digital servo

Hardware reference for the twelve leg servos used in this build. Purchased as
["Цифровий металевий сервопривід 180° DS3235 робоче зусилля 35 кг"](https://prom.ua/ua/p2230486878-tsifrovoj-metallicheskij-servoprivod.html)
(prom.ua), which also lists the alternative designation **TD-8135MG**.

Confidence is marked throughout, following
[ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[photo]** = read off the product images, **[listing]** = stated by the seller,
**[reference]** = from a TD-8135MG datasheet elsewhere, applied by analogy,
**[inferred]** = reasoned, needs confirmation on hardware.

## Identity

The delivered part is marked **`TD-8135MG`** on the case, alongside `35 Kg`, `4.8-7.4V`
and `DIGITAL SERVO` **[owner photo]**. So the TIANKONGRC TD-8135MG datasheet applies
directly rather than by analogy.

Two provenance notes:

- The seller's own listing photos show the **same housing without the model number**, and
  the listing titles it `DS3235`. The housing is shared across a 20 / 25 / 30 / 35 kg
  family — red for 20/25 kg, blue for 30/35 kg. Stock imagery on this listing is
  therefore not a reliable guide to what arrives.
- TD-8135MG is sold in both 180° positional and 360° continuous-rotation versions under
  the same name. The servos in this build are the **180° positional** version — confirmed
  by the owner and stated in the listing title. Worth re-checking when ordering
  replacements, since the continuous-rotation variant cannot hold a joint at all.

Datasheet figures for TD-8135MG still vary between resellers, so the measurement list at
the end is worth completing regardless.

## Confirmed from photographs of the delivered part

| Item | Value |
| --- | --- |
| Case marking | `35 Kg`, `4.8-7.4V`, **`TD-8135MG`**, `DIGITAL SERVO` |
| **Operating voltage (case)** | **4.8 – 7.4 V** |
| Dimensions | 46 mm (H) × 40 mm (L) × 20 mm (W) |
| Gear train | Full metal — steel gears with a brass/bronze output gear |
| Output shaft | Splined metal, **25T** **[owner-confirmed]** — standard 25-tooth horn fitment |
| Cable | 3-wire, female connector; **orange / red / brown** |
| Bottom cover | Removable, plain plastic — **not sealed** |
| Waterproofing | **None visible.** This is not the waterproof variant sometimes sold under these names |

### Cable colours

Standard JR/Graupner ordering **[photo]**, which matters when plugging into the PCA9685:

| Wire | Function |
| --- | --- |
| Orange | Signal (PWM) |
| Red | V+ (servo rail) |
| Brown | GND |

Check the silkscreen on your PCA9685 before inserting — brown must land on the
ground pin. Reversing the connector puts the servo rail onto the signal line.

### In the box

Per servo **[photo]**: 1-arm horn, 4-arm cross horn, 6-arm star horn, round disc horn,
4 × rubber grommets, 4 × brass eyelets, 4 × long mounting screws, 1 × horn screw.

## Stated by the seller

| Item | Value | Note |
| --- | --- | --- |
| Rotation | **180°, positional** | **[listing]** + **[owner-confirmed]** |
| Stall torque | 35 kg·cm | **[listing]** |
| Operating voltage | 3.7 – 7.2 V | **[listing]** — conflicts with the case, see below |
| Speed | 0.15 s/60° @ 5 V; 0.13 s/60° @ 6.8 V | **[listing]** — see below |
| Operating temperature | −20 °C … +60 °C | **[listing]** |
| Dead band | "3 seconds" | **[listing]** — plainly a typo for **3 µs** |
| Weight | 68 g | **[listing]** |
| Dimensions | 46 × 40 × 20 mm | **[listing]**, matches the photo |

**The seller does not state pulse width range, stall current, or no-load current** — the
three figures that matter most for this project.

## From the TD-8135MG datasheet

The case marking identifies the part, so these apply directly. **[datasheet]** — they
fill the gaps the seller left, but reseller figures for this model still vary, so treat
them as good planning numbers rather than guaranteed limits.

| Item | Value |
| --- | --- |
| Pulse width range | 500 – 2500 µs |
| Stall current | 2.6 – 3.4 A (±10%); up to 3.8 A at 8.4 V |
| Running current | 140 – 200 mA |
| No-load speed | 0.32 s/60° @ 4.8 V; 0.22 s/60° @ 8.4 V |
| Stall torque | 32.7 kg·cm @ 4.8 V; 35.2 kg·cm @ 8.4 V |
| Motor | Coreless |

## Conflicting figures

| Parameter | Case **[owner photo]** | Seller **[listing]** | TD-8135MG **[datasheet]** |
| --- | --- | --- | --- |
| Voltage | **4.8 – 7.4 V** | 3.7 – 7.2 V | 4.8 – 7.2 V (8.4 V in some listings) |
| Speed @ ~5 V | — | 0.15 s/60° | 0.32 s/60° @ 4.8 V |

**Voltage:** trust the case. 3.7 V is implausible as a *working* minimum for a 35 kg
digital servo and is more likely a copy-paste artefact. **6 V sits safely inside every
version of the range** and is what [servo-calibration.md](../servo-calibration.md) assumes.

**Speed:** the two sources differ by more than 2×. The seller's 0.15 s/60° is a
20 kg-class figure; the reference 0.32 s/60° is more plausible for 35 kg at low voltage.
This affects nothing during calibration, but it matters for gait timing later — the
100 Hz control loop can command faster transitions than the servo can physically track,
and if it does, the leg simply lags the commanded trajectory.

## What this means for the firmware

| Firmware value | Interaction with this servo |
| --- | --- |
| `center_pwm` = 306 | ≈1616 µs at the real ≈5.28 µs/count — inside the 500–2500 µs range |
| `conversion` = 2.0 | Maps ±90° to ≈665–2566 µs **[inferred]** — the top end is ~66 µs past 2500 |
| clamp 125–600 | ≈660–3170 µs — **wider than the servo's rated range at the top** |
| UI slider 80–600 | ≈420–3170 µs — **outside the rated range at both ends** |
| `FACTORY_SERVO_PWM_FREQUENCY` = 50 | Digital servos generally accept this; real output is ≈46 Hz |

The last two are the ones that can damage hardware — see the cautions in
[servo-calibration.md](../servo-calibration.md).

## To measure

Reseller figures for TD-8135MG disagree with each other, and the seller stated none of
the electrical limits, so establish these on the bench:

- [ ] **Usable pulse width range.** Sweep gently from centre and find where motion stops
      at each end. This defines the real safe clamp, replacing the assumed 500–2500 µs.
- [ ] **Stall current at 6 V**, with the bench supply's limit as protection. Determines
      the robot's supply sizing, currently estimated at 20–30 A for twelve.
- [ ] **Actual travel for a commanded 180°**, which calibrates `conversion` — the one
      number that makes commanded angle equal real angle.
- [ ] Whether the seller's 0.15 s/60° or the reference 0.32 s/60° is closer to reality,
      before tuning gait speed.
