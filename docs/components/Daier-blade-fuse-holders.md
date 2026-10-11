# Daier F114-C / F117-C blade fuse holders and fuses

Inline holders for **medium (19 mm, ATO/ATC) blade fuses**. **Decided, not bought.** Both from RCS
Components:

- ["Тримач запобіжника medium 19мм, F114-C 12AWG Daier"](https://www.rcscomponents.kiev.ua/product/trymach-zapobizhnyka-medium-19mm-f114-c-12awg_211721.html)
  — main fuse
- ["Тримач запобіжника medium 19мм, F117-C Daier"](https://www.rcscomponents.kiev.ua/product/trymach-zapobizhnyka-medium-19mm-f117-c_211722.html)
  — logic-branch fuse

The [battery](ZOP-Power-2S-1500mAh-LiPo.md) has no BMS: 40C × 1.5 Ah is 60 A continuous and far
more into a short. The fuses protect **wiring**, not loads — pack current can't exceed ~12 A by
design, so nothing past a working SZBK07 opens the 15 A fuse ([wiring.md](../wiring.md#fuses)).

## Specifications

| Item | F114-C — main | F117-C — branch |
| --- | --- | --- |
| Fuse | medium 19 mm, ATO/ATC | the same |
| Holder rating | **30 A, 32 V DC** — Daier's figure; RCS lists none, one reseller claims 40 A | **10 A**, 12/24 V DC — listing |
| Leads | **12 AWG**, 30 cm total | **16 AWG**, 15 cm |
| Sealing | IP55 rubber cover; T105 insulation | silicone cover |
| Price | 32 ₴ | 32 ₴ |

## Fuses

Standard medium 19 mm blades, 32 V — mini and micro don't fit. From the
[ATT/ATN/ATS 300-piece set](https://www.rcscomponents.kiev.ua/product/zapobizhnyky-avtomobilni-att-atn-ats-nabir-u-lotku-300sht-att-2a-3a-5a-7-5a-10a-15a-20a-25a-30a-35a-40a-atn-2a-3a-5a-7-5a-10a-15a-20a-25a-30a-35a-40a-ats-2a-3a-5a-7-5a-10a-15a-20a-25a-30a-35a-40a_219987.html)
or singly (e.g. the
[Zeeman AMF-15A](https://www.rcscomponents.kiev.ua/product/zapobizhnyk-avtomobilnyi-medium-15a-synii-amf-15a-bf-ats610100_22857.html)).
Keep five spares of each.

| Branch | Fuse | Load | Protects |
| --- | --- | --- | --- |
| Main, after the XT60 | **15 A** (blue) | ≤ ~12 A, capped by the SZBK07's CC | 14 AWG trunk |
| Logic branch | **2 A** (grey) | ~0.4 A **[inferred]** | 22 AWG feed, voltage-module tap |

- **15 A:** above the ~12 A ceiling, below 14 AWG's limit. Capacitor inrush lasts milliseconds and
  won't blow a blade. If it nuisance-blows under hard walking at CC = 12 A, go to 20 A — never higher.
- **2 A:** the set's smallest, ~4× the branch's ~0.5 A peak; it opens long before 22 AWG (~5–7 A)
  overheats. Jumper wire is too thin for it — the voltage-module tap is 22 AWG.

## Wiring

- **Solder the F114-C's 12 AWG straight into the XT60 cup**, shortened to the minimum — that
  stretch is the only unfused conductor. Trim the other side to length.
- The F117-C sits where the logic branch leaves the + split, so the whole 22 AWG run is behind it.
- Tie both holders down; their weight must not hang on the solder joints.
- **Mark the F117-C "2 A"** — both take the same fuse, and a 15 A spare there would leave the branch
  unprotected.
- **Discard the 30/40 A fuses** Daier bundles with the F114-C — above the trunk's rating.

## Evaluated, not chosen

- **[MTA-UNI-HOLDER](https://www.rcscomponents.kiev.ua/product/trymach-avtomobilnoho-zapobizhnyka-z-drotom-mta-uni-holder_108713.html)**
  (MTA 0300336): waterproof, 30 A, 4 mm² leads per the
  [MTA catalogue](https://www.mta.it/flex/TemplatesUSR/assets/pdf/1500000115000001Cat_AM.pdf) p. 58
  — too thick for an XT60 cup, needs a splice, ~85 ₴.
- **Mini (ATM) holders:** the local ones have 1 mm² leads, too thin for the main fuse. The
  Littelfuse `0FHM0001SXJ` (mini, 14 AWG, 20 A, IP67) would suit but isn't stocked in Ukraine.

## Open items

- [ ] What the F114-C ships with — discard 30/40 A
- [ ] Walking draw at CC = 12 A, and whether the 15 A fuse holds
