# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository layout

Three loosely coupled sub-projects that share one protocol definition:

| Path               | Stack                                          | Purpose                                        |
| ------------------ | ---------------------------------------------- | ---------------------------------------------- |
| `esp32/`           | C++20, ESP-IDF (via PlatformIO), FreeRTOS      | Robot firmware                                 |
| `app/`             | SvelteKit 2 / Svelte 5, TS, Tailwind 4, Three  | Web controller, served *from* the firmware     |
| `simulation/`      | Python ≥3.13, MuJoCo, Gymnasium, SB3 PPO, `uv` | Residual-gait RL training for `spot_pico`      |
| `platform_shared/` | `.proto` + nanopb `.options`                   | Single source of truth for the wire protocol   |

`submodules/nanopb` is a git submodule — clone with `--recurse-submodules` or the firmware build will fail.

## Commands

### Firmware (repo root)

```bash
pio run                          # build default env (esp32-camera); also builds the web app
pio run -e esp32-wroom-camera    # other envs: seeed-xiao-esp32s3, esp32dev, esp32-p4
pio run -t upload
pio run -t uploadfs              # LittleFS data partition (esp32/data)
pio device monitor
```

There is no working firmware test suite: `esp32/test/gait_performance.cpp` is a stale Unity benchmark that still includes `<Arduino.h>` and `gait/state.h` from before the ESP-IDF migration, so `pio test` does not compile. Firmware changes are validated by `pio run` plus the simulation's Python port of the gait.

`extra_scripts` in [platformio.ini](platformio.ini) run on every build:

- `esp32/scripts/pre_build.py` → `esp32/scripts/compile_protos.py` regenerates nanopb C sources into `esp32/src/platform_shared/`.
- `esp32/scripts/build_app.py` → when `EMBED_WEBAPP=1`, runs `pnpm install && pnpm run build:embedded` in `app/`, then gzips every built asset into `esp32/include/WWWData.h` as a single PROGMEM blob. It only rebuilds when a file under `app/src` is newer than `WWWData.h`, so touch a source file to force a regeneration.

### Web controller (`app/`)

```bash
pnpm install
pnpm proto            # regenerate ts-proto sources (requires protoc on PATH)
pnpm dev              # dev server, proxies /api + /api/ws to http://spot-micro.local
pnpm build            # runs `pnpm proto` first
pnpm build:embedded   # what the firmware build calls (VITE_USE_HOST_NAME=true)
pnpm check            # svelte-check
pnpm lint             # prettier --check + eslint
pnpm format
pnpm test             # playwright integration + vitest unit
pnpm test:unit                                  # all unit tests (tests/unit/*.spec.ts)
pnpm vitest run tests/unit/socket.spec.ts       # single unit test file
pnpm test:integration --grep "<name>"           # single playwright test
```

Point the dev proxy at your device by editing `server.proxy` in [app/vite.config.ts](app/vite.config.ts); a restart is required.

### Simulation (`simulation/`)

```bash
uv sync
uv run pytest -q                                  # regression tests (tests/)
uv run python -m src.robot.firmware_gait          # IK/FK self-test
uv run python replay_gait.py --headless           # validate zero-residual gait
uv run python train_mj.py --smoke                 # pipeline sanity run
uv run python eval_policy.py runs/<run> --vx 0.05
```

See [simulation/README.md](simulation/README.md) for the full training pipeline. Note the top-level [readme.md](readme.md) still describes a PyBullet simulator; it was replaced by MuJoCo (commit `9ccb0ff`) and that section is stale.

## Architecture

### Protobuf is the contract, and generated code is not committed

`platform_shared/*.proto` compiles to three targets: nanopb C (`esp32/src/platform_shared/`), TypeScript (`app/src/lib/platform_shared/`), and hand-written NumPy in the simulation. Both generated directories are gitignored — they must be regenerated after any proto change, and after a fresh clone.

