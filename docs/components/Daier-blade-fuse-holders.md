# Daier F114-C / F117-C blade fuse holders and fuses

Inline holders for **medium (19 mm, ATO/ATC) blade fuses**, one per fused branch.
**Decided, not yet bought.** Both from RCS Components (РАДІОМАГ):

- ["Тримач запобіжника medium 19мм, F114-C 12AWG Daier"](https://www.rcscomponents.kiev.ua/product/trymach-zapobizhnyka-medium-19mm-f114-c-12awg_211721.html)
  — the main fuse
- ["Тримач запобіжника medium 19мм, F117-C Daier"](https://www.rcscomponents.kiev.ua/product/trymach-zapobizhnyka-medium-19mm-f117-c_211722.html)
  — the CN3903 branch fuse

**Both use the same fuse size**, so one stock of spares serves both — which is also the
trap: see [Label the branch holder](#label-the-branch-holder).

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[listing]** = stated by the seller, **[manufacturer]** = from Daier's product page,
**[inferred]** = reasoned, needs confirmation on hardware.

## Why the robot needs fuses

The [battery](ZOP-Power-2S-1500mAh-LiPo.md) is an RC LiPo with **no BMS**: 40C × 1.5 Ah
is 60 A continuous and far more into a short. Nothing else in the chain limits that. The
SZBK07's CC protects only what is downstream of the converter.

The fuses protect **wiring**, not loads. A stalled servo is the CC pot's job; the 15 A
fuse will not and should not blow for one.

## Specifications

| Item | F114-C — main | F117-C — branch |
| --- | --- | --- |
| Fuse size | Medium 19 mm, ATO/ATC **[listing]** | Medium 19 mm, ATO/ATC **[listing]** |
| Rating | **30 A, 32 V DC** **[manufacturer]** — not stated by RCS | **10 A**, 12/24 V DC **[listing]** |
| Wire | **12 AWG**, 30 cm total **[manufacturer]** | **16 AWG**, 15 cm **[listing]** |
| Sealing | IP55, rubber-sealed cover **[manufacturer]** | Silicone-sealed cover **[listing]** |
| Insulation | T105 (105 °C) **[manufacturer]** | — |
| Dimensions | 44 × 35.5 mm **[listing]** | — |
| Price | 32 ₴ | 32 ₴ |

> **The F114-C rating is Daier's, not the seller's.** RCS lists none; Daier's product
> page states 30 A, while one reseller listing claims 40 A. 30 A is used here. At a 15 A
> fuse the margin is 2× either way.

## Fuses

Standard **medium 19 mm** automotive blade fuses, 32 V. Mini and micro blades do not fit
these holders.

| Branch | Fuse | Load | Protects |
| --- | --- | --- | --- |
| Main, after the XT60 | **15 A** (blue) | ≤ ~12 A, capped by the SZBK07's CC | 14 AWG trunk |
| CN3903 input | **2 A** (grey) | ~0.4 A ESP32 and sensors **[inferred]** | 22 AWG CN3903 feed, the voltage module's sense wire |

To buy from the
[ATT/ATN/ATS 300-piece set](https://www.rcscomponents.kiev.ua/product/zapobizhnyky-avtomobilni-att-atn-ats-nabir-u-lotku-300sht-att-2a-3a-5a-7-5a-10a-15a-20a-25a-30a-35a-40a-atn-2a-3a-5a-7-5a-10a-15a-20a-25a-30a-35a-40a-ats-2a-3a-5a-7-5a-10a-15a-20a-25a-30a-35a-40a_219987.html)
(unbranded), or singly — e.g. the
[Zeeman AMF-15A](https://www.rcscomponents.kiev.ua/product/zapobizhnyk-avtomobilnyi-medium-15a-synii-amf-15a-bf-ats610100_22857.html)
for the main fuse. Keep at least five spares of each rating.

### Choosing the ratings

- **15 A main.** Above the ~12 A CC ceiling, below what 14 AWG carries in short runs.
  Capacitor inrush at switch-on lasts milliseconds and does not blow a blade fuse. If it
  nuisance-blows under hard walking at CC = 12 A, go to **20 A — never higher**; that is
  the limit for 14 AWG.
- **2 A branch.** 1 A is too tight for Wi-Fi transmit peaks; 2 A still opens long before
  22 AWG (~5–7 A) overheats.

## Wiring

```
pack → XT60 → F114-C [15 A] → rocker ──────→ ACS712 ─┬─► SZBK07
                                                     └─► F117-C [2 A] ─┬─► CN3903
                                                                       └─► voltage module
```

The main switch is the [KCD2 rocker](KCD2-201N-B-rocker-switch.md).
Full harness: [wiring.md](../wiring.md).

- **Solder the F114-C straight into the robot-side XT60.** 12 AWG is the gauge the XT60
  solder cup is made for, so no splice is needed.
- **Keep the XT60 → fuse stretch as short as possible.** It is the only unprotected
  conductor on the robot. Shorten the holder's 30 cm lead on that side; trim the rest to
  length on the other.
- **The F117-C sits where the CN3903 feed leaves the trunk**, so the whole 22 AWG run is
  behind it.
- **Tie both holders to the body.** Their weight must not hang on the solder joints under
  walking vibration.

### Label the branch holder

Both holders take the same fuse, and the F117-C is rated only 10 A. **Mark it "2 A"**, so
a spare from the 15 A bag never ends up in the branch.

**Discard any 30 A / 40 A fuses shipped with the F114-C.** Daier bundles them; 30 A
exceeds the 14 AWG trunk's rating.

## Evaluated, not chosen

- **[MTA-UNI-HOLDER](https://www.rcscomponents.kiev.ua/product/trymach-avtomobilnoho-zapobizhnyka-z-drotom-mta-uni-holder_108713.html)**
  (MTA 0300336) — waterproof, 30 A, **4 mm²** leads of 120 mm according to the
  [MTA catalogue](https://www.mta.it/flex/TemplatesUSR/assets/pdf/1500000115000001Cat_AM.pdf),
  p. 58. Electrically fine, but 4 mm² is too thick for an XT60 cup and needs a 14 AWG
  splice, at ~85 ₴.
- **Mini (ATM) holders** — the ones found locally have 1 mm² leads, too thin for the main
  fuse. The Littelfuse `0FHM0001SXJ` (mini, 14 AWG, 20 A, IP67) would suit but is not
  stocked in Ukraine.

## Open items

- [ ] **Confirm what the F114-C ships with** — discard bundled 30 A / 40 A fuses.
- [ ] **Measure walking draw** at CC = 12 A and confirm the 15 A fuse holds.
