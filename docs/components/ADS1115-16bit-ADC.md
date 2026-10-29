# ADS1115 16-bit I2C ADC module

Four analog inputs on the I2C bus, for pack voltage. **Owned.** Sourced as
["АЦП модуль 16-біт ADS1115 I2C"](https://prom.ua/ua/p2796766325-atsp-modul-bit.html) (prom.ua).

The ESP32-S3-CAM has no usable analog input: ADC1 (GPIO 1–10) is the camera bus, and the free
IO19/IO20 are on ADC2, which is unreliable under Wi-Fi. The ADS1115 moves every reading onto I2C
and costs no GPIO.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Resolution | 16-bit | listing |
| Inputs | 4 single-ended or 2 differential | listing |
| Supply | 2.0–5.5 V — **3.3 V here** | listing |
| Address | `ADDR` → GND `0x48`, VDD `0x49`, SDA `0x4A`, SCL `0x4B` | datasheet |
| Full scale | ±6.144 / 4.096 / 2.048 / 1.024 / 0.512 / 0.256 V | datasheet |
| Sample rate | 8–860 SPS | listing |
| Input limit | **GND − 0.3 V … VDD + 0.3 V**, at any gain | datasheet |
| I2C | 100 and **400 kHz**; 3.4 MHz only after an HS-mode entry the firmware doesn't send | datasheet |
| Supply current | ~150 µA | listing |
| Extras | comparator, `ALRT` pin | listing |

Breakouts like this usually carry I2C pull-ups to `VDD` and an `ADDR` pull-down **[inferred]** —
confirm on the board.

**`VDD` must be 3.3 V** (the ESP32's `3V3`): the pull-ups go to `VDD`, and the ESP32-S3 isn't 5 V
tolerant — the same trap as the PCA9685's `VCC`. So no input may exceed 3.3 V; every signal is
scaled to fit.

## Use in this project

| Pin | To | At 8.4 V |
| --- | --- | --- |
| `VDD` | ESP32 `3V3` | — |
| `GND` | star, as the sensor ground | — |
| `SDA` / `SCL` | IO47 / IO41, shared with the PCA9685 | — |
| `ADDR` | GND → `0x48` | — |
| `A0` | [voltage module](Voltage-sensor-0-25V-divider.md) `S` — pack voltage | 1.68 V |
| `A1` | free — kept for the [ACS712](ACS712-30A-current-sensor.md), deferred | — |
| `A2`, `A3` | free | — |

- **Gain** ±4.096 V on every channel: 125 µV per count — 0.6 mV of pack voltage. The sensor
  limits accuracy, not the ADC.
- **Ground:** one 22 AWG sensor ground from the star; the voltage module takes its ground from
  this module.
- **Addresses:** PCA9685 `0x40`; MPU6050, HMC5883L, BMP180, BNO055 if ever fitted `0x68`, `0x1E`,
  `0x77`, `0x29` **[code]** — no conflicts.

## Firmware

`USE_ADS1115` in [features.ini](../../esp32/features.ini). Driver
[ads1115.h](../../esp32/include/peripherals/drivers/ads1115.h), role class
[battery.h](../../esp32/include/peripherals/battery.h):

- **Continuous mode on `A0`**, ±4.096 V, 128 SPS, so a read is one 2-byte fetch. `Peripherals`
  polls it at 4 Hz from the control task — the PCA9685's task, so the writes never interleave.
- **Pack volts** = input × `BATTERY_DIVIDER_RATIO` (default 5.0, the nominal divider), smoothed.
  Calibrate by overriding it in `build_flags`.
- **WebSocket `battery`** (`BatteryData.voltage`), sent each second to subscribers — only once the
  ADS1115 has initialised, which happens at boot only. `features.battery` reports the flag.
- **UI:** the status bar shows the pack voltage and a battery icon — amber below 7.3 V, red below
  the 7.0 V cutoff; thresholds in
  [BatteryIndicator.svelte](../../app/src/lib/components/statusbar/BatteryIndicator.svelte).

`I2CBus` has no lock: the service task's I2C scan can still interleave with control-task traffic.

## Open items

- [ ] Confirm the pull-ups and the `ADDR` pull-down
- [x] Driver, proto message and UI
- [x] Calibrate `BATTERY_DIVIDER_RATIO` against a meter — 5.043 in the `s3cam` env
- [x] `0x48` on the I2C scan — [build step 11](../pre-calibration-build.md#11-monitoring-ads1115-and-voltage-module)
