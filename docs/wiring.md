# Wiring and power

The robot's power and logic harness: battery to servos, converters, grounding, I2C and
monitoring. This is the **final** wiring, used from the first bench session;
[pre-calibration-build.md](pre-calibration-build.md) is the order of work, with a test per block.

## Status

| State | Parts |
| --- | --- |
| **Core design** | [SZBK07](components/SZBK07-buck-converter.md), [CN3903](components/CN3903-5V-buck-module.md), PCA9685, [ESP32-S3-CAM](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md), servo board, star point |
| **Added, owned** | [2S 1500 mAh LiPo](components/ZOP-Power-2S-1500mAh-LiPo.md), [ADS1115](components/ADS1115-16bit-ADC.md) |
| **Added, decided** | [Fuses](components/Daier-blade-fuse-holders.md), [rocker switch](components/KCD2-201N-B-rocker-switch.md), [voltage module](components/Voltage-sensor-0-25V-divider.md) |
| **Deferred** | [ACS712](components/ACS712-30A-current-sensor.md) current sensor — [Not fitted](#not-fitted) |
| **Decided against** | Relay, LED button, arm switch, per-servo PTCs — [Not fitted](#not-fitted) |
| **Open** | LiPo alarm — [Pending decisions](#pending-decisions) |

## Materials

| Item | Role |
| --- | --- |
| PCA9685 16-channel PWM board | Servo signals only; the driver targets `0x40` **[code]** |
| [SZBK07](components/SZBK07-buck-converter.md) 300 W CC/CV buck | Battery → 6.0 V servo rail, with a current limit |
| [CN3903](components/CN3903-5V-buck-module.md) fixed 5 V buck | Battery → 5 V for the ESP32 |
| XT60 × 2 | Robot-side half; the battery's half for the bench-supply lead |
| [F114-C + 15 A, F117-C + 2 A](components/Daier-blade-fuse-holders.md) | Main and logic-branch fuses |
| [KCD2-201N-B rocker](components/KCD2-201N-B-rocker-switch.md) | Main switch |
| Perfboard + 12 × 3-pin headers | [Servo board](#servo-distribution) |
| 14 AWG solid bare copper | The servo board's two rails |
| 14 AWG stranded | Trunk runs — [Wire gauge](#wire-gauge) |
| WAGO 221-415 × 3, 221-413 × 1 | [Ground star](#ground-star); the + split |
| 1000–2200 µF, ≥ 16 V, low-ESR, 105 °C | Across the servo rails at the centre feed |
| [ADS1115](components/ADS1115-16bit-ADC.md) | Four analog inputs over I2C |
| [Voltage module](components/Voltage-sensor-0-25V-divider.md) 0–25 V | Pack voltage |
| 22 AWG; jumper wire | Light returns and the logic branch; `3V3`, `SDA`, `SCL`, signals |

## Power path

```
 2S LiPo ─► XT60 ─► F114-C ─► rocker ─► + split ─┬──────────► SZBK07 +IN
           12 AWG    15 A     1 pole     WAGO    │ 14 AWG     CV 6.0 V, CC set
                                                 │            OUT+ ─► servo board centre feed
                                                 │
                                                 └─► F117-C ─┬──► CN3903 IN+
                                                     2 A     │    5.0 V fixed
                                                             │    OUT+ ──► ESP32 VCC
                                                             └──► voltage module VCC

 pack − ────────────────────────────────────────► ★ star point, 14 AWG
 balance lead ── not connected; charger only
```

The bench supply plugs into the same XT60 on its own lead, so the connector and fuse are in
circuit from the first session.

## Servo distribution

Servo power and ground run on a purpose-built board and **never touch the PCA9685**, which
carries signal only.

```
               SERVO BOARD — 12 × 3-pin headers
   ┌──────────────────────────────────────────────┐
   │ sig  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪  ▪      │──► PCA9685 CH0..11 (12 thin wires)
   │ 6 V  ▪──▪──▪──▪──▪──▪──▪──▪──▪──▪──▪──▪      │
   │ GND  ▪──▪──▪──▪──▪──▪──▪──▪──▪──▪──▪──▪      │
   └─────────────────────┬──┬─────────────────────┘
                     6 V │  │ GND     both rails fed at the CENTRE, not the end;
   SZBK07 OUT+ (6.0 V) ──┘  └──► star  capacitor across the rails, right here — all 14 AWG
```

1. **`V+` stays unconnected.** The PCA9685 IC needs only `VCC`, `GND`, `SDA`, `SCL`. Its stock
   `V+` trace is good for ~2–3 A **[inferred]** — why the [project BOM](1_components.md) says "add
   thicker solder traces" — so servo current stays off the board entirely.
2. **Servo ground returns on the board's own rail**, which reaches the star as one 14 AWG
   conductor.
3. **One 22 AWG wire from the star to `PCA9685 GND`.** PWM is referenced to ground; without it
   the servos jitter or ignore commands.
4. **Both converters land `IN−` and `OUT−` on the star.** They're the same copper inside a
   non-isolated buck; wiring both keeps return current off the converter's ground pour.

The board's rails are bare 14 AWG soldered to every pin — thin perfboard copper would recreate
the PCA9685's bottleneck — fed at the centre to halve the rail any current crosses. Header pins
(~3 A) are marginal only at stall. Servo plugs have no latch: add a bar or zip tie across the
row, and strain-relieve the leads. A reversed plug swaps signal and ground (V+ is the middle
pin), so the servo won't run; mark the GND row or use keyed headers.

**What CC does not cover** at the assembled 12 A:

- **A single stalled servo.** One stall (~3 A) never reaches the limit; CC engages only when four
  or five stall at once. Careful calibration and the PWM clamp must prevent sustained stalls.
- **A short in one branch.** CC holds the current rather than folding back, so a shorted servo or
  pinched lead gets 12 A until the rocker is switched off.
- **Loose hardware.** The bare rails are covered once tested, so a stray screw can't bridge them.

## Wire gauge

The trunk is sized to the **CC setting**, not the load: CC is a hard ceiling on the servo rail,
so the ~36 A of twelve stalled servos never needs wiring for. At these lengths heat, not voltage
drop, is the constraint.

| Conductor | Carries | Gauge |
| --- | --- | --- |
| XT60 → F114-C | full pack draw, **unfused** — keep it short | 12 AWG (holder lead) |
| F114-C → rocker → split | full pack draw | 14 AWG |
| Pack `−` → star | up to ~12 A | 14 AWG |
| Split → `SZBK07 +IN` | servo-rail input | 14 AWG |
| `SZBK07 OUT+` → board 6 V rail | whole servo rail, capped by CC | 14 AWG |
| Board rails | the same | 14 AWG solid, bare |
| Board GND rail → star | the same, returning | 14 AWG |
| `SZBK07 −IN` / `OUT−` → star | 0–3 A net — see [Ground star](#ground-star) | 14 AWG |
| Header → servo, × 12 | one servo, ≤ 3.4 A | stock lead (~22 AWG) |
| Split → F117-C | logic branch | 16 AWG (holder lead) |
| F117-C → CN3903 and the voltage-module input; CN3903 output | ESP32 and sensors, < 0.6 A | 22 AWG |
| Star → `PCA9685 GND`, ESP32 `GND`, sensor ground | logic returns and reference | 22 AWG |
| Sensor signals, `3V3`, `SDA`, `SCL` | signal | jumper wire |

Every thin wire sits behind a fuse sized for it — why the voltage module taps the logic branch
after the 2 A fuse. The exception is the twelve servo branches, behind CC only
([above](#servo-distribution)).

## Ground star

Three WAGO 221-415 on a carrier, chained with 3 cm of 14 AWG: one block for the heavy returns,
two for the light ones.

```
 HEAVY                  LIGHT 1                  LIGHT 2
 ├─ servo board GND     ├─ link from HEAVY       ├─ link from LIGHT 1
 ├─ pack −              ├─ link to LIGHT 2       ├─ ESP32 GND
 ├─ SZBK07 OUT−         ├─ CN3903 OUT−           ├─ PCA9685 GND
 ├─ SZBK07 IN−          ├─ CN3903 IN−            ├─ sensor ground
 └─ link to LIGHT 1     └─ (spare)               └─ (spare)
```

- **Where the current flows:** 12 A arrives from the servo board and 9.5–12 A of it leaves on
  pack `−`, straight through HEAVY. The rest — the SZBK07's output current minus its input — flows
  in its `IN−`/`OUT−` pair: ~2.5 A on a full pack, ~0 once it drops out. The pack leads are the
  critical conductors. The converter's stay 14 AWG in case a pack lead degrades.
- **Why WAGO 221:** a spring clamp holds under vibration where a screw loosens. Rated 32 A IEC /
  20 A UL over 0.2–4 mm² (24–12 AWG), fine-stranded wire directly, no ferrules. Not the 2273/773
  series, which is solid-wire only. Strain-relieve the bundle.
- **Chaining is fine:** the links carry only the logic returns, under 1 A — about a millivolt.
  Star grounding guards against servo current in a PCA9685 trace, not against millivolts.
- **The balance lead stays off the star:** its pin 1 is pack `−`, already here via the XT60.

## Fuses

The [battery](components/ZOP-Power-2S-1500mAh-LiPo.md) has no BMS: the fuses are all that stands
between a short and 60 A+. Parts and ratings: [Daier-blade-fuse-holders.md](components/Daier-blade-fuse-holders.md).

| Fuse | Where | Protects |
| --- | --- | --- |
| **15 A**, F114-C | directly after the XT60 | the 14 AWG trunk |
| **2 A**, F117-C | where the logic branch leaves the split | the 22 AWG branch and the voltage-module tap |

They protect wiring, not loads. Pack current can't exceed ~12 A by design, so the 15 A fuse never
opens for a fault past the SZBK07 — that side is CC's. The exception is the SZBK07 failing short
(a failed high-side transistor): CC is gone, the servos see the pack's 8.4 V, and the 15 A fuse is
the backstop.

## Main switch

The [KCD2-201N-B rocker](components/KCD2-201N-B-rocker-switch.md), in pack `+` between the F114-C
and the + split, is the **only** on/off control; off means zero draw.

- One pole, 14 AWG soldered to the tabs; switch `+` only.
- AC-only rating: at 2S the pack is below the ~12 V needed to sustain an arc, so breaking ~12 A
  is acceptable **[inferred]**. Revisit at 3S.
- **Deactivate the servos before switching off** — a standing robot drops.

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

- **`IO47`/`IO41`** are `SDA_PIN`/`SCL_PIN` in the `s3cam` env **[code]**, clear of the camera bus
  and memory pins. `IO14` stays reserved for the WS2812.
- **`VCC` and `VDD` must be 3.3 V.** Both breakouts pull SDA/SCL up to that pin, and the ESP32-S3
  is not 5 V tolerant.
- **`IO47` runs at the module's memory voltage:** 3.3 V on this N16R8, 1.8 V on 1.8 V-memory
  variants **[datasheet]**. [Build step 1](pre-calibration-build.md#1-esp32-alone-flash-and-check)
  checks it.
- **Bus at 400 kHz**, set by the `s3cam` env's `I2C_FREQUENCY`. The global default of 1 MHz
  ([global.h](../esp32/include/global.h)) is beyond the ADS1115, and the breakouts' combined
  pull-ups (3–5 kΩ) suit 400 kHz. Saved settings override it — the I2C page shows the live value.

## Monitoring

Pack voltage through the [ADS1115](components/ADS1115-16bit-ADC.md). The ESP32 has no usable
analog input: ADC1 is the camera bus, ADC2 is unreliable under Wi-Fi. No current sensing for now —
the [ACS712](components/ACS712-30A-current-sensor.md) is deferred ([Not fitted](#not-fitted)).

```
   voltage module
   ──────────────
   VCC ◄── after F117-C
   S ─────────────► A0
   − ─────────────► ADS1115 GND ◄── star (sensor ground)
```

| ADS1115 | Signal | At 8.4 V |
| --- | --- | --- |
| `A0` | pack ÷ 5 — [voltage module](components/Voltage-sensor-0-25V-divider.md) | 1.68 V |
| `A1`–`A3` | free | — |

Every input stays under 3.3 V — never the 5 V rail undivided. Pack voltage only: the balance
charger equalises the cells, and its display is the per-cell check. The voltage module takes ground
from the ADS1115's `GND`, sharing its reference.

Under load `A0` reads ~0.1–0.2 V below the pack terminals — the trunk's drop at 12 A
**[inferred]** — so a 7.0 V cutoff trips early, on the safe side. The firmware reads `A0` and streams
pack voltage, but acts on nothing yet — [the ADS1115 doc](components/ADS1115-16bit-ADC.md#firmware).

## Power architecture

### Battery and headroom

This build runs the [2S 1500 mAh LiPo](components/ZOP-Power-2S-1500mAh-LiPo.md) — the
[project BOM](1_components.md) specifies 7.6–8.4 V, and [spot.md](spot.md) a maximum of 8.4 V. The
SZBK07 needs ~1.5–2 V over its output **[inferred]**; the CN3903 ~0.5 V (5.5 V in, listing):

| State | Pack | SZBK07 over 6 V | CN3903 over 5 V |
| --- | --- | --- | --- |
| Full | 8.4 V | 2.4 V | 3.4 V |
| Nominal | 7.4 V | 1.4 V — dropped out | 2.4 V |
| **Working cutoff**, under load | **7.0 V** | 1.0 V — dropped out | 2.0 V |
| Resting floor | 6.6 V | 0.6 V — dropped out | 1.6 V |
| Damage | 6.0 V | none | 1.0 V |

- **If that headroom holds, the servo rail is regulated only near full charge**, down to
  ~7.5–8 V. Below, it follows the pack, and the servos (4.8–7.4 V) get weaker, not damaged.
  [Test 12](pre-calibration-build.md#12-servo-rail-to-the-board) measures the real dropout.
- **The SZBK07 is still needed for the top of the range:** a full pack's 8.4 V exceeds the servos'
  7.4 V.
- **The CN3903 regulates down to 5.5 V in**, so the ESP32 outlives the servos, as it should.
- **3S** would give headroom throughout and cut pack current by a third, but diverges from the
  8.4 V in `spot.md` — check that isn't a hard limit first.

### Current limit (CC)

| Phase | CC |
| --- | --- |
| Calibration | **1.5 A** — set in [build step 7](pre-calibration-build.md#7-szbk07-servo-rail-converter) |
| Assembled robot | **~12 A** — what the trunk and the 15 A fuse are sized for |

12 A is a ceiling, not a prediction: under the SZBK07's dependable 15 A, and 8C on the pack.
Measure walking draw with a DC clamp meter on the trunk and lower it if you can. Setting ~12 A by
shorting the output through an ammeter exceeds most meters' 10 A range — use a 20 A range within
its time limit, or the clamp meter.

**Thermal:** the SZBK07 runs cold at calibration currents; its 300 W figure assumes forced air.
Feel the heatsinks after sustained multi-servo load.

## Operating rules

- **After every session: deactivate, rocker off, unplug the XT60.** The rocker is a switch, not an
  isolator — pressed on in a bag, it drains the pack.
- **Flash with the rocker on and only the adapter's `GND`/`TX`/`RX`** once the ESP32 is on the
  harness. The adapter's 5 V back-feeds through the CN3903 **[inferred]**.
- **If the robot collapses but the UI stays up,** suspect the servo rail: rocker off first, then
  debug.

## Not fitted

Decided against, so it isn't re-litigated — or deferred:

| Item | Why not |
| --- | --- |
| **[ACS712](components/ACS712-30A-current-sensor.md) current sensor** — deferred | Monitoring only, not protection: nothing acts on its reading, and the fuses and CC cover faults without it. Walking draw is a one-off clamp-meter measurement. Fitting it later means cutting the trunk between the rocker and the + split; `A1` stays free for it. |
| **Relay** (ESP32-controlled servo power) | The firmware already boots deactivated, and deactivate stops PWM. Its real gain, no drain in deep sleep, is moot with the rocker as the off switch. Costs a single point of failure in the servo path (contact bounce on impact), a driver, 90–180 mA of coil current and firmware. Kubina's build fitted one and forced it permanently on. |
| **LED button** (wake / soft on-off) | Deep-sleep "soft off" still drains the pack and wakes through a full reboot anyway. A latching button also fights the level-triggered wake. Leika lists it as optional. |
| **Hardware servo arm switch** | Handling the robot powered with servos dead — not needed now. Adding it later means splicing a relay into `SZBK07 +IN`. |
| **Per-servo PTC fuses** | Twelve more parts in the servo path; the risks they'd cover are accepted and noted under [Servo distribution](#servo-distribution). |

With no wake button, sleep is off: the configured `WAKEUP_PIN_NUMBER=38` can't wake an S3, as ext1
wake takes only IO0–21 **[datasheet]**, so `/api/system/sleep` refuses with 400
([system_service.cpp](../esp32/src/system_service.cpp)) rather than sleeping until a power cycle.

Free GPIO after I2C and the WS2812: IO19, IO20, and IO38–40 if the microSD slot is unused.

## Pending decisions

| Item | Notes |
| --- | --- |
| **LiPo alarm** | On the balance lead, ~50 ₴; warns per cell, independent of firmware. Firmware can at most warn, sit the robot down and deactivate at 7.0 V — it can't cut power. Deactivated, the ESP32 and the live servo rail still draw **~0.15–0.35 A [inferred]**: from the 7.0 V cutoff to 3.0 V/cell damage in roughly **15–90 minutes**. An alarm left plugged in also drains the pack — unplug it with the XT60. |

## Open questions

- [ ] **Walking draw** — sets CC, confirms the 15 A fuse, gives runtime. A DC clamp meter on the
      trunk measures it.
- [ ] **Idle draw** — logic plus twelve deactivated servos, replacing the estimate above.
      [Build steps 9 and 13](pre-calibration-build.md#9-esp32-onto-the-harness) take the first readings.
- [ ] **Is the ESP32 board's `3V3` pin a regulator output?** Build step 9 answers it.
- [ ] **Do the servos take 3.3 V pulses?** The PCA9685 runs at 3.3 V. Test 13 shows it; a failure
      looks like a missing PCA9685 ground.
- [ ] **Rail voltage.** 6 V is safe and assumed here. The servo is rated to 7.2–7.4 V (8.4 V in
      some listings); stall torque is 32.7 kg·cm at 4.8 V and 35.2 at 8.4 V, so 6 → 7.4 V gains
      under 5 %, at higher stall current. Decide before finalising CC.
