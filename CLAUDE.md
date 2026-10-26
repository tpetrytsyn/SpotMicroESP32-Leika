# CLAUDE.md

Guidance for Claude Code in this repository.

## Layout

| Path | Stack | Purpose |
| --- | --- | --- |
| `esp32/` | C++20, ESP-IDF via PlatformIO, FreeRTOS | Robot firmware |
| `app/` | SvelteKit 2 / Svelte 5, TS, Tailwind 4, Three | Web controller, served *from* the firmware |
| `simulation/` | Python ≥ 3.13, MuJoCo, Gymnasium, SB3 PPO, `uv` | Residual-gait RL training for `spot_pico` |
| `platform_shared/` | `.proto` + nanopb `.options` | The wire protocol — single source of truth |
| `docs/` | Markdown | Upstream guides, plus this fork's hardware docs: [wiring.md](docs/wiring.md), [pre-calibration-build.md](docs/pre-calibration-build.md), [servo-calibration.md](docs/servo-calibration.md), [components/](docs/components/) |

`submodules/nanopb` is a submodule — clone with `--recurse-submodules`, or the firmware build fails.

## Commands

### Firmware (repo root)

```bash
pio run                          # default env esp32-camera; also builds the web app
pio run -e <env> -t upload       # envs: esp32-wroom-camera, s3cam, seeed-xiao-esp32s3, esp32dev, esp32-p4
pio run -t uploadfs              # LittleFS partition (esp32/data)
pio device monitor
```

This fork's board is **`s3cam`** (ESP32-S3-CAM, ESP-IDF 5.5): build and upload from **PowerShell in
`D:\s`**, never Git Bash, and flash manually over UART — see
[build-and-upload.md](docs/build-and-upload.md).

No working firmware tests: `esp32/test/gait_performance.cpp` predates the ESP-IDF migration and
doesn't compile. Validate with `pio run` plus the simulation's Python port of the gait.

`extra_scripts` in [platformio.ini](platformio.ini) run on every build:

- `pre_build.py` → `compile_protos.py` regenerates nanopb sources into `esp32/src/platform_shared/`.
- `build_app.py`, when `EMBED_WEBAPP=1`, runs `pnpm install && pnpm run build:embedded` in `app/`
  and gzips the output into `esp32/include/WWWData.h` — only if a file under `app/src` is newer, so
  touch one to force it.

### Web controller (`app/`)

```bash
pnpm install
pnpm proto            # regenerate ts-proto sources (needs protoc on PATH)
pnpm dev              # proxies /api and /api/ws to http://spot-micro.local (server.proxy in vite.config.ts; restart after editing)
pnpm build            # runs pnpm proto first
pnpm build:embedded   # what the firmware build calls
pnpm check            # svelte-check
pnpm lint             # prettier --check + eslint; pnpm format to fix
pnpm test             # playwright + vitest; pnpm test:unit for unit tests only
pnpm vitest run tests/unit/socket.spec.ts
pnpm test:integration --grep "<name>"
```

### Simulation (`simulation/`)

```bash
uv sync
uv run pytest -q
uv run python -m src.robot.firmware_gait          # IK/FK self-test
uv run python replay_gait.py --headless           # zero-residual gait
uv run python train_mj.py --smoke
uv run python eval_policy.py runs/<run> --vx 0.05
```

Pipeline: [simulation/README.md](simulation/README.md). The top-level [readme.md](readme.md) still
describes a PyBullet simulator, replaced by MuJoCo in `9ccb0ff`.

## Architecture

**Protobuf is the contract; generated code isn't committed.** `platform_shared/*.proto` compiles to
nanopb C (`esp32/src/platform_shared/`), TypeScript (`app/src/lib/platform_shared/`), and a
hand-written NumPy port in the simulation. Both generated directories are gitignored — regenerate
after a proto change or a fresh clone. To add a message, follow
[ADDING_PROTO_FILES.md](platform_shared/ADDING_PROTO_FILES.md): the `.proto`, an `.options` file
with nanopb `max_size` (fixed buffers, no heap), both compile-script lists, `DEFINE_MESSAGE_TRAITS`
in [proto_helpers.h](esp32/include/communication/proto_helpers.h), and `combinedReferences` in
[socket.ts](app/src/lib/stores/socket.ts) if it joins the `Message` oneof. The TS socket store
derives its tag↔key maps from the descriptor at runtime.

**Two firmware tasks**, both created in [main.cpp](esp32/src/main.cpp):

- **Control** — priority 5, core 1, 10 ms (100 Hz) via `vTaskDelayUntil`:
  `peripherals.update() → motionService.update() → servoController.setAngles() → servoController.update()`.
  Keep it allocation-free; `WARN_IF_SLOW` logs overruns, and `-Wstack-usage=4096` is enforced.
- **Service** — priority 2, unpinned: Wi-Fi/AP/mDNS, HTTP, WebSocket, and `EXECUTE_EVERY_N_MS`
  telemetry (IMU + RSSI 100 ms, analytics 2 s) gated on `wsSocket.hasSubscribers(...)`.

