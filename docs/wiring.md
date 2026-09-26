# Wiring and power

The robot's power and logic harness: battery to servos, converters, grounding, I2C and
monitoring. [Servo calibration](servo-calibration.md) runs on this harness unchanged.

**This is the final wiring.** Topology, gauge and converter settings are specified for the
assembled robot from the first bench session, so nothing is rebuilt at assembly and the
harness is tested early. Where a bench value must differ — the bench supply standing in for
the battery, and a temporarily low [current limit](#current-limit-cc) — it is called out.

Confidence is marked throughout, following
[ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[code]** = read from this repo, **[datasheet]** = from the part's spec,
**[inferred]** = reasoned and needs confirmation on hardware.

## Status

| State | Parts |
| --- | --- |
| **Core** | [SZBK07](components/SZBK07-buck-converter.md), [CN3903](components/CN3903-5V-buck-module.md), PCA9685, [ESP32-S3-CAM](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md), servo distribution board, star point |
| **Power additions — owned** | [2S 1500 mAh LiPo](components/ZOP-Power-2S-1500mAh-LiPo.md), [ADS1115](components/ADS1115-16bit-ADC.md) |
| **Power additions — decided** | [Fuses and holders](components/Daier-blade-fuse-holders.md), [rocker switch](components/KCD2-201N-B-rocker-switch.md), [ACS712](components/ACS712-30A-current-sensor.md), [voltage module](components/Voltage-sensor-0-25V-divider.md) |
| **Decided against** | Relay, LED button — see [Not fitted](#not-fitted) |
| **Under discussion** | LiPo alarm — see [Pending decisions](#pending-decisions) |

## Materials

| # | Item | Why |
| --- | --- | --- |
| 1 | **PCA9685** 16-channel PWM board, I2C | The driver targets `0x40` **[code]** |
| 2 | **[SZBK07](components/SZBK07-buck-converter.md)** 300 W CC/CV buck | battery → 6 V servo rail, with its own current limit |
| 3 | **[CN3903](components/CN3903-5V-buck-module.md)** fixed 5 V buck | battery → 5 V for the ESP32 and ACS712; no adjustment |
| 4 | **XT60**, robot-side half | Mates the [battery](components/ZOP-Power-2S-1500mAh-LiPo.md); a second one for the bench supply |
| 5 | **[F114-C + 15 A, F117-C + 2 A](components/Daier-blade-fuse-holders.md)** | Main and CN3903-branch fuses — see [Fuses](#fuses) |
| 6 | **[KCD2-201N-B rocker](components/KCD2-201N-B-rocker-switch.md)** | Main power switch — see [Main switch](#main-switch) |
| 7 | **Servo distribution board** — perfboard + 12 × 3-pin headers | Servos plug in with their stock connectors; carries all servo current. See [Building the servo board](#building-the-servo-board) |
| 8 | **14 AWG solid bare copper** | The board's two bus rails |
| 9 | **14 AWG stranded** | The trunk runs — see [Wire gauge](#wire-gauge) |
| 10 | **3 × WAGO 221** 5-way lever connectors | The ground star point — see [The ground star point](#the-ground-star-point) |
| 11 | **1000 – 2200 µF** electrolytic, ≥ 10 V | On the servo board, across the two rails at the centre feed |
| 12 | **[ADS1115](components/ADS1115-16bit-ADC.md)** | Four analog inputs over I2C — see [Monitoring](#monitoring) |
| 13 | **[ACS712 30 A](components/ACS712-30A-current-sensor.md)** + 10 kΩ | Pack current |
| 14 | **[Voltage module](components/Voltage-sensor-0-25V-divider.md)** 0–25 V | Pack voltage |
| 15 | Crimp ferrules | For the ACS712's screw terminal |
| 16 | Jumper wires | `3V3`, `SDA`, `SCL`, sensor signals |

## Power path

```
 2S LiPo ─► XT60 ─► F114-C ─► rocker ──────► ACS712 ──────┬─────────────────► SZBK07 IN+
 (bench:     12 AWG  15 A      KCD2, 1 pole    IP− → IP+    │ 14 AWG            CV = 6.0 V
  supply on                                    (reversed)   │                   CC = set limit
  its own XT60)                                             │                   OUT+ ─► servo board
                                                            │                        centre feed, 14 AWG
                                                            └─► F117-C ─┬──────► CN3903 IN+
                                                                2 A     │ 22 AWG  fixed 5.0 V
                                                                        │         OUT+ ─► ESP32 VCC
                                                                        │              └► ACS712 VCC
                                                                        └──────► voltage module VCC

 pack − ────────────────────────────────────────────────────► ★ star point, 14 AWG

 balance lead ── NOT CONNECTED to the robot; charger only
```

The bench supply stands in for the 2S pack — the configuration this project's BOM
specifies ([1_components.md](1_components.md)) — on **its own XT60**, so the connector and
the fuse are in circuit from the first session.

## Servo distribution

**Servo power and ground come from a purpose-built distribution board and never touch the
PCA9685**, which carries signal only.

```
   SZBK07 OUT+ (6.0 V), 14 AWG
            │      SERVO DISTRIBUTION BOARD — 12 × 3-pin headers
            │  ┌───────────────────────────────────────────────┐
            │  │ sig  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪       │──► PCA9685 CH0..11
            └─►│ 6 V  ▪──▪──▪──▪──▪──▪──▪──▪──▪──▪──▪──▪       │    (12 thin wires)
               │ GND  ▪──▪──▪──▪──▪──▪──▪──▪──▪──▪──▪──▪       │──► star point, 14 AWG
               └─────────────────────▲─────────────────────────┘
                    feed both rails at the CENTRE, not the end
                    1000–2200 µF across the two rails, right here
```

### The four rules that make it work

1. **`V+` stays unconnected.** The PCA9685 IC needs only `VCC`, `GND`, `SDA` and `SCL` —
   it never carries servo current. Leaving `V+` off keeps every amp off the board's thin
   copper, which is the whole point of this topology.
2. **Servo ground returns on the distribution board's own rail, not through the PCA9685.**
   Returning it through the PCA9685 would move the bottleneck to its ground trace and
   achieve nothing. The board's rail then reaches the star point as one 14 AWG conductor.
3. **One wire from the star point to `PCA9685 GND`.** PWM is referenced to ground; without
   it the servos have no valid reference and will jitter or ignore commands entirely. That
   wire carries reference current only, so it can be thin.
4. **Both converters land `IN−` *and* `OUT−` on the star point.** Both modules are
   non-isolated, so `IN−` and `OUT−` are already the same copper inside — wiring both
   externally is what keeps return current off the converter's own ground pour, where it
   would drop voltage across the feedback reference and spoil regulation. Their job here
   is to **bond** the converter's ground to the star, not to carry the rail; see
   [Where the current actually flows](#where-the-current-actually-flows).

### Building the servo board

Twelve 3-pin headers on perfboard, wired as three rails. Servos plug in with their **stock
connectors** — there is no harness to build and no extension cables to modify.

This is the PCA9685's power distribution rebuilt with copper sized for the job, which is
the entire reason `V+` is left unconnected. **Build it with thin copper and you have
recreated the problem with extra steps.**

**Lay bare solid 14 AWG copper along the 6 V and GND rows**, soldered to every pin along
its length and flooded with solder. The bus wire is the conductor; the pad copper is only
attachment. Perfboard done this way beats a cheap 1 oz PCB — 12 A needs pours wider than
you would guess.

**Feed both rails at the centre, not at one end.** End-feeding makes the far servo's
current traverse the whole rail; centre-feeding halves the copper any current crosses.
Same reasoning as [reinforcing a PCA9685](#if-you-ever-revert-to-powering-servos-through-the-board),
applied where it is cheap to get right.

Three things to design around:

- **Pin ampacity is marginal at stall.** Standard 2.54 mm headers are ~3 A per pin against
  a 2.6–3.4 A stall. At running current (0.15–0.3 A) it is nothing, stall is transient, and
  CC folds back — the PCA9685's own headers have the identical limit. If buying anyway,
  headers rated ≥ 5 A cost the same.
- **Nothing stops a servo going in backwards.** Plain 0.1" headers do not enforce the
  connector's polarity ridge, and reversed means reverse voltage into the servo. Use
  shrouded or keyed headers, or mark the ground row unmistakably.
- **Friction-fit connectors walk off under vibration.** Servo plugs have no latch. Plan
  retention from the start — a zip tie or printed bar across the row.

Mount the board and strain-relieve the twelve cables so their weight never hangs on the
pins. The electrolytic goes **on this board**, across the two rails at the centre feed —
that is the closest it can physically get to the servos.

## Wire gauge

**These are the assembled-robot figures, used from the first bench session so nothing is
rewired later.** Calibration draws a fraction of this, but building to the bench load
would mean replacing the trunk at assembly.

Two currents, an order of magnitude apart, and the mistake is giving them the same wire:

- **Each servo** draws at most its stall current, **2.6–3.4 A**, and only briefly. That
  gauge is **not yours to choose**: the servo's moulded lead is roughly 22 AWG and sits in
  series with whatever it plugs into. It is also sufficient — ~16 mΩ over a 30 cm lead is
  ~55 mV at stall.
- **The trunk** — pack leads, the board's two bus rails, and the board's ground back to
  the star — carries **all twelve at once**.

What sizes the trunk is **the CC pot, not the load**. That is the point of the SZBK07: CC
is a hard ceiling on the rail, so the trunk can never carry more than it allows no matter
what the servos attempt. Size for the CC setting and the question is closed — which also
means the theoretical ~36 A of twelve stalled servos never needs wiring for, provided CC
is actually set.

| Conductor | Carries | Gauge |
| --- | --- | --- |
| XT60 → F114-C | full pack draw, **unprotected** — keep it short | **12 AWG** (the holder's lead) |
| F114-C → ACS712 → split | full pack draw | **14 AWG** |
| Pack `−` → star point | ~11 A at 2S — **the heaviest single conductor** | **14 AWG** |
| Split → `SZBK07 IN+` | servo rail's input | **14 AWG** |
| `SZBK07 OUT+` → board 6 V rail | whole servo rail, capped by CC | **14 AWG** |
| The board's 6 V and GND rails | the same | **14 AWG** solid, bare |
| Board GND rail → star point | the same, returning | **14 AWG** |
| `SZBK07 IN−` / `OUT−` → star point | only the net, ~1 A — but keep it heavy | **14 AWG** |
| Header → servo, ×12 | one servo, ≤ 3.4 A | stock servo lead |
| Split → F117-C | CN3903 branch | 16 AWG (the holder's lead) |
| F117-C → `CN3903`, and its output | ESP32 and sensors, < 0.6 A | 22 AWG |
| Star → `PCA9685 GND` | reference current only | 24–26 AWG |
| Star → sensor ground | ~10 mA | 22 AWG |
| Voltage module inputs, sensor signals, `3V3`, `SDA`, `SCL` | signal | jumper wire |

At these lengths the constraint is **heat, not voltage drop**. 18 AWG at 15 A over a 20 cm
run drops only ~60 mV, irrelevant on a 6 V rail — but it sits at its 16 A chassis limit
with no margin. 14 AWG at the same current is unremarkable, which is why it is the figure
here rather than the 18 AWG that would survive the bench.

**Every thin wire sits behind a fuse sized for it.** That is why the voltage module taps
the CN3903 branch after the 2 A fuse, not the 14 AWG trunk — see
[the module doc](components/Voltage-sensor-0-25V-divider.md#why-it-taps-after-the-2-a-fuse).

### Where the current actually flows

Worth doing the node accounting once, because the intuitive answer is wrong and it changes
which conductor you terminate most carefully.

`IN−` and `OUT−` are the same copper inside a non-isolated buck, so the two wires running
from the SZBK07 to the star point are **in parallel between the same pair of nodes**. At
the star: 12 A arrives from the board's ground rail, and the SZBK07's *input* current
leaves toward the pack. Because 7.4 V → 6 V is a shallow step-down, that input current is
roughly **11 A** — so only the difference, about **1 A**, flows in `IN−`/`OUT−` at all.

The heavy path is **board ground → star → pack `−` → battery → pack `+` → `SZBK07 IN+`**,
straight through. The converter's ground wires are a bond, not a bus.

This is the same fact as "at 2S the pack current is close to the rail current, since the
step-down ratio is small", seen from the ground side. At 3S the cancellation is weaker and
`OUT−` would carry appreciably more.

Two consequences:

- **The pack leads are the critical conductors**, not the converter's. Size and terminate
  those with the most care.
- **Keep `IN−`/`OUT−` at 14 AWG anyway.** The cancellation is a steady-state result that
  assumes the pack lead is intact; if it degrades, all of it lands here. The wire is free.

## The ground star point

Nine conductors on **3 × WAGO 221 5-way**, chained with 3 cm of 14 AWG: one block for the
heavy returns, two for the light ones.

```
 HEAVY block            LIGHT block 1            LIGHT block 2
 ├─ servo board GND     ├─ link from HEAVY       ├─ link from LIGHT 1
 ├─ pack −              ├─ link to LIGHT 2       ├─ ESP32 GND
 ├─ SZBK07 OUT−         ├─ CN3903 OUT−           ├─ PCA9685 GND
 ├─ SZBK07 IN−          ├─ CN3903 IN−            ├─ sensor ground
 └─ link to LIGHT 1     └─ (spare)               └─ (spare)
```

| # | Conductor | Why it is there | Carries | Gauge |
| --- | --- | --- | --- | --- |
| 1 | Servo board GND rail | All twelve returns, aggregated on the board | **12 A** | 14 AWG |
| 2 | Pack `−` | The battery's return | **~11 A** | 14 AWG |
| 3 | `SZBK07 OUT−` | Bonds converter ground to the star | ~1 A | 14 AWG |
| 4 | `SZBK07 IN−` | Same node, parallel with #3 | ~1 A | 14 AWG |
| 5 | `CN3903 OUT−` | Closes the 5 V output loop | < 0.6 A | 22 AWG |
| 6 | `CN3903 IN−` | Closes its input loop | ~0.4 A | 22 AWG |
| 7 | `ESP32 GND` | The ESP32's own supply return | few hundred mA | 22 AWG |
| 8 | `PCA9685 GND` | PWM and I2C reference | ~10 mA | 24–26 AWG |
| 9 | Sensor ground | ADS1115, ACS712 and the voltage module — see [Monitoring](#monitoring) | ~10 mA | 22 AWG |

Keeping the heavy terminations together is convenient rather than electrically necessary,
but it stops #8's 26 AWG sharing a clamp with 14 AWG. The two spare positions leave room
for later additions.

**The balance lead is not on this list, deliberately.** Its pin 1 is the pack's `−`,
already here through the XT60; wiring it too would put the balance lead's thin wire in
parallel with conductor #2.

#### Why WAGO rather than a screw terminal

Under vibration the ranking is **spring clamp > crimp > solder > screw**. A spring clamp
holds force by design and needs no re-torque; a screw holds it by friction, and loosening
is its characteristic failure mode. The 221 series is rated 32 A IEC / 20 A UL over
0.2–4 mm² (24–12 AWG), which brackets everything in the table above.

**No ferrules** — the 221 cage clamp is built for fine-stranded wire directly. **Not** the
2273/773 push-in series, which is solid-conductor only; every wire in this robot is
stranded. Use the mounting carriers, and strain-relieve the bundle a few centimetres back:
the clamp grips the conductor, not the cable.

> **On chaining.** Blocks joined by links look like the daisy-chaining star grounding
> forbids. It is not, at this scale: 3 cm of 14 AWG is ~0.25 mΩ, so 12 A across it is
> **3 mV**. Star grounding here protects against the gross error — routing 12 A through a
> PCA9685 ground trace, hundreds of times more resistance — not against millivolts. Once
> the returns land in one cluster, you have already won.

## Fuses

The [battery](components/ZOP-Power-2S-1500mAh-LiPo.md) has no BMS, so the fuses are the
only thing between a short and 60 A+. Parts, ratings and the reasoning are in
[Daier-blade-fuse-holders.md](components/Daier-blade-fuse-holders.md).

| Fuse | Where | Protects |
| --- | --- | --- |
| **15 A** in the F114-C | directly after the XT60, soldered into it | the 14 AWG trunk |
| **2 A** in the F117-C | where the CN3903 branch leaves the trunk | the 22 AWG branch, the voltage module's sense wire |

They protect wiring, not loads: a stalled servo is the CC pot's job.

## Main switch

A [KCD2-201N-B rocker](components/KCD2-201N-B-rocker-switch.md) in pack `+`, between the
F114-C and the ACS712. **It is the robot's only on/off control**: off means everything
downstream — converters, ESP32, sensors — is dead, and the pack sees **zero** draw.

- **One pole**, 14 AWG soldered to its tabs; the second pole stays unconnected.
- **Switch `+` only.** Ground stays continuous to the star.
- **No DC rating is given**; at 2S the pack is below the voltage that sustains an arc, so
  breaking ~12 A is acceptable **[inferred]**. Revisit at 3S.
- **Deactivate the servos in the UI before switching off** — the rocker cuts servo power
  instantly and a standing robot drops.

Leika's BOM lists a main power switch as required ([1_components.md](1_components.md));
this is it, with no soft-power layer on top — see [Not fitted](#not-fitted).

## Logic and I2C

```
   ESP32-S3-CAM              PCA9685 (0x40)            ADS1115 (0x48)
   ────────────              ──────────────            ──────────────
    3V3  ──────────────────►  VCC  ──────────────────►  VDD
    IO47 ── SDA ───────────►  SDA  ──────────────────►  SDA
    IO41 ── SCL ───────────►  SCL  ──────────────────►  SCL
                              GND ◄── star              GND ◄── star (sensor ground)
                              V+  ── NOT CONNECTED      ADDR ── GND
                              CH0..CH11 ──► board signal row
```

| ESP32 pin | Goes to | Note |
| --- | --- | --- |
| `IO47` | SDA — PCA9685, ADS1115 | `SDA_PIN=47` in the `s3cam` env **[code]** |
| `IO41` | SCL — PCA9685, ADS1115 | `SCL_PIN=41` in the `s3cam` env **[code]** |
| `3V3` | PCA9685 `VCC`, ADS1115 `VDD` | **3.3 V only** — see [Cautions](#cautions) |
| `GND` | star point | The ESP32's supply return |

Both pins are free on this board and touch neither the camera DVP bus nor the memory
pins. `IO14` is reserved for the WS2812 and stays clear.

Both breakouts carry their own I2C pull-ups, so no external resistors are needed; in
parallel they come to roughly 3–5 kΩ, fine at this bus length **[inferred]**.

## Monitoring

Pack voltage and pack current, read over I2C. The ESP32 has no usable analog input — ADC1
is taken by the camera, ADC2 is unreliable under Wi-Fi — so everything goes through the
[ADS1115](components/ADS1115-16bit-ADC.md).

```
   ACS712 (reversed)       voltage module
   ────────────────        ──────────────
   VCC ◄── CN3903 5 V      VCC ◄── after F117-C
   OUT ── 10 kΩ ──► A1     S ─────────────► A0
   GND ─┐                  − ─┐
        └──────────────────────┴──► ADS1115 GND ◄── star (#9)
```

| ADS1115 | Signal | At 8.4 V / 12 A | Component |
| --- | --- | --- | --- |
| `A0` | pack ÷ 5 | 1.68 V | [voltage module](components/Voltage-sensor-0-25V-divider.md) |
| `A1` | 2.5 V − 66 mV/A | 1.71 V | [ACS712](components/ACS712-30A-current-sensor.md), reversed |
| `A2` | free | — | reserved for per-cell monitoring, [not fitted](components/Voltage-sensor-0-25V-divider.md#per-cell-monitoring--not-fitted) |
| `A3` | free | — | optionally the 5 V rail, for ACS712 zero correction |

Every input stays under 3.3 V, the ADS1115's limit on a 3.3 V supply.

**Pack voltage only, no per-cell reading.** The balance charger equalises the cells on
every charge, and the 7.0 V cutoff leaves margin for drift; the charger's per-cell display
is the check. Nothing on the robot connects to the balance lead.

**One sensor ground, not three star conductors.** The two sensor modules take their
ground from the ADS1115's `GND` pin, which has the single wire to the star. They draw
~10 mA together, so the local link costs microvolts — and they then share exactly the
reference the ADS1115 measures against.

Nothing in the firmware reads any of this yet — see
[the ADS1115 doc](components/ADS1115-16bit-ADC.md#firmware).

## Servo rail

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

### The battery: 2S LiPo

This project's BOM specifies a **7.6–8.4 V** pack — "4x 18650 in 2s2p configuration, but
other people have 2s LiPos" ([1_components.md](1_components.md)) — and
[spot.md](spot.md) records a maximum battery voltage of 8.4 V. This build runs on a
[2S 1500 mAh 40C LiPo](components/ZOP-Power-2S-1500mAh-LiPo.md).

A 2S pack does not sit still, and the LiPo thresholds sit higher than an 18650's:

| State | Pack voltage | SZBK07 headroom over 6 V | CN3903 headroom over 5 V |
| --- | --- | --- | --- |
| Fully charged | 8.4 V | 2.4 V — fine | 3.4 V — fine |
| Nominal | 7.4 V | 1.4 V — marginal | 2.4 V — fine |
| **Working cutoff** (3.5 V/cell, under load) | **7.0 V** | 1.0 V — below spec | 2.0 V — fine |
| Resting floor (3.3 V/cell) | 6.6 V | none | 1.6 V — fine |
| Damage (3.0 V/cell) | 6.0 V | none | 1.0 V — marginal |

### What that means

**The SZBK07 will stop regulating before the battery is empty.** Both converters are
bucks needing roughly 1.5–2 V of headroom, and a discharging 2S pack crosses that
threshold around 7.5–8 V.

This degrades gracefully rather than failing: once it drops out, the converter becomes
roughly a pass-through and the servo rail follows the pack down. The servos are rated
**4.8–7.4 V**, so a rail sagging from 6.0 V toward 5.5 V stays in spec — they get weaker
and slower, not damaged. Budget for less torque near the end of a charge — by the 7.0 V
cutoff there is little left to lose by stopping.

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

### Current limit (CC)

At 2S the pack current is close to the rail current, since the step-down ratio is small.
That is the flip side of the good headroom a higher pack would give: **3S would draw
roughly half the amps for the same power**, which matters for wiring gauge and connector
choice.

| Phase | CC setting |
| --- | --- |
| Calibrating one servo | **~1.5 A** — temporary; see [servo-calibration.md](servo-calibration.md#current-limit-during-calibration) |
| Assembled robot | **~12 A** — the figure the 14 AWG trunk and 15 A fuse are sized for |

The 12 A is a ceiling, not a prediction: it sits under the SZBK07's dependable 15 A
([component doc](components/SZBK07-buck-converter.md)) and under the trunk's rating, while
staying clear of real walking peaks. Confirm it against [measured](#open-questions)
walking draw — the ACS712 is there to measure it — and lower it if the measurement
allows: a tighter limit is a better backstop. The chain is: measure the load, set CC above
it, size the wire to CC.

The pack is not the constraint: 12 A is **8C** against the LiPo's 40C rating. Its
1500 mAh capacity is — see [the battery doc](components/ZOP-Power-2S-1500mAh-LiPo.md).

### Thermal

At calibration currents the SZBK07 runs cold. Its 300 W rating assumes forced air, and
the seller quotes both 15 A and 20 A without resolving which — see
[the component doc](components/SZBK07-buck-converter.md). Check the heatsinks by hand
after sustained multi-servo load.

## Commissioning

Both converters ship at arbitrary settings, and the SZBK07 can put out **36 V**. Set them
with **nothing connected downstream**.

1. Bench supply to 7.4 V, current limit low (~0.5 A) for the first power-up. Power enters
   through the XT60 and the 15 A fuse, as the battery's will.
2. **SZBK07 alone**: CC pot to minimum, then raise **CV until the unloaded output reads
   6.0 V**. Confirm the polarity of `OUT+`/`OUT−` with the meter before trusting the
   silkscreen.
3. **CN3903 alone**: nothing to adjust, but **meter the unloaded output and confirm
   5.0 V** before it goes anywhere near the ESP32 — a fixed module built with the wrong
   feedback divider cannot be corrected.
4. Power down. Connect `SZBK07 OUT+` to the servo board's centre feed, the ESP32's
   `VCC` to `CN3903 OUT+`, all nine conductors to the
   [ground star](#the-ground-star-point), and the logic wiring.
   **`PCA9685 V+` stays unconnected** — it is not part of this build.
5. **Before connecting ADS1115 `SDA`/`SCL`, meter them against ground: 3.3 V**, not 5 V.
6. **Check the monitoring inputs with the meter**, ahead of any firmware: `A0` ≈ pack ÷ 5,
   `A1` ≈ 2.5 V minus a little for the ESP32's draw. Anything above 3.3 V on these pins is
   a wiring fault.
7. Raise the bench limit to suit the work, and set the SZBK07's **CC** — see
   [Current limit](#current-limit-cc).

## Cautions

**Set the SZBK07's output before connecting anything downstream.** Its CV pot can deliver
**36 V** and it ships at an arbitrary setting. Measure 6.0 V on the unloaded output first
— see [Commissioning](#commissioning).

**Meter the CN3903 too.** It is fixed at 5 V with nothing to adjust, which is why it is
the documented choice, but confirm 5.0 V unloaded before connecting the ESP32. There is no
pot to correct a module built wrong.

**The PCA9685's `VCC` and the ADS1115's `VDD` must be 3.3 V, not 5 V.** Both breakouts'
I2C pull-ups go to that pin, so a 5 V supply idles SDA/SCL at 5 V. The ESP32-S3 is **not
5 V tolerant on GPIO** — this damages the chip. Feed both from `3V3`.

**Nothing on the robot connects to the balance lead.** Its pin 1 is the pack's `−` and
pin 3 its `+`; wiring either duplicates a main lead through a thin wire. If per-cell
monitoring is ever added, only the middle pin connects — see
[the voltage module doc](components/Voltage-sensor-0-25V-divider.md#per-cell-monitoring--not-fitted).

**Keep the XT60 → fuse stretch as short as possible.** It is the only unprotected
conductor on the robot.

## Not fitted

Decided against, with the reasoning, so it is not re-litigated:

| Item | Why not |
| --- | --- |
| **Relay** (ESP32-controlled servo power) | Adds little over the firmware: it already boots with servos deactivated, and UI deactivate stops their PWM. The real gain — no drain in deep sleep — does not matter when the rocker is the off switch. Costs a single point of failure in the servo path (contact bounce on impact drops the robot), a driver circuit, 90–180 mA of coil current and firmware. Kubina's SpotMicroESP32 fitted one and forced it permanently on in practice. |
| **LED button** (wake / soft on-off) | "Soft off" via deep sleep is worse than the rocker: Wi-Fi is off either way, waking is a full reboot either way, and deep sleep still drains the pack. A **latching** button also fights the firmware, which wakes on pin *level* — held low, it wakes straight back up. Leika lists the button as optional; its only firmware use is deep-sleep wake. |
| **Hardware servo arm switch** (latching button driving the relay coil) | The one variant with a real use — handling the robot with the ESP32 on and servos physically unpowered. Not needed for now; adding it later means splicing a relay into `SZBK07 IN+`. |

Consequences:

- **Do not rely on UI sleep.** With no wake button, `/api/system/sleep` leaves the robot
  asleep until the rocker power-cycles it. The configured wake pin, `WAKEUP_PIN_NUMBER=38`,
  could not wake an S3 anyway — ext1 wake needs IO0–21 **[inferred]**.
- **The off switch is the rocker**, and only the rocker.

Free GPIO after I2C and the WS2812: IO19, IO20, and IO38–40 if the microSD slot is unused.

## Pending decisions

| Item | What was discussed |
| --- | --- |
| **LiPo alarm** | On the balance lead, ~50 ₴. Warns per cell, independent of firmware — but only warns. With the rocker as the only off switch, firmware can at most warn, sit the robot down and deactivate the servos at 7.0 V; the ESP32 and sensors keep drawing ~15–25 mA until the rocker is switched off. **Switch off after every session**, alarm or not. |

## Open questions

- [ ] **Walking draw.** Sets CC, confirms the 15 A fuse, gives runtime. The ACS712 measures
      it once firmware exists.
- [ ] **Is the board's `3V3` pin a regulator output or an input?** Carried over from
      [the board doc](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md). Matters here because
      the PCA9685 and ADS1115 logic sides are fed from it.
- [ ] **Rail voltage choice.** 6 V is safe and is what this document assumes. The servo is
      rated to 7.2 V (8.4 V by some listings), where torque rises from 32.7 to
      35.2 kg·cm at the cost of higher stall current. Decide before finalising CC, since it
      changes the amperage budget.
