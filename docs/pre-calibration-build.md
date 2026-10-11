# Pre-calibration build

Wires the robot's electronics into their final form, one block at a time, testing each block
before the next. Servo calibration then runs on this harness unchanged — from the bench
supply or the battery — and nothing is rewired at assembly.

What each part is and why it sits where it does: [wiring.md](wiring.md). This page is the
order of work.

## Before you start

**Parts:** everything in [wiring.md → Materials](wiring.md#materials).

**Tools:** multimeter with DC volts, continuity and a 10 A range · bench supply with a current
limit · soldering iron that can heat 14 AWG (60 W+) · ferrule crimper · CH343P adapter ·
a vice or clamp.

**Consumables:** solder, heat-shrink, ferrules, Kapton tape or hot glue, nail varnish.

**Rules for every step**

- Wire with everything off: rocker off, bench supply off. Insulate loose ends before powering.
- Bench supply at **7.4 V**; its current limit is given per step.
- Don't start a step until the previous test passes.

**Firmware** is flashed once, in step 1. No step needs new code; steps 1, 9 and 11 change how
you work with it, marked **Firmware**.

## 1. ESP32 alone: flash and check

Powered from the CH343P's `5V` — this step only.

1. Wire the adapter per [the board doc](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md#programming),
   its VIO jumper on 3V3.
2. **Firmware:** if the board already serves the web UI at `http://192.168.4.1`, skip the flash.
   Otherwise confirm `EMBED_WEBAPP=1` in [build_settings.ini](../esp32/build_settings.ini) —
   calibration needs the web UI, and the build needs `pnpm` and `protoc` on PATH. Enter the
   bootloader (hold `IO0` low, pulse `RST`, release `IO0`) and flash from **PowerShell**
   ([why](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md#building-on-windows)):
   ```powershell
   cd D:\s
   pio run -e s3cam -t upload --upload-port COM3
   ```
3. Join the `Spot-Micro` AP (password `spot-leika`) and open `http://192.168.4.1`.
4. **Firmware:** the `s3cam` env runs the I2C bus at 400 kHz (`I2C_FREQUENCY` in
   [platformio.ini](../platformio.ini)); the global 1 MHz default is beyond the ADS1115. A board
   flashed before keeps its saved setting: if Peripherals → I2C → Configuration shows `1000000`,
   set `400000` there once and save.

> **✅ Test 1**
> - Serial log at 115200: one boot, no reset loop, `Camera probe successful`.
> - The UI loads. I2C scan finds nothing — expected, the bus is empty; `unexpected nack` messages
>   in the log are normal until step 10.
> - I2C Configuration shows `400000`.
> - Meter `IO47` and `IO41` to `GND`: close to 3.3 V (the firmware's pull-ups; a little less
>   while it retries the empty bus). **About 1.8 V on `IO47` means a 1.8 V-memory module —
>   stop**: the breakouts' 3.3 V pull-ups would damage it.

## 2. Lay out the parts

Fix every part where it will live — in the body, or on a plate that moves into it unchanged —
and cut wires to final length as you go.

- **XT60** where the battery plugs in, the **F114-C** right behind it: that stretch is unfused.
- **Rocker** reachable without lifting the robot.
- **ESP32** with its left header (`RX`, `TX`, `IO0`, `RST`) reachable — serial is the only way
  to reflash.
- **SZBK07** with air around its heatsinks, away from the LiPo.
- **Servo board** where all twelve stock servo leads reach with slack at full joint travel;
  **PCA9685** beside it.
- **Star point** central, so the heavy returns stay short.

## 3. Servo board

1. Twelve 3-pin headers in a row on perfboard; rows **signal / 6 V / GND**. Mark the GND row
   (brown wire) unmistakably — plain headers don't enforce polarity.
2. Bare solid **14 AWG** along the 6 V row and the GND row, soldered to every pin. Keep solder
   **out of the gap between the rows** — it's under 1 mm.
3. Mark the **centre** of each rail: the trunk lands there in step 12, not at an end.
4. Capacitor across the rails at the centre: **1000–2200 µF, ≥ 16 V, low-ESR, 105 °C**, with the
   **stripe (−) on the GND rail**. Fitted backwards, it vents when the rail comes up.

> **✅ Test 3** — unpowered, continuity mode:
> - Centre feed → every header's middle pin: continuity. GND rail → every GND pin: the same.
> - 6 V ↔ GND: the reading climbs as the meter charges the capacitor. A steady ~0 Ω is a
>   solder bridge.
> - Every signal pin: open to 6 V, to GND and to its neighbours.

Then **cover the bare rails** with Kapton tape or hot glue, so a loose screw can't bridge them.

## 4. Signal wires: PCA9685 → servo board

1. Check the PCA9685 has resistor arrays beside its channel headers (typically 220 Ω) — they
   limit the current if a servo is plugged in backwards.
2. Twelve thin wires from each channel's **PWM** pin to the matching signal pin:
   CH0 → header 0 … CH11 → header 11.
3. The PCA9685's `V+` terminal stays empty.

> **✅ Test 4** — unpowered: each `CHn` PWM pin → header *n* signal pin: continuity; to the
> neighbouring headers, 6 V and GND: open.

## 5. Ground star

Three WAGO 221-415 on a carrier, chained with 3 cm of 14 AWG and labelled **HEAVY**,
**LIGHT 1**, **LIGHT 2**. Later steps land each conductor per [wiring.md](wiring.md#ground-star).
No ferrules.

> **✅ Test 5** — continuity across all three blocks; tug every link.

## 6. Power input: XT60 → fuse → rocker → ACS712 → split

1. Solder the **F114-C**'s 12 AWG lead into the robot-side XT60 `+`, cut as short as it will go.
   Fuse **out**; discard any 30/40 A fuses it came with.
2. XT60 `−` → 14 AWG → **HEAVY**.
3. F114-C → 14 AWG → **rocker** (one pole, soldered, heat-shrunk) → 14 AWG → **ACS712 `IP−`**.
   Current enters at `IP−`: reversed on purpose. Ferrules on both ACS712 wires.
4. ACS712 `IP+` → 14 AWG → the **+ split**, a WAGO 221-413. From it: 14 AWG for the SZBK07, and
   the F117-C's input lead. Insulate their far ends.
5. The **bench-supply lead**: 14 AWG with an XT60 of the **battery's** gender (the shrouded
   half), + and − marked.

> **✅ Test 6**
>
> Unpowered:
> - XT60 `+` → F114-C input: continuity. Fit the **15 A** fuse.
> - Rocker on: XT60 `+` → split, continuity. Rocker off: open.
> - XT60 `−` → star: continuity. XT60 `+` ↔ `−`: open, rocker either way.
>
> Bench lead, supply at 7.4 V: + on the XT60 housing's + contact.
>
> Powered — 7.4 V, **0.5 A** limit:
> - Rocker off: 0 V from split to star. Rocker on: 7.4 V, ~0 A drawn.

## 7. SZBK07: servo-rail converter

1. Split → **`+IN`**; `−IN` → 14 AWG → **HEAVY**; `OUT−` → 14 AWG → **HEAVY**.
2. `OUT+`: its 14 AWG to the servo board, **board end insulated** — connected in step 12.
3. Turn **CC** and **CV** fully counter-clockwise (multi-turn). The module ships at an
   arbitrary setting.

Set current first, then voltage — 7.4 V, 0.5 A limit:

4. Meter on its **10 A range** across `OUT+` and `OUT−`. Rocker on, module switch on. Turn
   **CC** up to **1.5 A**, the calibration limit. Rocker off; remove the meter.
5. Meter on DC volts across the output. Rocker on. Turn **CV** up to **6.0 V**.
6. Nail varnish on the CV pot. CC stays free: it goes up for the assembled robot.

> **✅ Test 7**
> - Unloaded: **6.0 V**, positive on `OUT+`.
> - Flip the module's `ON/OFF` switch while watching the output, and record whether it cuts it
>   ([open item](components/SZBK07-buck-converter.md#open-items)). Leave it on and tape it.

## 8. Logic branch: F117-C and CN3903

1. **F117-C** on its lead from the split. Mark it **"2 A"** and fit a **2 A** fuse — it takes the
   same size as the 15 A, so a spare from the wrong bag would unprotect this branch.
2. F117-C → 22 AWG → **CN3903 `IN+`**; leave a 22 AWG tail on this node for the voltage module
   (step 11) — it carries pack voltage, so not jumper wire.
3. `IN−` and `OUT−` → 22 AWG each → **LIGHT 1**.
4. `OUT+` → 22 AWG toward the ESP32, end insulated. Strain-relieve the solder pads.

> **✅ Test 8** — 7.4 V, 0.5 A limit, rocker on:
> - `OUT+` → star: **5.0 V** (4.9–5.1). A fixed module can't be adjusted — if it's off,
>   replace it.
> - Sweep the supply 6.6 → 8.4 V: the output stays at 5.0 V.
> - Pull the 2 A fuse: 0 V. Refit.

## 9. ESP32 onto the harness

1. CN3903 `OUT+` → ESP32 **`VCC`**; ESP32 **`GND`** → 22 AWG → **LIGHT 2**. Use a keyed two-pin
   housing or clear polarity marks — the pins sit side by side.
2. Remove the CH343P's **`5V`** wire for good; keep `GND`, `TX`, `RX`.

**Firmware:** from now on, flash with the **rocker on** and only `GND`/`TX`/`RX` connected.
Rocker off, the adapter's 5 V back-feeds through the CN3903 into the harness; rocker on, it
fights the CN3903.

> **✅ Test 9** — supply limit **1 A** from here:
> - Before plugging in: rocker on, meter the connector — 5.0 V, positive on `VCC`. Rocker
>   off, plug in.
> - Rocker on: the serial log boots and the UI loads.
> - Board `3V3` pin: 3.3 V — the regulator's output
>   ([open question](wiring.md#open-questions)).
> - Note the supply current: the **logic baseline** for the idle-drain figure in
>   [wiring.md](wiring.md#pending-decisions).

## 10. PCA9685 on the I2C bus

1. ESP32 `3V3` → PCA9685 **`VCC`**; **LIGHT 2** → 22 AWG → PCA9685 **`GND`**. Not `SDA`/`SCL` yet.

> **✅ Test 10a** — rocker on: PCA9685 `VCC`, `SDA` and `SCL` all read 3.3 V (its own
> pull-ups). **5 V anywhere — stop**: the ESP32 isn't 5 V tolerant.

2. Rocker off. `IO47` → **`SDA`**, `IO41` → **`SCL`**.

> **✅ Test 10b** — rocker on:
> - I2C scan lists **`0x40`**.
> - **Signal chain, no servos.** Servo page → **Active** on: every signal pin on the servo board
>   reads 0.1–0.5 V DC to GND — each channel's pulse, averaged by the meter. Select servo *n*
>   and move the PWM slider: only header *n* follows, about 0.8 mV per count (306 → 0.25 V).
>   Walk all twelve; this proves every wire and the channel order.
> - **Active** off: every signal pin ~0 V.

## 11. Monitoring: ADS1115, voltage module, ACS712

1. **ADS1115:** `VDD` ← ESP32 `3V3`; `GND` ← 22 AWG from **LIGHT 2**, the *sensor ground*;
   `ADDR` → `GND` (address 0x48). Not `SDA`/`SCL` yet.
2. **ACS712:** `VCC` ← CN3903 `OUT+` (splice); `GND` → ADS1115 `GND`; `OUT` → **10 kΩ** → free
   end.
3. **Voltage module:** input `VCC` ← the tail after the 2 A fuse, its input `GND` unused;
   `−` → ADS1115 `GND`; `S` → free end.

**Firmware:** nothing reads the ADS1115 yet, so these tests use the scan and the meter. The
bus must be at 400 kHz (step 1).

> **✅ Test 11a** — rocker on, nothing at the ADS1115's inputs yet:
> - ADS1115 `VDD`, `SDA`, `SCL`: 3.3 V.
> - Voltage module `S` → GND: supply ÷ 5 (≈ 1.48 V at 7.4 V).
> - ACS712, at the resistor's free end → GND: ≈ 2.5 V.
> - Nothing above 3.3 V.

4. Rocker off. ADS1115 onto the bus; `S` → **`A0`**; the 10 kΩ → **`A1`**.

> **✅ Test 11b** — I2C scan: **`0x40`** and **`0x48`**. `A0` and `A1` read as in 11a.

## 12. Servo rail to the board

SZBK07 `OUT+` → **6 V centre feed**; board **GND centre** → 14 AWG → **HEAVY**.

> **✅ Test 12** — rocker on, no servos:
> - Every header: **6.0 V** from the middle pin to the GND pin, positive on the middle.
> - Signal pins ~0 V (Active off).
> - *Optional, checks the ACS712:* supply limit to 2 A; hold a 5 Ω ≥ 10 W resistor (or a
>   12 V 21 W car bulb) across one header's 6 V and GND pins for a few seconds — it gets hot.
>   The supply current rises by ~1 A, and `A1` must **fall** by ~66 mV per amp of that rise.
>   If it rises, the ACS712 is mounted forward: swap its two current wires.
> - *Optional, measures the SZBK07's dropout:* the same load, supply swept 8.4 → 7.0 V. Note the
>   input where the rail leaves 6.0 V — a light-load figure; it rises with current
>   ([why it matters](wiring.md#battery-and-headroom)).

## 13. First servo

1. Rocker off. One servo in the vice, **horn off**, into **header 0** — brown on the GND row.
2. Supply limit **2–3 A**.

> **✅ Test 13** — rocker on:
> - Servo page → servo 0 → **Active** on: the servo moves to channel 0's pose angle. If it
>   jitters or ignores the pulses with the PCA9685 ground in place, suspect its 3.3 V pulses
>   ([open question](wiring.md#open-questions)).
> - Nudge the PWM slider either side of centre: the servo follows. Keep off the slider's ends
>   ([why](servo-calibration.md#cautions)).
> - **Active** off: note whether the servo goes limp or keeps holding — it decides whether
>   deactivating ends a stall.
> - Supply current with the servo plugged in and deactivated: record it with the step 9
>   baseline.

The harness is finished: continue with [servo calibration](servo-calibration.md).

## Running from the battery

Every step works the same from the [LiPo](components/ZOP-Power-2S-1500mAh-LiPo.md), but nothing
in the firmware watches the pack yet — you do.

1. Charge it on the balance charger, out of the robot; meter it: 7.4–8.4 V.
2. Rocker off, bench lead out, pack in. A LiPo alarm on the balance lead, if you have one.
3. Rocker on: `A0` × 5 matches the pack.
4. Every ~30 minutes, meter the pack (voltage-module input, or `A0` × 5). **Stop at 7.2 V** —
   margin over the 7.0 V cutoff, since only you are watching.
5. After the session: deactivate, rocker off, **unplug the XT60** and the alarm.