`main.cpp` is the router: HTTP routes in `setupServer()`, WebSocket handlers in `setupEventSocket()`.

**Communication:** HTTP `/api/...` with protobuf bodies in `api_Request`/`api_Response` oneofs
([api.md](docs/api.md)); WebSocket `/api/ws` with a binary `Message` oneof
([websocket.md](docs/websocket.md)) — client commands, subscription-gated broadcasts, and
`CorrelationRequest`/`CorrelationResponse` keyed by `correlation_id`, dispatched through
`correlationHandlers` in `main.cpp`.

**Stateful services:** persisted, HTTP-exposed settings use `esp32/include/template/` —
`StatefulService<T>` (mutex-guarded state, update/hook handlers) with
`StatefulProtoEndpoint<T, ProtoT>` and `stateful_persistence.h` (LittleFS). `ServoController`,
`WiFiService`, `APService`, `Peripherals` and `MDNSService` each expose a `protoEndpoint` bound in
`main.cpp`; new settings follow the same shape. Once a settings file exists, it overrides the
compile-time defaults.

**Motion:** `MotionService` ([motion.h](esp32/include/motion.h)) is a state machine over
`motion_states/` (`rest`, `stand`, `walk`); `WalkState` has a Bezier trot and an 8-phase crawl
(`WALK_GAIT`). `Kinematics`/`KinConfig` ([kinematics.h](esp32/include/kinematics.h)) hold link
lengths and limits as per-variant `constexpr`. See [motion_system.md](docs/motion_system.md) and
[kinematics.md](docs/kinematics.md).

**Kinematics and gait exist three times and must agree** — change one, port to the other two:

- C++ — `kinematics.h`, `motion_states/walk_state.h` (authoritative)
- TypeScript — [kinematic.ts](app/src/lib/kinematic.ts), [gait.ts](app/src/lib/gait.ts) (browser preview)
- Python — [firmware_gait.py](simulation/src/robot/firmware_gait.py), a NumPy port of
  `walk_state.h`. The RL policy learns only a ±15 mm residual on top, so a divergence silently
  invalidates training.

**Compile-time config** merges three ini files via `extra_configs`:

- [features.ini](esp32/features.ini) — `USE_*` toggles (`FT_ENABLED(...)`, defaults in
  [features.h](esp32/include/features.h)) and exactly one variant: `SPOTMICRO_ESP32`,
  `SPOTMICRO_ESP32_MINI` or `SPOTMICRO_YERTLE`. The variant drives `KinConfig` and the URDF/meshes
  `build_app.py` embeds.
- [factory_settings.ini](esp32/factory_settings.ini) — app name/version, factory Wi-Fi/AP, servo
  oscillator and PWM frequency, deep-sleep pin.
- [build_settings.ini](esp32/build_settings.ini) — `EMBED_WEBAPP` (`0` skips the web build),
  `APPLICATION_CORE`.

Board pins (`SDA_PIN`, `SCL_PIN`, `WS2812_PIN`, camera model) are per-env in `platformio.ini`.

## Git and GitHub

- **Never commit or push without a direct instruction.** Only an explicit "commit this" / "push"
  counts, and only for that request. Approving the work, saying it looks right, answering a
  question, asking for a file change, or a plan that mentioned committing are not authorisation.
  Leave changes in the working tree and say what's pending.
- **SSH remotes only, never HTTPS** — GitHub rejects password auth, and SSH keys are set up.
  `origin` is the `tpetrytsyn` fork (push here); `upstream` is `runeharlyk` (`git fetch upstream`).
  ```bash
  git remote set-url origin git@github.com:<owner>/<repo>.git
  ssh -T git@github.com
  ```
- **Never commit credentials** — tokens, keys, `.env` files, passwords — in code, messages or
  config; read secrets from the environment. Trap: [factory_settings.ini](esp32/factory_settings.ini)
  is tracked and holds `FACTORY_WIFI_SSID`/`FACTORY_WIFI_PASSWORD`, which ship empty — set Wi-Fi at
  runtime (UI or `/api/wifi/sta/settings`, persisted to LittleFS), never there. The same goes for
  `FACTORY_AP_PASSWORD` if changed from the default.
- Commit messages start with a gitmoji (`♻️`, `⚡`, `🎨`, `🐛`, `✨`, `📝`, …).

## Conventions

- **Firmware:** 4-space indent, ~120 columns, `#pragma once` in newer headers; templates and motion
  states are header-only. Peripherals are two-layered: raw drivers in
  `esp32/include/peripherals/drivers/` (`mpu6050.h`, `bno055.h`, `pca9685.h`, …) wrapped by role
  classes (`imu.h`, `magnetometer.h`, `barometer.h`) implementing `SensorBase<T>` (`sensor.hpp`),
  aggregated by `Peripherals`.
- **Frontend:** the prettier config is unusual — 4-space tabs, single quotes, **no semicolons**, no
  trailing commas, `arrowParens: avoid`, `experimentalTernaries`. Run `pnpm format`; don't match it
  by hand.
