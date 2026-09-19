# API

The back end exposes a number of API endpoints which are referenced in the tables below.
All routes are registered in `setupServer()` in [main.cpp](../esp32/src/main.cpp) — that
function is the authoritative list.

## Request and response format

Every endpoint speaks Protocol Buffers, not JSON. Requests are wrapped in `api.Request`
and responses in `api.Response` ([api.proto](../platform_shared/api.proto)), with
`Content-Type: application/x-protobuf`.

`api.Request` is a bare `oneof`, so **a POST body must be non-empty and must decode as a
valid `api.Request`** — even for endpoints that ignore the payload entirely, such as
`/api/system/restart`. `receiveProto()` rejects a zero-length body with **400 Invalid
request payload** ([webserver.h](../esp32/include/communication/webserver.h)). The
smallest accepted body is a zero-length submessage, e.g. bytes `62 00` for an empty
`ap_status_request`.

An unregistered path returns **405 Method Not Allowed** rather than 404, because `/*` is
bound to `HTTP_OPTIONS` only. With `EMBED_WEBAPP=0` this includes the web UI itself —
`GET /` returns 405, since the static-asset and SPA-fallback handlers are compiled out.

## System

| Method | Endpoint            | Description                                  |
| ------ | ------------------- | -------------------------------------------- |
| POST   | /api/system/reset   | Reset the ESP32 and all settings to defaults |
| POST   | /api/system/restart | Restart the ESP32                            |
| POST   | /api/system/sleep   | Put the device in deep sleep mode            |

Feature flags and system status are **not** HTTP endpoints — they are WebSocket
correlation requests. See [WebSocket](#websocket).

## WiFi

| Method | Endpoint               | Description                           |
| ------ | ---------------------- | ------------------------------------- |
| GET    | /api/wifi/sta/settings | Get current WiFi settings             |
| POST   | /api/wifi/sta/settings | Update WiFi settings and credentials  |
| GET    | /api/wifi/scan         | Trigger async scan for networks       |
| GET    | /api/wifi/networks     | List networks in range after scanning |
| GET    | /api/wifi/sta/status   | Get WiFi client connection status     |

`/api/wifi/scan` returns **202** while a scan is in flight; poll `/api/wifi/networks`
until it returns **200** with a populated `wifi_network_list`. A scan takes roughly four
seconds, and the soft AP briefly stops responding while the radio leaves its channel.

`wifi_status.status` in `/api/wifi/sta/status` is an `wl_status_t`: **3** is connected,
**6** is disconnected. The web UI renders anything other than 3 as "Inactive".

## Access Point

| Method | Endpoint         | Description           |
| ------ | ---------------- | --------------------- |
| GET    | /api/ap/status   | Get current AP status |
| GET    | /api/ap/settings | Get AP settings       |
| POST   | /api/ap/settings | Update AP settings    |

## Camera (if `USE_CAMERA`)

| Method | Endpoint             | Description            |
| ------ | -------------------- | ---------------------- |
| GET    | /api/camera/still    | Capture a still image  |
| GET    | /api/camera/stream   | Get camera stream      |
| GET    | /api/camera/settings | Get camera settings    |
| POST   | /api/camera/settings | Update camera settings |

The two settings routes additionally require `USE_DVP_CAMERA`; they are absent on MIPI-CSI
targets. `still` and `stream` return `image/jpeg` straight from the framebuffer rather than
a protobuf response.

## Servo

| Method | Endpoint          | Description             |
| ------ | ----------------- | ----------------------- |
| GET    | /api/servo/config | Get servo configuration |
| POST   | /api/servo/config | Update servo config     |

## Peripherals

| Method | Endpoint                  | Description                |
| ------ | ------------------------- | -------------------------- |
| GET    | /api/peripherals/settings | Get peripheral settings    |
| POST   | /api/peripherals/settings | Update peripheral settings |

## mDNS (if `USE_MDNS`)

| Method | Endpoint           | Description          |
| ------ | ------------------ | -------------------- |
| GET    | /api/mdns/settings | Get mDNS settings    |
| POST   | /api/mdns/settings | Update mDNS settings |
| GET    | /api/mdns/status   | Get mDNS status      |
| POST   | /api/mdns/query    | Query mDNS services  |

## Filesystem

| Method | Endpoint          | Description      |
| ------ | ----------------- | ---------------- |
| GET    | /api/config/\*    | Get config file  |
| GET    | /api/files        | List files       |
| POST   | /api/files/delete | Delete file      |
| POST   | /api/files/edit   | Edit file        |
| POST   | /api/files/mkdir  | Create directory |

File **uploads** and **downloads** go over the WebSocket, not HTTP — see
`fs_upload_start` / `FSUploadData` and `fs_download_request` in
[websocket.md](websocket.md).

## WebSocket

Real-time communication is handled via WebSocket at `/api/ws` using Protocol Buffers.

Several operations that look like they should be REST endpoints are correlation requests
on this socket instead, dispatched through the `correlationHandlers` map in
[main.cpp](../esp32/src/main.cpp):

| Correlation request        | Purpose                                     |
| -------------------------- | ------------------------------------------- |
| `features_data_request`    | Enabled features for the UI                 |
| `system_information_request` | Analytics plus static system information  |
| `i2c_scan_data_request`    | Scan the I2C bus                            |
| `imu_calibrate_execute`    | Calibrate the IMU                           |
| `fs_list_request`          | List files                                  |
| `fs_delete_request`        | Delete a file                               |
| `fs_mkdir_request`         | Create a directory                          |
| `fs_upload_start`          | Begin a file upload                         |
| `fs_download_request`      | Begin a file download                       |
| `fs_cancel_transfer`       | Cancel an in-flight transfer                |

See [websocket.md](websocket.md) for the full WebSocket API documentation.
