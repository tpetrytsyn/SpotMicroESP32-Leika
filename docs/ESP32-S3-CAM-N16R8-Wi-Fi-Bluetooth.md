# ESP32-S3-CAM N16R8 (HW-679 V0.0.1)

Hardware reference for the ESP32-S3-CAM board used as an alternative to the
AI-Thinker ESP32-CAM in this project.

- **Silkscreen:** `ESP32-S3-CAM`, `HW-679`, `V0.0.1`
- **Module:** ESP32-S3-WROOM-1-N16R8 (Xtensa LX7 dual-core, up to 240 MHz)
- **Shipped camera:** RHYX M21-45 — **not usable with this firmware**, see [Camera](#camera)
- **Source:** [prom.ua listing](https://prom.ua/ua/p1401063322-esp32-cam-n16r8.html)

Confidence is marked throughout: **[listing]** = stated by the vendor,
**[image]** = read off the board pinout diagram, **[chip]** = follows from the
ESP32-S3 / WROOM-1 datasheet regardless of board, **[inferred]** = reasoned,
needs confirmation against hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Chip | ESP32-S3-N16R8, dual-core Xtensa LX7, ≤240 MHz | [listing] |
| Flash | 16 MB, quad SPI (3.3 V) | [listing] + [chip] |
| PSRAM | 8 MB, **Octal** SPI | [listing] + [chip] |
| SRAM | 512 KB | [listing] |
| Wireless | Wi-Fi 802.11 b/g/n, Bluetooth 5 (LE) | [listing] |
| Wi-Fi modes | STA / AP / STA+AP | [listing] |
| Camera connector | FPC, 2 MP max (1600×1200) | [listing] |
| Camera support | "OV2640/OV5640 та ін." | [listing] |
| Image formats | JPEG, BMP, grayscale | [listing] |
| Storage | microSD slot | [listing] |
| Interfaces | GPIO, I2C, SPI, UART, ADC, PWM, USB OTG | [listing] |
| Buttons | Reset button on board | [listing] + [image] |
| Power | **5 V via pins** — no USB connector | [listing] |
| Dimensions | 40 × 27 × 12 mm | [listing] |
| Weight | 7 g | [listing] |

The 40 × 27 mm footprint matches the AI-Thinker ESP32-CAM, but **the header
pinout does not** — do not assume ESP32-CAM wiring carries over.

## Header pinout

All pin assignments below are **[image]**, read from the vendor pinout diagram.

### Left header (top → bottom)

| Silkscreen | GPIO | Function |
| --- | --- | --- |
| `RST` | — | Chip reset (EN). Pull low to reset. |
| `TX` | **IO43** | UART0 TX |
| `RX` | **IO44** | UART0 RX |
| `19` | **IO19** | Free GPIO (chip's USB D−, unused here) |
| `20` | **IO20** | Free GPIO (chip's USB D+, unused here) |
| `0` | **IO0** | Boot strapping pin — low at reset = download mode |
| `GND` | — | Ground |
| `3V3` | — | 3.3 V rail |

### Right header (top → bottom)

| Silkscreen | GPIO | Function |
| --- | --- | --- |
| `41` | **IO41** | Free GPIO |
| `40` | **IO40** | Free GPIO / microSD D0 |
| `39` | **IO39** | Free GPIO / microSD CLK |
| `38` | **IO38** | Free GPIO / microSD CMD |
| `47` | **IO47** | Free GPIO |
| `I14` | **IO14** | Free GPIO |
| `GND` | — | Ground |
| `VCC` | — | **5 V input** |

Every exposed GPIO: **0, 14, 19, 20, 38, 39, 40, 41, 43, 44, 47**.

## GPIO budget

Of the ESP32-S3's 45 GPIOs (0-21, 26-48), most are consumed on this board:

| GPIO | Consumed by | Source |
| --- | --- | --- |
| 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 17, 18 | Camera (DVP bus) | [inferred] |
| 26-32 | Internal SPI flash | [chip] |
| 33-37 | **Octal PSRAM** — permanently unavailable on R8 parts | [chip] |
| 38, 39, 40 | microSD (SDMMC 1-bit) | [inferred] |
| 43, 44 | UART0 — flashing and console | [image] |

The exposed pins are almost exactly the complement of the camera bus plus the
memory pins, which is a useful consistency check on the camera pinout below.

**Genuinely free for peripherals: IO14, IO19, IO20, IO41, IO47** — plus
IO38/39/40 if you don't use the microSD slot. IO19/20 are the chip's USB D−/D+,
but since this board is flashed over UART0 they are ordinary GPIO here.
IO43/44 must stay free for the serial link.

`IO0` is a strapping pin: it must be high at reset for normal boot. Usable as an
output afterwards, but never wire it to something that pulls it low at power-on.

## Camera

### The shipped sensor does not work with this firmware

The board ships with an **RHYX M21-45**, which is a **GC2415** sensor with **no
onboard JPEG encoder** — it outputs only RGB565, YCbCr422, and raw Bayer. Two
independent blockers:

1. [`camera_service.cpp:69`](../esp32/src/peripherals/camera_service.cpp#L69)
   hardcodes `camera_config.pixel_format = PIXFORMAT_JPEG`, and the HTTP paths
   (`cameraStill`, `cameraStream`) send `image/jpeg` straight from the framebuffer.
2. There is no `gc2415.c` in
   [`managed_components/espressif__esp32-camera/sensors/`](../managed_components/espressif__esp32-camera/sensors/),
   so `esp_camera_init()` fails the SCCB probe outright.

**Replace it.** OV2640, OV3660 and OV5640 are all supported — `ov2640.c`,
`ov3660.c`, `ov5640.c` are present and their `CONFIG_*_SUPPORT` options default
to `y`. OV3660 (3 MP, hardware JPEG) is a good choice; the init code is
sensor-agnostic and probes SCCB to select the driver, so **no code change is
needed for the sensor swap**.

### Camera pin mapping — [verified on hardware]

The DVP pins are not broken out, so they cannot be read off the board. This
board follows the Espressif reference layout, already present in this repo as
`CAMERA_MODEL_ESP32S3_EYE` at
[`camera_pins.h:275-293`](../esp32/include/peripherals/camera_pins.h#L275-L293):

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

Confirmed working with an OV3660 fitted. Boot log:

```
CameraService: Initializing camera
sccb-ng: pin_sda 4 pin_scl 5
sccb-ng: sccb_i2c_port=1
camera: Camera PID=0x3660 VER=0x00 MIDL=0x00 MIDH=0x00
camera: Detected OV3660 camera
camera: Detected camera at address=0x3c
CameraService: Camera probe successful
```

Both `/api/camera/still` and `/api/camera/stream` serve correctly over the AP.

**`cam_hal: FB-OVF` during streaming is a warning, not a failure.** It means the
sensor produced a frame faster than DMA could store it, so that frame was
dropped; the stream continues at a lower rate. The OV3660 is a 3 MP sensor and
the defaults (`FRAMESIZE_SVGA`, `jpeg_quality = 10`) are demanding. To reduce
it, lower the XCLK in
[`camera_service.cpp:68`](../esp32/src/peripherals/camera_service.cpp#L68) from
20 MHz to 10 MHz, or raise `jpeg_quality` to ~12. On the ESP32-S3 the XCLK is
divided from 80 MHz, so it must divide evenly — 20, 16 and 10 MHz are valid.

## Programming — CH343P on UART0

There is no USB connector and no onboard USB-serial bridge, so firmware is
flashed over UART0 with a USB-TTL adapter. **This firmware implements no OTA**
(no `esp_ota_begin` / `esp_ota_write` anywhere in `esp32/`; the `ota_0`/`ota_1`
partitions are never written), so serial is the only route in — permanently.
Mount the board so the left header stays reachable after assembly.

### Wiring

| Adapter | Board |
| --- | --- |
| TX | `RX` (IO44) |
| RX | `TX` (IO43) |
| GND | `GND` — required, common ground |
| 5 V | `VCC` |

**Before the first connection, confirm the adapter's logic level is 3.3 V.**
The WeAct CH343P has a `VIO` jumper; at 5 V the adapter drives 5 V into IO44 and
the ESP32-S3 is not 5 V tolerant on GPIO. Measure it. See
[CH343P-USB-Serial-WeAct.md](CH343P-USB-Serial-WeAct.md) for that adapter's
pins, level selection, and driver notes.

Power: the adapter's **5 V** pin is USB VBUS passed through and is fine for
bench flashing a standalone board — `VCC` feeds the board's own regulator. Do
**not** use the adapter's 3.3 V pin; that is an LDO rail with far too little
headroom. A USB port's 500 mA is comfortable for the bare board and marginal
once the camera is attached — use a powered hub or a separate 5 V supply if you
see brownout resets. In the assembled robot, power `VCC` from the robot's own
5 V supply instead, keeping the adapter ground common.

### Manual bootloader entry

There is no auto-reset circuit on this board, so DTR/RTS do nothing:

1. Pull `IO0` to `GND`
2. Pulse `RST` to `GND` briefly, then release
3. Release `IO0`
4. Run `pio run -t upload` — the log should show `waiting for download`

Start at `upload_speed = 460800` and raise once stable. On Windows, CH343 may
need WCH's `CH343SER` driver package if Windows Update does not supply it.

## PlatformIO environment

The env is `[env:s3cam]` in [`platformio.ini`](../platformio.ini). Neither
existing S3 env fits: `[env:esp32-wroom-camera]` is configured for 8 MB flash and
uses `SCL_PIN=21` / `WS2812_PIN=48`, **and neither IO21 nor IO48 is broken out on
this board**.

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
```

Verified build output: RAM 33.1% (108,336 / 327,680), Flash 18.0%
(1,182,595 / 6,553,600).

Notes:

- `qio` is correct because N16R8 (no `V` suffix) has **quad** SPI flash. An
  N16R8**V** module has OPI flash and needs `CONFIG_ESPTOOLPY_FLASHMODE_OPI`.
- PSRAM needs no extra config: [`sdkconfig.defaults`](../sdkconfig.defaults)
  already sets `CONFIG_SPIRAM_MODE_OCT=y`, correct for R8. Confirmed at boot:
  `octal_psram: density 0x03 (64 Mbit)`, `esp_psram: Found 8MB PSRAM device`,
  `Speed: 80MHz`, `SPI SRAM memory test OK`.
- [`esp32/partition_table/default_16MB.csv`](../esp32/partition_table/default_16MB.csv)
  gives 6.4 MB per app slot. Confirmed at boot: `SPI Flash Size : 16MB`, all six
  partitions loaded, LittleFS mounted with 3,538,944 bytes.
- `monitor_rts`/`monitor_dtr` are zeroed because nothing drives the reset lines.
- PlatformIO prints `Warning! Flash memory size mismatch detected. Expected 16MB,
  found 2MB!` and the generated `sdkconfig.s3cam` says `FLASHSIZE="2MB"`. **This
  is cosmetic** - esptool writes the correct size into the image header at flash
  time and the bootloader reports `SPI Flash Size : 16MB`. No fix needed.

### ESP-IDF 5.5 is mandatory - and why

The default `espressif32 @ 6.8.1` ships **IDF 5.3.0**, which cannot build this
board with the camera enabled. ESP-IDF has two mutually exclusive I2C APIs and
aborts if both are linked:

| API | Header | Used by |
| --- | --- | --- |
| Legacy | `driver/i2c.h` | esp32-camera's `sccb.c` |
| New (driver_ng) | `driver/i2c_master.h` | this project's [`i2c_bus.h`](../esp32/include/peripherals/i2c_bus.h) |

IDF's legacy `i2c.c` registers `check_i2c_driver_conflict()` as a
`__attribute__((constructor))`. It runs during `do_global_ctors`, **before
`app_main`**, and calls `abort()` if driver_ng is also linked - so the board
boot-loops and never reaches a line of project code.

[`esp32-camera/CMakeLists.txt:90`](../managed_components/espressif__esp32-camera/CMakeLists.txt#L90)
selects `sccb-ng.c` (new driver, no conflict) only for **IDF >= 5.4**; below that
it selects legacy `sccb.c`. Hence the pioarduino platform pinned above, which
ships IDF 5.5.5.

This affects **every camera env in this repo**, not just this board. The
driver_ng migration landed in commit `d81b1b0` ("Working camera stream with p4")
on 2026-02-06. It went unnoticed because on ESP32-P4 `esp32-camera` is excluded
entirely (`target not in [esp32p4]` - P4 uses `esp_cam_sensor` for MIPI-CSI) and
`[env:esp32-p4]` already uses a newer IDF. It is a known ecosystem problem:
[esp32-camera #741](https://github.com/espressif/esp32-camera/issues/741),
[#713](https://github.com/espressif/esp32-camera/issues/713).

### Building on Windows - required setup

IDF 5.5 imposes two constraints that IDF 5.3 did not.

**1. Build from PowerShell, never Git Bash.** `idf_tools.py` refuses to run under
MSYS/MinGW with `ERROR: MSys/Mingw is not supported`. Git Bash exports
`MSYSTEM`, so every `pio` invocation for this env - build *and* upload - must go
through PowerShell or cmd.

**2. Paths must be short.** The `ldgen.py` step that generates `sections.ld` runs
through `cmd.exe`, which caps command lines at **8,191 characters**. That command
passes one absolute path per linker fragment; measured at **8,324 chars**, i.e.
133 over. 90% of it is the `--fragments` list - 60 paths averaging 122
characters, of which ~100 are a repeated prefix.

Two directory junctions bring it under the limit. Neither copies any files, and
both are removable with `rmdir`:

```powershell
New-Item -ItemType Junction -Path C:\pio -Target $env:USERPROFILE\.platformio
New-Item -ItemType Junction -Path D:\s   -Target <repo path>
```

with `core_dir = C:/pio` set in `[platformio]`, and builds run from `D:\s`. The
env is named `s3cam` rather than something descriptive for the same reason - the
name appears 5x in that command line.

```powershell
cd D:\s
pio run -e s3cam
pio run -e s3cam -t upload --upload-port COM3
```

**This is fragile.** After the junctions and the short env name there are only
~58 characters of headroom. Adding a component, a build flag, or a longer path
can push it back over, and the failure reads
`The command line is too long.` against the `sections.ld` target.

Note also that the two platforms share package names (`tool-cmake`,
`framework-espidf`, `toolchain-xtensa-esp-elf`) and **evict each other**.
Building an IDF 5.3 env re-downloads its toolchain over the 5.5 one and vice
versa. Never run two builds against different platforms concurrently - killing
one mid-unpack corrupts the shared package (symptom:
`CMake Error: Could not find CMAKE_ROOT`; fix: delete the package directory and
rebuild).

### I2C for the servo controller

The PCA9685 needs only SDA and SCL. `IO47` and `IO41` are both free and neither
touches the camera bus, the memory pins, or the microSD slot. `IO14` is left for
the WS2812. If you need the microSD slot, IO38/39/40 become unavailable and the
free-pin budget drops to IO14/41/47 only.

## Firmware bugs found during bring-up

Three defects were hit bringing this board up. All are in the project, not the
board, and none is specific to this hardware.

1. **I2C driver conflict** - described above. Worked around with `USE_CAMERA=0`;
   fixed properly by IDF >= 5.4.
2. **The soft AP never started.** `APService::begin()` only read persistence and
   relied on `manageAP()` to notice. But `manageAP()` treats "radio mode == AP"
   as proof the AP is configured, and the mode can already be `WIFI_MODE_AP`
   without `softAP()` ever having run - a state it cannot recover from. Fixed by
   setting `_reconfigureAp = true` in `begin()`.
3. **`softAP()` never started the radio.** The only `esp_wifi_start()` call sits
   in `WiFiClass::mode()`, on the `WIFI_MODE_NULL -> x` transition. `softAP()`
   skips `mode()` when the radio is already in AP mode, so the SSID was
   configured but no beacon transmitted - the AP was invisible while the log
   claimed success. Fixed by calling `esp_wifi_start()` in `softAP()` and
   resyncing the cached `_mode`.

Bugs 2 and 3 compound: any build that boots without stored Wi-Fi credentials is
unreachable over the network, and `FACTORY_WIFI_SSID` is empty by default.

A latent issue, not fixed: [`camera_service.cpp:17-36`](../esp32/src/peripherals/camera_service.cpp#L17-L36)
creates `cameraMutex` with `xSemaphoreCreateMutex()` (non-recursive) but uses
`xSemaphoreTakeRecursive`/`GiveRecursive` on it, and `cameraStream` calls
`safe_sensor_return()` on a mutex that `safe_camera_fb_get()` has already
released. Harmless in practice - FreeRTOS returns `pdFAIL` - but wrong.

## Bench bring-up, verified

With no PCA9685 and no servos attached the board boots fully. `PCA9685Driver::begin()`
probes the I2C address and returns cleanly if nothing answers, so an empty bus is
harmless - though it logs repeated `i2c.master: I2C transaction unexpected nack`
and drags the 100 Hz control loop down to ~75-84 Hz until a device is connected.

With `FACTORY_WIFI_SSID` empty and `FACTORY_AP_PROVISION_MODE=AP_MODE_DISCONNECTED`
the board starts its own AP: **`Spot-Micro` / `spot-leika`** at **192.168.4.1**.
`http://192.168.4.1/api/camera/still` returns a JPEG directly - the quickest
end-to-end camera check.

With `EMBED_WEBAPP=0` the root URL returns **405**, not the web UI:
[`main.cpp:117`](../esp32/src/main.cpp#L117) registers the static-asset and
SPA-fallback handlers only under that flag, leaving `/*` bound to `HTTP_OPTIONS`
alone. API routes still work. Building the UI needs `pnpm` and `protoc`;
`esp32/include/WWWData.h` is a generated, gitignored stub until then.

## Open items

- [x] ~~Confirm the camera pin mapping~~ - verified, `ESP32S3_EYE` is correct
- [ ] Confirm whether `3V3` is a regulator output or an input
- [ ] Confirm microSD is wired to IO38/39/40 if you want both SD and I2C
- [ ] Confirm the module is N16R8 and not N16R8V (flash mode differs)
- [ ] Tune XCLK / `jpeg_quality` if `FB-OVF` frame drops matter
- [ ] Re-enable `EMBED_WEBAPP=1` once `pnpm` and `protoc` are installed
