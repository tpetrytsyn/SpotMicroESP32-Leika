# ZOP Power 2S 1500 mAh 40C LiPo battery

The robot's main battery. Sourced as "Акумулятор ZOP Power Li-Po 2s 7.4v 1500 mAh 40C
конектор XT60".

**This is an RC hobby pack and has no BMS.** Nothing inside it limits current or stops
over-discharge, so both jobs fall to the robot's wiring and firmware — see
[No BMS: what the robot must provide](#no-bms-what-the-robot-must-provide).

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[listing]** = stated in the product name, **[chemistry]** = standard LiPo figures,
**[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Chemistry | Lithium polymer (LiPo) | **[listing]** |
| Configuration | 2S — two cells in series | **[listing]** |
| Nominal voltage | 7.4 V (3.7 V/cell) | **[listing]** |
| Fully charged | 8.4 V (4.2 V/cell) | **[chemistry]** |
| Capacity | 1500 mAh | **[listing]** |
| Discharge rating | 40C → **60 A continuous** | **[listing]**, amperage computed |
| Burst rating | not stated | — |
| Power connector | XT60 | **[listing]** |
| Balance connector | JST-XH 3-pin | **[inferred]** — standard on 2S RC packs |
| Protection circuit | **none** | **[inferred]** — confirm under the shrink wrap |
| Dimensions, weight, lead gauge | not stated | — |

## No BMS: what the robot must provide

An RC LiPo leaves protection out on purpose, so the pack can supply its full rated
current. On a robot that has two consequences:

- **A fuse is mandatory.** 40C × 1.5 Ah is 60 A continuous and far more into a short —
  enough to heat the 14 AWG trunk red. A **15 A blade fuse**
  ([holders and fuses](Daier-blade-fuse-holders.md)) goes directly after the XT60,
  before the switch and everything else. It is the only thing that limits a short.
- **Over-discharge is the robot's job.** Below ~3.0 V/cell a LiPo is permanently damaged
  and can swell. Nothing in the pack prevents it.
  - **Measurement is decided:** an [ADS1115](ADS1115-16bit-ADC.md) reads pack voltage
    through a [voltage module](Voltage-sensor-0-25V-divider.md). Pack voltage only — the
    balance charger equalises the cells each charge, and its per-cell display is the check.
  - **What acts on it:** firmware can warn, sit the robot down and deactivate the servos
    at the 7.0 V cutoff, but it cannot cut power — only the
    [rocker](KCD2-201N-B-rocker-switch.md) does. **Switch off after every session.** A LiPo
    alarm on the balance lead is still under discussion — see
    [Pending decisions](../wiring.md#pending-decisions).

## Use in this project

Feeds both converters; the full harness is in [wiring.md](../wiring.md). The
[rocker switch](KCD2-201N-B-rocker-switch.md) is the only on/off control.

```
pack → XT60 → fuse 15 A → rocker ──────► ACS712 ─┬─► SZBK07 → 6 V servo rail
                                                 └─► fuse 2 A → CN3903 → 5 V ESP32
```

| | |
| --- | --- |
| Load ceiling | ~12 A, set by the SZBK07's CC — **8C**, a fifth of the rating |
| Runtime | **~15 – 25 min** walking, assuming 3 – 5 A average and 80% usable capacity **[inferred]** |
| Headroom | Ample. Capacity, not discharge rate, is what limits this pack |

### Voltage thresholds

| Pack voltage | Per cell | Meaning |
| --- | --- | --- |
| 8.4 V | 4.2 V | Fully charged |
| 7.4 V | 3.7 V | Nominal |
| **7.0 V** | **3.5 V** | **Working cutoff under load** — stop walking and sit down |
| 6.6 V | 3.3 V | Resting floor — do not go below |
| < 6.0 V | < 3.0 V | Damage |

The SZBK07 drops out of regulation around 7.5 – 8 V, so by the 7.0 V cutoff the servo
rail is already following the pack down. Stopping there costs little torque that was not
already gone.

### Wiring

- **XT60 on the robot side** matching the pack. By RC convention the pack carries the
  female half so its live contacts are shrouded **[inferred]** — check your pack's gender.
- **The bench supply gets the same XT60**, so calibration runs through the final connector.
- **Fuse holder immediately after the connector**, on the `+` lead — the
  [Daier F114-C](Daier-blade-fuse-holders.md)'s 12 AWG lead solders straight into the XT60.
- **Balance lead** is not wired into the robot. It stays free for the charger, and for a
  LiPo alarm if one is chosen.

## Handling

- **Charge only on a balance charger**, through the balance lead, inside a LiPo bag, and
  never unattended.
- **Store at ~3.8 V/cell** (the charger's *Storage* mode) if unused for more than a few days.
- **Mount it where a fall cannot puncture it.** A punctured LiPo can catch fire.
- **Retire a puffed pack.** Swelling is permanent damage, not a cosmetic issue.

## Open items

- [ ] **Confirm there is no protection board** under the shrink wrap.
- [ ] **Check the XT60's gender** before ordering the robot-side mating half.
- [ ] **Measure weight and dimensions** — needed for body fit and centre of mass.
- [ ] **Lead gauge** — 16 AWG is common on 1500 mAh packs; fine at 12 A over its short length.
- [ ] **Real runtime** once walking draw can be measured.
