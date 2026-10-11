# ADS1115 16-bit I2C ADC module

Four analog inputs on the I2C bus, for pack voltage and current. **Owned.** Sourced as
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

| Pin | To | At 8.4 V / 12 A |
| --- | --- | --- |
| `VDD` | ESP32 `3V3` | — |
| `GND` | star, as the sensor ground | — |
| `SDA` / `SCL` | IO47 / IO41, shared with the PCA9685 | — |
| `ADDR` | GND → `0x48` | — |
| `A0` | [voltage module](Voltage-sensor-0-25V-divider.md) `S` — pack voltage | 1.68 V |
| `A1` | [ACS712](ACS712-30A-current-sensor.md) `OUT` via 10 kΩ — pack current | 1.71 V |
| `A2` | free | — |
| `A3` | free — optionally the 5 V rail **÷ 2** (2 × 10 kΩ): the ACS712's zero | 2.50 V |

- **Gain** ±4.096 V on every channel: 125 µV per count — 0.6 mV of pack voltage, 1.9 mA of current.
  The sensors limit accuracy, not the ADC.
- **Ground:** one 22 AWG sensor ground from the star; the two sensor modules take their ground
  from this module.
- **Addresses:** PCA9685 `0x40`; MPU6050, HMC5883L, BMP180, BNO055 if ever fitted `0x68`, `0x1E`,
  `0x77`, `0x29` **[code]** — no conflicts.

## Firmware

**Nothing reads it yet.** A driver belongs in `esp32/include/peripherals/drivers/` on the shared
`I2CBus`, wrapped by a role-level sensor the way `imu.h` wraps the MPU6050. The bus is also the
PCA9685's, written every 10 ms: single-shot conversions at 128 SPS take ~8 ms per channel, so poll
at a few hertz, not per control tick **[inferred]**. The `s3cam` env runs the bus at 400 kHz, the
ADS1115's maximum ([build step 1](../pre-calibration-build.md#1-esp32-alone-flash-and-check)).

`I2CBus` caches one device handle, swapped whenever the address changes, and has no lock: a driver
polling from another task must not interleave with the control task's PCA9685 writes.

## Open items

- [ ] Confirm the pull-ups and the `ADDR` pull-down
- [ ] Driver, proto message and UI
- [ ] `0x48` on the I2C scan — [build step 11](../pre-calibration-build.md#11-monitoring-ads1115-voltage-module-acs712)
