# CH343P USB-serial adapter (WeAct Studio USB2Serial V1)

Flashes and monitors the [ESP32-S3-CAM](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md), which has no USB.
Silkscreen `USB2UART`, `CH343`, `6Mbps Max.`, `WeAct Studio`, `V1.0`; chip WCH CH343P. Vendor repo:
[WeActStudio.USB2SerialV1](https://github.com/WeActStudio/WeActStudio.USB2SerialV1). Sourced from
[rcscomponents.kiev.ua](https://www.rcscomponents.kiev.ua/product/peretvoriuvach-ch343p-usb-serial-port-weact-studio_198651.html).

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| USB | Full-speed USB 2.0, **Type-C** | listing + photo |
| Baud rate | 50 bps – **6 Mbps** | photo + datasheet |
| Modem signals | RTS, DTR, DCD, RI, DSR, CTS; hardware flow control | listing |
| I/O level | **3.3 V / 5 V / custom 1.8–5 V**, selectable | photo + datasheet |
| Size | 35 × 13 mm; `READY` LED; one jumper cap | listing + photo |

The listing's "maximum speed 115200 bps" is the chip's auto-baud limit, not its maximum.

## Pins

`TX`, `RX`, `GND`, `5V` (USB VBUS, passed through), `3V3` (onboard regulator), `VIO`, `RTS`,
`DTR`. The photos don't settle the physical order — match by label, or buzz `GND` against the USB
shell.

## VIO: set it to 3.3 V first

`VIO` sets the level of `TX`, `RX`, `RTS` and `DTR`. The ESP32-S3 isn't 5 V tolerant, so at 5 V the
adapter would damage `IO44`.

- **Jumper cap** `VIO` ↔ `3V3` — or the solder jumper's `3V3 TTL` position.
- `CUSTOM` leaves `VIO` floating for an external reference; never leave it unconnected.
- **Measured:** 3.3 V at `TX` with the jumper on `3V3`.

## Wiring to the ESP32-S3-CAM

| Adapter | ESP32-S3-CAM |
| --- | --- |
| `TX` | `RX` (IO44) |
| `RX` | `TX` (IO43) |
| `GND` | `GND` |
| `5V` | `VCC` — standalone board only |

- **On the robot's harness, no `5V` wire:** rocker on, `GND`/`TX`/`RX` only. The adapter's 5 V
  would back-feed through the CN3903 with the rocker off, or fight it with the rocker on.
- Standalone, USB's 500 mA is fine for the bare board and marginal with the camera. Never power
  the board from the adapter's `3V3`.

## Boot mode

The board has no auto-reset circuit, so the bootloader is entered by hand — see
[the board doc](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md#programming). PlatformIO sets
`monitor_rts = 0` and `monitor_dtr = 0` so the lines don't interfere.

Since both `DTR` and `RTS` are broken out, the standard two-NPN auto-program circuit (`RTS` → `EN`,
`DTR` → `IO0`) would automate it:

| DTR | RTS | EN | IO0 |
| --- | --- | --- | --- |
| 1 | 1 | 1 | 1 |
| 0 | 0 | 1 | 1 |
| 1 | 0 | 0 | 1 |
| 0 | 1 | 1 | 0 |

## Drivers and speed

- **Windows 11:** no install needed — enumerated as `COM3`, `USB VID:PID=1A86:55D3`. Otherwise WCH's
  `CH343SER`. Linux: mainline `ch341`/`ch343`. macOS: WCH's signed driver.
- **`upload_speed = 460800`** is reliable: seven consecutive flashes of ~1.2 MB verified, ~31 s per
  cycle. The chip handles 6 Mbps if you want faster.
- IDF 5.5 envs must run `pio` from PowerShell — [why](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md#building-on-windows).

## Diagnosing "No serial data received"

1. **Loopback:** disconnect the board, jumper `TX` to `RX`, and check sent bytes come back.
2. **Power:** the board's `VCC` ~5 V, `3V3` ~3.3 V.
3. **Boot ROM:** open the port at 115200 and tap `RST` — every reset prints `ESP-ROM:esp32s3-…`,
   whatever the firmware.

Silence at step 3 with step 1 passing means power, ground, or swapped `TX`/`RX`.

## Open items

- [x] `VIO` measured 3.3 V at `TX`
- [ ] Record the physical pin order
- [ ] Build the auto-reset circuit, or keep manual boot entry
