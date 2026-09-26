# ADS1115 16-bit I2C ADC module

Four analog inputs on the existing I2C bus, for battery and current monitoring. **Owned.**
Sourced as ["АЦП модуль 16-біт ADS1115 I2C"](https://prom.ua/ua/p2796766325-atsp-modul-bit.html)
(prom.ua).

**Why an external ADC at all:** the ESP32-S3-CAM has no usable analog input. ADC1
(GPIO 1–10) is consumed by the camera bus, and the free pins IO19/IO20 are on ADC2, which
reads unreliably while Wi-Fi is active. The ADS1115 moves every analog reading onto I2C
and costs no GPIO.

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[listing]** = stated by the seller, **[datasheet]** = TI ADS1115 datasheet,
**[code]** = read from this repo, **[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Resolution | 16-bit | **[listing]** |
| Inputs | 4 single-ended or 2 differential | **[listing]** |
| Supply | 2.0 – 5.5 V — **run it at 3.3 V here**, see below | **[listing]** |
| I2C address | 0x48 – 0x4B, set by the `ADDR` pin | **[listing]** |
| Address map | `ADDR` → GND `0x48`, VDD `0x49`, SDA `0x4A`, SCL `0x4B` | **[datasheet]** |
| Gain (PGA) | full scale ±6.144 / 4.096 / 2.048 / 1.024 / 0.512 / 0.256 V | **[datasheet]** |
| Sample rate | 8 – 860 samples/s | **[listing]** |
| Input limit | **GND − 0.3 V … VDD + 0.3 V**, regardless of the gain setting | **[datasheet]** |
| Supply current | ~150 µA | **[listing]** |
| Extras | Programmable comparator, `ALRT` pin | **[listing]** |

The module carries I2C pull-ups to `VDD` and a pull-down on `ADDR` **[inferred]** — both
common on these breakouts; confirm on the board.

## The one hard rule: VDD = 3.3 V

**Power it from the ESP32's `3V3`, never 5 V.** The breakout's I2C pull-ups go to `VDD`,
so at 5 V it idles SDA/SCL at 5 V — and the ESP32-S3 is not 5 V tolerant. It is the same
trap as the PCA9685's `VCC`.

The consequence is that **no analog input may exceed 3.3 V**. Every signal fed to it is
scaled to stay under that — see the channel table.

## Use in this project

| Pin | To | Measures | Signal at 8.4 V / 12 A |
| --- | --- | --- | --- |
| `VDD` | ESP32 `3V3` | — | — |
| `GND` | star point, via the sensor ground | — | — |
| `SDA` / `SCL` | IO47 / IO41, shared with the PCA9685 | — | — |
| `ADDR` | GND | address **0x48** | — |
| `A0` | [voltage module](Voltage-sensor-0-25V-divider.md) `S` | whole pack | 1.68 V |
| `A1` | [ACS712](ACS712-30A-current-sensor.md) `OUT`, via 10 kΩ | pack current | 1.71 V |
| `A2` | free | — reserved for per-cell monitoring, [not fitted](Voltage-sensor-0-25V-divider.md#per-cell-monitoring--not-fitted) | — |
| `A3` | free | — optionally the 5 V rail, for ACS712 correction | — |

**Gain:** ±4.096 V full scale for every channel. One count is 125 µV — 0.6 mV of pack
voltage behind the ÷5 divider, 1.9 mA of current at the ACS712's 66 mV/A. The ADC is not
the limit on either; the sensors are.

**The ground is the sensor ground**, not a separate star conductor per module: one 22 AWG
from the star to this module's `GND`, and the two sensor modules take their ground from
here. See [wiring.md](../wiring.md#monitoring).

### No address conflicts

| Device | Address |
| --- | --- |
| PCA9685 | `0x40` **[code]** |
| **ADS1115** | **`0x48`** |
| MPU6050 / HMC5883L / BMP180 / BNO055, if fitted | `0x68` / `0x1E` / `0x77` / `0x29` **[code]** |

## Firmware

**Nothing reads it yet.** A driver belongs in `esp32/include/peripherals/drivers/` beside
the others, on the shared `I2CBus`, wrapped by a role-level sensor the way `imu.h` wraps
the MPU6050.

The bus is also the PCA9685's, written every 10 ms by the control task. Single-shot
conversions at 128 SPS take ~8 ms per channel, so poll all four at a few hertz, not per
control tick **[inferred]**.

## Open items

- [ ] **Confirm the module's pull-ups and `ADDR` pull-down** on the board.
- [ ] **Driver, proto message and UI** — none exist.
- [ ] **Scan for `0x48`** on the I2C scan page once wired.