Adding a proto message is a multi-file change: the `.proto`, a matching `.options` file with nanopb `max_size` constraints (fixed-size buffers — there is no heap allocation), the two compile-script file lists, plus `DEFINE_MESSAGE_TRAITS` in [esp32/include/communication/proto_helpers.h](esp32/include/communication/proto_helpers.h) and `combinedReferences` in [app/src/lib/stores/socket.ts](app/src/lib/stores/socket.ts) if the message joins the `Message` oneof. [platform_shared/ADDING_PROTO_FILES.md](platform_shared/ADDING_PROTO_FILES.md) is the authoritative checklist — follow it.

The TS socket store derives its tag↔key maps at runtime from the proto file descriptor, so a message added to the `Message` oneof needs no manual dispatch table on the client.

### Firmware task model

[esp32/src/main.cpp](esp32/src/main.cpp) creates exactly two tasks:

- **Control task** — priority 5, pinned to core 1, fixed 10 ms period (100 Hz) via `vTaskDelayUntil`. Runs `peripherals.update() → motionService.update() → servoController.setAngles() → servoController.update()`. Keep this path allocation-free and fast; `WARN_IF_SLOW` logs overruns and `-Wstack-usage=4096` is enforced at compile time.
- **Service task** — priority 2, unpinned. WiFi/AP/mDNS, HTTP server, WebSocket, and `EXECUTE_EVERY_N_MS` telemetry emission (IMU + RSSI at 100 ms, analytics at 2 s), gated on `wsSocket.hasSubscribers(...)`.

All HTTP routes and WebSocket handlers are registered in `setupServer()` / `setupEventSocket()` in `main.cpp` — that file is the router.

### Communication patterns

- **HTTP** (`/api/...`, [docs/api.md](docs/api.md)) — protobuf bodies wrapped in `api_Request` / `api_Response` oneofs.
- **WebSocket** `/api/ws` ([docs/websocket.md](docs/websocket.md)) — binary `Message` oneof. Three patterns: client→server commands, server→client pub/sub broadcasts (subscription-gated), and correlation request/response (`CorrelationRequest`/`CorrelationResponse` keyed by `correlation_id`, dispatched through the `correlationHandlers` map in `main.cpp`).

### Stateful service pattern

Persisted, HTTP-exposed settings use the templates in `esp32/include/template/`: `StatefulService<T>` (mutex-guarded state + update/hook handler lists) composed with `StatefulProtoEndpoint<T, ProtoT>` (proto↔state reader/updater plus oneof extractor/assigner) and `stateful_persistence.h` for filesystem-backed persistence. Services such as `ServoController`, `WiFiService`, `APService`, `Peripherals`, and `MDNSService` each expose a public `protoEndpoint` that `main.cpp` binds to GET/POST routes. New settings should follow the same shape rather than hand-rolling handlers.

### Motion system

`MotionService` ([esp32/include/motion.h](esp32/include/motion.h)) is a state machine over `MotionState` subclasses in `esp32/include/motion_states/` (`rest`, `stand`, `walk`). `WalkState` implements both a Bezier trot and an 8-phase crawl selected by `WALK_GAIT`. `Kinematics` / `KinConfig` ([esp32/include/kinematics.h](esp32/include/kinematics.h)) holds all link lengths and motion limits as `constexpr`, selected at compile time by the variant flag. See [docs/motion_system.md](docs/motion_system.md) and [docs/kinematics.md](docs/kinematics.md).

**The kinematics and gait are implemented three times** and must be kept in agreement:

- C++ — `esp32/include/kinematics.h`, `esp32/include/motion_states/walk_state.h` (authoritative)
- TypeScript — [app/src/lib/kinematic.ts](app/src/lib/kinematic.ts), [app/src/lib/gait.ts](app/src/lib/gait.ts) (browser preview/visualization)
- Python — [simulation/src/robot/firmware_gait.py](simulation/src/robot/firmware_gait.py) (explicitly a NumPy port of `walk_state.h`; the RL policy only learns a ±15 mm residual on top of it, so a divergence here silently invalidates training)

