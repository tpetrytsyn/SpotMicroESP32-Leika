# ESP32-S3-CAM N16R8 (HW-679 V0.0.1)

The robot's controller, used instead of the AI-Thinker ESP32-CAM. Sourced from
[prom.ua](https://prom.ua/ua/p1401063322-esp32-cam-n16r8.html).

| Item | Value | Source |
| --- | --- | --- |
| Silkscreen | `ESP32-S3-CAM`, `HW-679`, `V0.0.1` | photo |
| Module | ESP32-S3-WROOM-1-N16R8: dual-core LX7 ≤ 240 MHz, 512 KB SRAM | listing |
| Memory | 16 MB **quad** SPI flash, 8 MB **octal** PSRAM | listing + datasheet |
| Wireless | 802.11 b/g/n (STA / AP / STA+AP), Bluetooth 5 LE | listing |
| Camera | FPC connector, up to 2 MP, "OV2640/OV5640 etc." | listing |
| Other | microSD slot, reset button | listing |
| Power | **5 V on `VCC`** — no USB connector, no USB-serial bridge | listing |
| Size | 40 × 27 × 12 mm, 7 g | listing |

The footprint matches the AI-Thinker ESP32-CAM; **the header pinout does not**.

## Header pinout

From the vendor's pinout diagram.

| Left (top → bottom) | GPIO | Function |
| --- | --- | --- |
| `RST` | — | Chip reset (EN); pull low to reset |
| `TX` | IO43 | UART0 TX |
| `RX` | IO44 | UART0 RX |
| `19` | IO19 | Free (chip's USB D−, unused) |
| `20` | IO20 | Free (chip's USB D+, unused) |
| `0` | IO0 | Strapping: low at reset = download mode |
| `GND` | — | Ground |
| `3V3` | — | 3.3 V rail |

| Right (top → bottom) | GPIO | Function |
| --- | --- | --- |
| `41` | IO41 | Free — I2C `SCL` |
| `40` | IO40 | Free / microSD D0 |
| `39` | IO39 | Free / microSD CLK |
| `38` | IO38 | Free / microSD CMD |
| `47` | IO47 | Free — I2C `SDA` |
| `I14` | IO14 | Free — WS2812 |
| `GND` | — | Ground |
| `VCC` | — | **5 V input** |

## GPIO budget

| GPIO | Used by | Source |
| --- | --- | --- |
| 4–13, 15–18 | Camera DVP bus | verified, below |
| 26–32 | Internal SPI flash | datasheet |
| 33–37 | Octal PSRAM — never available on R8 parts | datasheet |
| 38–40 | microSD (SDMMC 1-bit) | inferred |
| 43, 44 | UART0 — flashing and console | pinout |

**Free: IO14, IO19, IO20, IO41, IO47**, plus IO38–40 without the microSD slot. IO19/20 are
ordinary GPIO here, since the board flashes over UART0. `IO0` must be high at reset: never wire
it to something that pulls it low at power-on.

**IO47 (and IO48) sit in the VDD_SPI power domain** and run at the module's memory voltage
**[datasheet]**: 3.3 V on this N16R8, 1.8 V on 1.8 V-memory variants — which 3.3 V I2C pull-ups
would damage. [Build step 1](../pre-calibration-build.md#1-esp32-alone-flash-and-check) meters it.

## Camera

The board ships with an **RHYX M21-45 (GC2415)**, which **doesn't work with this firmware**: it
has no JPEG encoder while [camera_service.cpp](../../esp32/src/peripherals/camera_service.cpp)
hardcodes `PIXFORMAT_JPEG`, and esp32-camera has no GC2415 driver, so the SCCB probe fails.
**Fitted instead: an OV3660** (OV2640 and OV5640 also work). The init code probes SCCB to pick the
driver — no code change.

The DVP pins aren't broken out; the board follows the Espressif reference,
`CAMERA_MODEL_ESP32S3_EYE` in [camera_pins.h](../../esp32/include/peripherals/camera_pins.h) —
**verified on hardware** with the OV3660 (`camera: Detected OV3660 camera`).

| Signal | GPIO | Signal | GPIO |
| --- | --- | --- | --- |
| PWDN | −1 | Y2 (D0) | 11 |
| RESET | −1 | Y3 (D1) | 9 |
| XCLK | 15 | Y4 (D2) | 8 |
| SIOD (SDA) | 4 | Y5 (D3) | 10 |
| SIOC (SCL) | 5 | Y6 (D4) | 12 |
| VSYNC | 6 | Y7 (D5) | 18 |
| HREF | 7 | Y8 (D6) | 17 |
| PCLK | 13 | Y9 (D7) | 16 |

`/api/camera/still` and `/api/camera/stream` both work. `cam_hal: FB-OVF` is a dropped-frame
warning, not a failure; to reduce it, lower XCLK (20 → 10 MHz; it divides 80 MHz, so 20, 16 and
10 are valid) or raise `jpeg_quality` to ~12.

## Programming

No OTA in this firmware, so **serial over UART0 is the only way in** — keep the left header
reachable. Adapter: [CH343P](CH343P-USB-Serial-WeAct.md), its VIO set to 3.3 V (the ESP32-S3 isn't
5 V tolerant).

| Adapter | Board |
| --- | --- |
| `TX` | `RX` (IO44) |
| `RX` | `TX` (IO43) |
| `GND` | `GND` |
| `5V` | `VCC` — **standalone board only** |

- **On the robot's harness, no `5V` wire:** switch the rocker on and connect `GND`/`TX`/`RX`
  only. The adapter's 5 V would back-feed through the CN3903, or fight it.
- Standalone, the adapter's `5V` (USB VBUS) is fine but marginal with the camera; never use its
  `3V3` pin.
- **No auto-reset circuit**, so enter the bootloader by hand: hold `IO0` low, pulse `RST`,
  release `IO0`, then upload — the log shows `waiting for download`.
- `upload_speed = 460800` is reliable.

## PlatformIO environment

```ini
[env:s3cam]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.311/platform-espressif32.zip
board = esp32-s3-devkitc-1
board_build.flash_mode = qio
board_upload.flash_size = 16MB
board_build.partitions = esp32/partition_table/default_16MB.csv
upload_speed = 460800
monitor_rts = 0
monitor_dtr = 0
build_flags =
	${env.build_flags}
	-DBOARD_HAS_PSRAM
	-D USE_CAMERA=1
	-D CAMERA_MODEL_ESP32S3_EYE=1
	-D SDA_PIN=47
	-D SCL_PIN=41
	-D WS2812_PIN=14
	; 400 kHz is the ADS1115's maximum; the global default in global.h is 1 MHz
	-D I2C_FREQUENCY=400000
```

- `qio`: N16R8 has quad flash. A module with octal flash would need OPI mode.
- `I2C_FREQUENCY` only sets the default: a board with saved peripheral settings keeps its old
  value until changed on the I2C page.
- Octal PSRAM comes from `CONFIG_SPIRAM_MODE_OCT=y` in [sdkconfig.defaults](../../sdkconfig.defaults)
  (boot: `Found 8MB PSRAM device`).
- 6.4 MB per app slot; LittleFS ~3.5 MB.
- `monitor_rts`/`monitor_dtr` are zeroed because nothing drives the reset lines.
- The `Flash memory size mismatch … found 2MB` warning is cosmetic: the bootloader reports 16 MB.
- The existing `esp32-wroom-camera` env doesn't fit: 8 MB flash, and `SCL_PIN=21` / `WS2812_PIN=48`
  aren't broken out here.

### ESP-IDF 5.5 is required

The default `espressif32 @ 6.8.1` ships IDF 5.3, which links esp32-camera's legacy `sccb.c`
(`driver/i2c.h`) next to the project's driver_ng [i2c_bus.h](../../esp32/include/peripherals/i2c_bus.h).
IDF aborts on that in a global constructor, before `app_main`, so the board boot-loops.
esp32-camera picks `sccb-ng.c` only on IDF ≥ 5.4, hence the pioarduino platform (IDF 5.5.5). This
affects every camera env except the P4 ([esp32-camera #741](https://github.com/espressif/esp32-camera/issues/741),
[#713](https://github.com/espressif/esp32-camera/issues/713)).

### Building on Windows

- **PowerShell or cmd only, never Git Bash** — IDF 5.5's `idf_tools.py` rejects MSYS, for
  uploads too.
- **Paths must be short.** The `ldgen` step runs through `cmd.exe`, capped at 8,191 characters,
  and the full-path command line ran over. Two junctions fix it:
  ```powershell
  New-Item -ItemType Junction -Path C:\pio -Target $env:USERPROFILE\.platformio   # core_dir = C:/pio
  New-Item -ItemType Junction -Path D:\s   -Target <repo path>
  cd D:\s
  pio run -e s3cam -t upload --upload-port COM3
  ```
  Only ~58 characters of headroom remain: a new component, flag or longer path can break it
  again (`The command line is too long.`).
- The IDF 5.3 and 5.5 platforms share package names and evict each other. Never build both at
  once; a package corrupted mid-unpack shows `Could not find CMAKE_ROOT` — delete it and rebuild.

## Bench notes

- With no stored Wi-Fi the board starts its own AP, **`Spot-Micro` / `spot-leika`**, at
  `192.168.4.1`. `/api/camera/still` returning a JPEG is the quickest end-to-end check.
- An empty I2C bus is harmless but logs `i2c.master: I2C transaction unexpected nack` and slows the
  control loop to ~75–84 Hz until the PCA9685 is connected.
- With `EMBED_WEBAPP=0` the root URL returns 405 — only the API is served.
- Fixed during bring-up, see git history: the I2C driver conflict (`e921138`), a soft AP that never
  broadcast (`6b66415`), and first-network provisioning over the AP never connecting (`d4b6aa8`).
- Latent, not fixed: `camera_service.cpp` creates `cameraMutex` non-recursive but takes it
  recursively — harmless (`pdFAIL`), but wrong.

## Open items

- [x] Camera pin mapping — `ESP32S3_EYE` verified
- [ ] `3V3` is a regulator output? — [build step 9](../pre-calibration-build.md#9-esp32-onto-the-harness)
- [ ] microSD wired to IO38/39/40
- [ ] Module is N16R8 (quad flash, 3.3 V memory) — read the shield; it also sets IO47's level
- [ ] Tune XCLK / `jpeg_quality` if `FB-OVF` drops matter
