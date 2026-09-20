# CH343P USB-Serial adapter (WeAct Studio USB2Serial V1)

USB-to-UART adapter used to flash and monitor the
[ESP32-S3-CAM N16R8](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md), which has no USB
connector and no onboard serial bridge.

- **Board silkscreen:** `USB2UART`, `CH343`, `6Mbps Max.`, `WeAct Studio`, `V1.0`
- **Chip:** WCH CH343P
- **Vendor:** WeAct Studio — [WeActStudio.USB2SerialV1](https://github.com/WeActStudio/WeActStudio.USB2SerialV1)
- **Source:** [rcscomponents.kiev.ua listing](https://www.rcscomponents.kiev.ua/product/peretvoriuvach-ch343p-usb-serial-port-weact-studio_198651.html)

Sources are marked: **[listing]** = vendor product page, **[board]** = read off
the product photographs, **[chip]** = CH343 datasheet / WCH documentation.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Controller | CH343P | [listing] |
| USB | Full-speed USB 2.0 device | [listing] |
| Connector | **USB Type-C** | [board] |
| Baud range | **50 bps – 6 Mbps** | [board] + [chip] |
| Auto-baud | Optional auto-detect at 115200 and below | [listing] |
| Data bits | 5, 6, 7, 8 | [listing] |
| Parity | odd / even / mark / space / none | [listing] |
| Modem signals | RTS, DTR, DCD, RI, DSR, CTS | [listing] |
| Flow control | Hardware CTS/RTS auto flow control | [listing] |
| Half-duplex | Supported, with RS485 direction switching | [listing] |
| I/O voltage | **Selectable 3.3 V / 5 V / custom 1.8–5 V** | [board] + [chip] |
| Unique ID | USB serial number burned into the chip | [listing] |
| Dimensions | 35 × 13 mm | [listing] |
| Indicator | `READY` LED | [board] |
| Accessory | One jumper cap included | [board] |

> **The listing's "maximum speed 115200 bps" is wrong.** The board's own
> silkscreen says `6Mbps Max.`, and the same listing's technical section says
> `50 ~ 6 Мбіт/с`. The 115200 figure refers to the chip's optional automatic
> baud-rate detection feature, which works at 115200 and below — not to the
> maximum usable rate.

## Pins

Signal labels present on the header, split across both sides of the board
**[board]**:

| Label | Meaning |
| --- | --- |
| `TX` | Adapter transmit → target's RX |
| `RX` | Adapter receive ← target's TX |
| `GND` | Ground |
| `5V` | USB VBUS passed through |
| `3V3` | Onboard regulator output |
| `VIO` | I/O reference rail for TX/RX/RTS/DTR — see below |
| `RTS` | Modem control, usable for auto-reset |
| `DTR` | Modem control, usable for auto-bootloader |

The front silkscreen carries `TX`, `RX`, `GND`, `3V3`, `5V`; the back carries
`DTR`, `RTS`, and the `3V3` / `VIO` / `5V` level-select legend.

> **Confirm the physical pin order against your board before wiring.** The
> product photographs do not resolve the order unambiguously, and WeAct's
> repository does not publish a pin table. Read the silkscreen directly, or
> buzz out `GND` with a multimeter against the USB shell to find your reference
> pin. Nothing below depends on the order — match by label.

## VIO — I/O level selection (read this first)

`VIO` sets the logic level of **TX, RX, RTS and DTR**. It is the single most
important setting on this adapter: the ESP32-S3 is **not 5 V tolerant on GPIO**,
so an adapter left at 5 V TTL will drive 5 V into `IO44` and damage the chip.

Two ways to set it, both shown on the back silkscreen **[board]**:

- **Jumper cap** across `VIO` ↔ `3V3` or `VIO` ↔ `5V` (the included cap)
- **Solder jumper**, a three-position pad array labelled `3V3 TTL` / `CUSTOM` / `5V TTL`

| VIO connected to | TX/RX/RTS/DTR level |
| --- | --- |
| `3V3` | **3.3 V — use this for the ESP32-S3** |
| `5V` | 5 V |
| Target's own supply | Follows the target, valid 1.8–5 V |

The `CUSTOM` position leaves `VIO` floating so you can feed it the target's
rail. Do not leave `VIO` unconnected with no jumper — the outputs have no
reference.

**Before the first connection:** set the jumper to `3V3`, plug in the adapter
with nothing else attached, and measure `TX` to `GND`. It must read ~3.3 V.

## Wiring to the ESP32-S3-CAM

| Adapter | ESP32-S3-CAM |
| --- | --- |
| `TX` | `RX` (IO44) |
| `RX` | `TX` (IO43) |
| `GND` | `GND` — required, common ground |
| `5V` | `VCC` |

`5V` is USB VBUS passed through and is fine for bench-flashing a standalone
board. Do **not** use the adapter's `3V3` pin as the board supply — it is a
small LDO rail. In the assembled robot, power `VCC` from the robot's own 5 V
supply and keep only the adapter's ground common.

A USB port's 500 mA is comfortable for the bare ESP32-S3 and marginal once the
camera is attached; use a powered hub or a separate 5 V supply if you see
brownout resets.

## Auto-reset (optional)

Because this adapter breaks out **both DTR and RTS**, the standard ESP32
auto-program circuit can be built, removing the manual boot-mode sequence:

| DTR | RTS | EN | IO0 |
| --- | --- | --- | --- |
| 1 | 1 | 1 | 1 |
| 0 | 0 | 1 | 1 |
| 1 | 0 | 0 | 1 |
| 0 | 1 | 1 | 0 |

This is the usual two-NPN cross-coupled pair — each transistor conducts only
when its own control line is high while the other is low, so neither line alone
can hold the board in reset. `RTS` drives `EN`, `DTR` drives `IO0`.

**Without that circuit**, DTR/RTS do nothing useful and must be kept from
interfering. The PlatformIO env therefore sets:

```ini
monitor_rts = 0
monitor_dtr = 0
```

and the bootloader is entered by hand:

1. Pull `IO0` to `GND`
2. Pulse `RST` to `GND` briefly, then release
3. Release `IO0`
4. Run `pio run -t upload` — the log should show `waiting for download`

## Drivers - verified on Windows 11

**No driver install was needed.** Windows supplied it automatically; the adapter
enumerated on first plug-in:

```
COM3
Hardware ID: USB VID:PID=1A86:55D3 SER=5565052534 LOCATION=1-2.3
Description: USB-Enhanced-SERIAL CH343 (COM3)
```

`1A86` is WCH's vendor ID and `55D3` the CH343. If yours shows as an unknown
device instead, install WCH's `CH343SER` package. The listing advertises
WIN98/ME/2000/XP/Vista/WIN7/WIN8 **[listing]**, which is simply stale.

- **Linux:** supported by mainline kernels via the `ch341`/`ch343` driver.
- **macOS:** WCH publishes a signed driver; recent macOS versions may include it.

Drivers and documentation live in the vendor repository's `Drivers` and `Doc`
folders: [WeActStudio.USB2SerialV1](https://github.com/WeActStudio/WeActStudio.USB2SerialV1).

## Upload speed - measured

`upload_speed = 460800` works reliably. Seven consecutive flashes of a ~1.2 MB
image completed with `Hash of data verified` every time:

```
Wrote 1183056 bytes (760448 compressed) at 0x00010000 in 18.1 seconds (524.2 kbit/s)
```

Effective throughput 520-560 kbit/s, ~18 s of transfer plus ~13 s of PlatformIO
overhead, so about 31 s per flash cycle. The chip handles 6 Mbps, so there is
headroom to raise this if you flash often.

## Running it from the right shell

For ESP-IDF 5.5 environments, `pio` **must** be invoked from PowerShell or cmd,
never Git Bash. `idf_tools.py` rejects MSYS/MinGW outright:

```
ERROR: MSys/Mingw is not supported.
```

This applies to `-t upload` as well as builds. IDF 5.3 environments are
unaffected. See
[ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md)
for the full Windows setup.

```powershell
cd D:\s
pio run -e s3cam -t upload --upload-port COM3
```

## Diagnosing a failed connection

If esptool reports `Failed to connect to ESP32-S3: No serial data received`, work
outward from the adapter rather than guessing:

1. **Loopback the adapter.** Disconnect `TX`/`RX` from the board and jumper them
   to each other. Send bytes and check they return. This tests the chip, the
   driver, the COM port and - importantly - whether the pins you identified as
   `TX`/`RX` really are those pins, with the ESP32 removed from the picture.
2. **Check board power.** Measure `VCC` (~5 V) and `3V3` (~3.3 V) on the ESP32
   board against its `GND`.
3. **Listen for the boot ROM.** Open the port at 115200 and tap `RST`. Every
   reset prints an `ESP-ROM:esp32s3-...` banner regardless of the firmware
   flashed, so text appearing proves the board's TX reaches the adapter's RX.

Silence at step 3 with a passing step 1 points at power, ground, or a swapped
`TX`/`RX` pair.

## Open items

- [x] ~~Confirm `VIO` measures 3.3 V~~ - measured 3.3 V at `TX`, adapter safe for ESP32-S3
- [ ] Confirm the physical pin order on the board and record it here
- [ ] Decide whether to build the auto-reset circuit or keep manual boot entry