Changing gait or IK in one place means porting the change to the other two.

### Compile-time configuration

Build flags come from three ini files merged by `extra_configs`:

- [esp32/features.ini](esp32/features.ini) — `USE_*` peripheral/feature toggles (queried via `FT_ENABLED(...)`; defaults live in [esp32/include/features.h](esp32/include/features.h)) **and** the robot variant: exactly one of `SPOTMICRO_ESP32`, `SPOTMICRO_ESP32_MINI`, `SPOTMICRO_YERTLE`. The variant flag drives `KinConfig` constants *and* which URDF/mesh assets `build_app.py` embeds.
- [esp32/factory_settings.ini](esp32/factory_settings.ini) — app name/version, factory WiFi/AP defaults, servo oscillator and PWM frequency, deep-sleep pin.
- [esp32/build_settings.ini](esp32/build_settings.ini) — `EMBED_WEBAPP` (set to `0` to skip the web build and iterate on firmware only), `APPLICATION_CORE`.

Board-specific pin assignments (`SDA_PIN`, `SCL_PIN`, `WS2812_PIN`, camera model, …) live per-env in `platformio.ini`, not in the ini files above.

## Git and GitHub

**Never commit or push without a direct instruction to do so.** Make the changes, leave them in the working tree, and say what is pending. Only an explicit "commit this" / "push" counts. None of the following are authorisation: the user approving the work itself, saying a change looks right or works, answering a clarifying question, asking you to write or update a file, or a plan of yours that mentioned committing and drew no objection. Authorisation is per-request and does not carry forward — a previous "commit and push" does not license the next commit.

**Always use SSH remotes, never HTTPS.** GitHub has disabled password authentication for Git operations, so an `https://github.com/...` remote fails with `Password authentication is not supported for Git operations` unless a token is configured. SSH keys are already set up here.

```bash
git remote set-url origin git@github.com:<owner>/<repo>.git   # not https://
ssh -T git@github.com                                          # verify identity
```

Remote layout:

| Remote     | Points at            | Use                                    |
| ---------- | -------------------- | -------------------------------------- |
| `origin`   | `tpetrytsyn` fork    | Push work here                          |
| `upstream` | `runeharlyk` original | `git fetch upstream` to pull changes in |

**Never commit or push credentials.** No tokens, private keys, `.env` files, or passwords — not in code, commit messages, or config. If a secret is needed, read it from the environment.

Repo-specific trap: [esp32/factory_settings.ini](esp32/factory_settings.ini) is **tracked**, and holds `FACTORY_WIFI_SSID` / `FACTORY_WIFI_PASSWORD`. They ship empty. Do not commit real network credentials there — set Wi-Fi at runtime through the web UI or the `/api/wifi/sta/settings` endpoint, which persists to LittleFS rather than to git. The same applies to `FACTORY_AP_PASSWORD` if you change it from the default.

Commit messages start with a gitmoji (`♻️`, `⚡`, `🎨`, `🐛`, `✨`, `📝`, …).

## Conventions

- Firmware: 4-space indent, ~120 col, `#pragma once` in newer headers (older ones use include guards), headers-only for templates and motion states. Peripherals are two-layered: raw chip drivers in `esp32/include/peripherals/drivers/` (`mpu6050.h`, `bno055.h`, `hmc5883l.h`, `pca9685.h`, …) wrapped by role-level classes (`imu.h`, `magnetometer.h`, `barometer.h`) that implement `SensorBase<T>` from `sensor.hpp` and are aggregated by `Peripherals`.
- Frontend: prettier config is unusual — 4-space tabs, single quotes, **no semicolons**, no trailing commas, `arrowParens: avoid`, `experimentalTernaries`. Always `pnpm format` rather than matching by hand.
