# Build and upload

Compiling the firmware with the embedded web app and flashing it to the
[ESP32-S3-CAM](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md) (`s3cam` env) from Windows. There's
no OTA: every update goes over UART0.

## 1. One-time setup

| Tool | Why |
| --- | --- |
| [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/) (`pio`) | Firmware build and upload; pulls ESP-IDF 5.5 itself |
| Node.js + `pnpm` | The web app, embedded into the firmware |
| `protoc` on `PATH` | TypeScript protos (`pnpm proto`); nanopb uses its own Python generator |
| Git | The `submodules/nanopb` submodule |
| [CH343P adapter](components/CH343P-USB-Serial-WeAct.md), VIO at 3.3 V | The only way to flash |

1. Clone with the submodule, or the build fails:
   ```powershell
   git clone --recurse-submodules git@github.com:tpetrytsyn/SpotMicroESP32-Leika.git
   # existing clone: git submodule update --init
   ```
2. Short-path junctions. IDF 5.5's `ldgen` command line exceeds `cmd.exe`'s 8,191-character limit
   on full paths; `platformio.ini` already sets `core_dir = C:/pio`.
   ```powershell
   New-Item -ItemType Junction -Path C:\pio -Target $env:USERPROFILE\.platformio
   New-Item -ItemType Junction -Path D:\s   -Target <repo path>
   ```
3. Web app dependencies and generated protos:
   ```powershell
   cd D:\s\app
   pnpm install
   pnpm proto
   ```

## 2. Build

**PowerShell or cmd, from `D:\s` — never Git Bash** (IDF's `idf_tools.py` rejects MSYS).

```powershell
cd D:\s
pio run -e s3cam
```

Every build runs two scripts first:

| Script | Does |
| --- | --- |
| `pre_build.py` → `compile_protos.py` | `platform_shared/*.proto` → nanopb C in `esp32/src/platform_shared/` |
| `build_app.py` (`EMBED_WEBAPP=1`) | `pnpm install && pnpm run build:embedded` in `app/`, gzipped into `esp32/include/WWWData.h` — **only if a file under `app/src` is newer** than it |

- **After a `.proto` change, run `pnpm proto` in `app/` first.** `build:embedded` doesn't
  regenerate the TypeScript, and because the output lands in `app/src`, running it also triggers
  the web rebuild.
- **Force a web rebuild** by touching any file under `app/src`, or deleting `WWWData.h`.
- **Timing:** a clean build takes ~9 min; an incremental one, ~1.5 min.
- `Error when using sourcemap` lines from `svelte-dnd-list` during the web build are noise.
- Success ends with `[SUCCESS]` and the RAM/Flash summary — ~33 % RAM, ~28 % of the 6.4 MB slot.

Compile-time config lives in [features.ini](../esp32/features.ini) (`USE_*`, variant),
[factory_settings.ini](../esp32/factory_settings.ini) and [build_settings.ini](../esp32/build_settings.ini).
Board pins and `I2C_FREQUENCY` are in the `s3cam` env of [platformio.ini](../platformio.ini).

## 3. Connect the adapter

| Adapter | Board |
| --- | --- |
| `TX` | `RX` (IO44) |
| `RX` | `TX` (IO43) |
| `GND` | `GND` |
| `5V` | `VCC` — **standalone board only** |

- **On the harness:** rocker **on**, `GND`/`TX`/`RX` only. The adapter's 5 V back-feeds through
  the CN3903.
- Find the port with `pio device list`: the CH343 shows as `USB-Enhanced-SERIAL CH343`
  (`VID:PID=1A86:55D3`), here **COM3**. The Bluetooth COM ports are not it.

## 4. Upload

The board has **no auto-reset circuit**: enter the bootloader by hand, every time.

1. Hold **`IO0`** low, pulse **`RST`**, release **`IO0`**. The serial log shows
   `waiting for download`.
2. Upload — it rebuilds first if anything changed, so build before entering download mode:
   ```powershell
   pio run -e s3cam -t upload --upload-port COM3
   ```
   Expect `Hash of data verified` and `[SUCCESS]` in under a minute at 460800 baud.
3. **Press `RST`.** The closing `Hard resetting via RTS pin` does nothing on this board.

No `uploadfs`: `esp32/data` is empty, and settings on LittleFS — Wi-Fi, servo calibration,
peripherals — survive a firmware upload. `pio run -e s3cam -t erase` wipes them too.

## 5. Check

```powershell
pio device monitor -p COM3      # 115200 baud; Ctrl+C to exit
```

- The boot log prints the feature flags, then `Service task started`.
- Open the UI at `http://spot-micro.local` (home network) or join `Spot-Micro` / `spot-leika`
  and open `http://192.168.4.1` — see [the board doc](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md#bench-notes).

## Troubleshooting

| Symptom | Cause / fix |
| --- | --- |
| `The command line is too long.` | Not building from `D:\s`, or the junctions are missing. Only ~58 characters of headroom remain — a new component or flag can break it again. |
| `idf_tools.py` rejects the environment | Running from Git Bash — use PowerShell. |
| `Could not find CMAKE_ROOT` | A package corrupted mid-unpack: the IDF 5.3 and 5.5 platforms share package names and evict each other. Delete the package under `C:\pio\packages` and rebuild; never build both at once. |
| Missing `nanopb` / `pb.h` | Submodule not checked out — `git submodule update --init`. |
| Upload: `Failed to connect` / `No serial data received` | Not in download mode — repeat the `IO0`/`RST` sequence; check TX↔RX are crossed. |
| UI missing a change | `WWWData.h` wasn't rebuilt — touch a file under `app/src`. |
| Firmware and UI disagree on a message | Stale generated TS — `pnpm proto`, then rebuild. |
| `Flash memory size mismatch … found 2MB` | Cosmetic; the bootloader reports 16 MB. |
