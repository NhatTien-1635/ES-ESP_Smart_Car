# Quickstart Validation Guide

## Prerequisites

- ESP32-S3 DevKitC-1, motor driver and sensors connected according to the approved pin map.
- PlatformIO CLI/IDE and a browser capable of landscape mode.
- A USB serial connection at 115200 baud.

## Build and baseline checks

From the repository root:

```sh
pio run
pio test
```

Expected result: firmware compiles for `esp32-s3-devkitc-1`; native/unit tests pass. Before hardware modules exist, `pio test` may contain only the existing test scaffold and must not be presented as feature acceptance.

## Filesystem and board smoke test

1. Upload firmware and filesystem (`pio run -t upload`, then the PlatformIO filesystem upload target).
2. Open the serial monitor at 115200.
3. Join Wi-Fi `ESP32_WebModule` with the configured password.
4. Browse to the device AP address.

Expected result: serial output confirms AP startup; `/` serves `index.html`; missing filesystem produces an observable error; UI shows connection state.

## Command validation

Use the command contract in [contracts/commands.md](./contracts/commands.md):

```sh
curl -i 'http://<device-ip>/api/cmd?d=S'
curl -i 'http://<device-ip>/api/cmd?d=F'
curl -i 'http://<device-ip>/api/cmd?d=X'
curl -i 'http://<device-ip>/api/linetrace?on=1'
```

Expected result: valid commands are acknowledged and reflected in state; invalid commands return 4xx and do not move the robot; line tracing disables manual motion and starts from a stopped state.

For versioned control validation, send each of the eight directions:
`FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `FORWARD_LEFT`, `FORWARD_RIGHT`,
`BACKWARD_LEFT`, and `BACKWARD_RIGHT`. Confirm that each command produces
the corresponding motion and that an unsupported diagonal is rejected while
the motor remains stopped.

Also test command replacement: hold `FORWARD_LEFT`, switch directly to
`BACKWARD_RIGHT`, then release. The first command must not remain active, the
replacement must be explicit, and release must end with `STOP`.

## Safety validation

1. Hold a movement command.
2. Release the control, switch tabs, background the page and press STOP.
3. While moving, disable the client/network and wait past the configured timeout.

Expected result: every path calls `stopAll()`, motor outputs are zero and state becomes `IDLE` with a visible safety reason.

## Telemetry validation

```sh
curl -s 'http://<device-ip>/api/telemetry'
```

Validate against [contracts/telemetry.md](./contracts/telemetry.md). Confirm schema version, state, sensor validity flags and line count. Inject a disconnected/invalid sensor and verify the response marks it invalid or stale rather than returning a fabricated zero.

## Optional WebSocket validation

Only after the WebSocket adapter is approved and enabled: connect one client, send a versioned `MOVE`, receive a telemetry snapshot, disconnect, and verify safe stop. Measure command-to-motor acknowledgement latency on the SoftAP; acceptance target is p95 below 100 ms.
