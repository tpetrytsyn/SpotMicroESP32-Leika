# ZOP Power 2S 1500 mAh 40C LiPo battery

The main battery. **Owned.** Sourced as "Акумулятор ZOP Power Li-Po 2s 7.4v 1500 mAh 40C конектор
XT60".

**An RC pack with no BMS** — nothing inside limits current or stops over-discharge:

- **The fuse is mandatory.** 40C × 1.5 Ah is 60 A continuous and far more into a short. The 15 A
  [fuse](Daier-blade-fuse-holders.md) directly after the XT60 is all that limits it.
- **Over-discharge is the robot's job.** Below ~3.0 V/cell a LiPo is damaged and can swell. The
  ADS1115 reads pack voltage, but nothing in the firmware acts on it yet — and firmware can never
  cut power; only the [rocker](KCD2-201N-B-rocker-switch.md) does.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Chemistry | LiPo, 2S | listing |
| Voltage | 7.4 V nominal, 8.4 V full | listing + chemistry |
| Capacity | 1500 mAh | listing |
| Discharge | 40C → 60 A continuous; burst not stated | listing |
| Connectors | XT60; balance JST-XH 3-pin | listing + inferred |
| Protection | none — confirm under the wrap | inferred |
| Size, weight, lead gauge | not stated | — |

## Voltage thresholds

| Pack | Per cell | Meaning |
| --- | --- | --- |
| 8.4 V | 4.2 V | Full |
| 7.4 V | 3.7 V | Nominal |
| **7.0 V** | **3.5 V** | **Working cutoff under load** — stop walking and sit down |
| 6.6 V | 3.3 V | Resting floor — don't go below |
| < 6.0 V | < 3.0 V | Damage |

The SZBK07 drops out around 7.5–8 V **[inferred]**, so by 7.0 V the servo rail is already following
the pack ([wiring.md](../wiring.md#battery-and-headroom)).

## Use in this project

- **Load ceiling** ~12 A (the SZBK07's CC): 8C, a fifth of the rating. Capacity is the limit, not
  discharge rate.
- **Runtime** ~15–25 min walking, at 3–5 A average and 80 % usable **[inferred]**.
- **Left on, it drains:** deactivated, the ESP32 and the live servo rail still draw ~0.15–0.35 A
  **[inferred]** — from the 7.0 V cutoff to damage in roughly 15–90 minutes.
  **Rocker off and XT60 unplugged after every session.**

**Wiring:**
- The robot takes the XT60's other half; the bench-supply lead takes the **pack's** half, so
  calibration runs through the final connector. By RC convention the pack carries the female
  (shrouded) half **[inferred]** — check yours.
- The F114-C's 12 AWG lead solders straight into the XT60 `+`.
- **The balance lead never connects to the robot** — it's for the charger (and a LiPo alarm, if
  one is chosen).

## Handling

- Charge only on a balance charger, out of the robot, inside a LiPo bag, never unattended.
- Store at ~3.8 V/cell (the charger's *Storage* mode) if unused for more than a few days.
- Mount it where a fall can't puncture it.
- Retire a puffed pack — swelling is permanent damage.

## Open items

- [ ] Confirm there's no protection board under the wrap
- [ ] The XT60's gender, before buying the mating halves
- [ ] Weight and dimensions — body fit and centre of mass
- [ ] Lead gauge — 16 AWG is common, fine at 12 A over its length
- [ ] Real runtime, once walking draw is measured
