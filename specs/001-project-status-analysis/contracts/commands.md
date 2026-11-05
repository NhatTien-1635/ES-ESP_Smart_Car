# Control Command Contract

## Current compatibility endpoint

`GET /api/cmd?d=<command>`

- Accepted compatibility values: `F`, `B`, `L`, `R`, `S`. Các hướng chéo dùng versioned command shape bên dưới để giữ ký hiệu tương thích cũ không mơ hồ.
- `S` is always safe and idempotent.
- Invalid/missing `d` must return `400` after integration; it must not return success-shaped `200 ok`.
- Valid commands return `200` with a small acknowledgement containing accepted command and current mode.

`GET /api/linetrace?on=0|1`

- `on=1` requests `LINE_TRACKING`; `on=0` requests `MANUAL` or stopped manual readiness.
- Invalid/missing value returns `400`.
- Enabling line tracking first stops manual motion.
- The endpoint is a compatibility adapter; new clients may use the versioned command body below.

## Versioned command shape

```json
{
  "schema_version": 1,
  "cmd": "MOVE",
  "dir": "FORWARD",
  "speed": 80
}
```

Allowed `cmd`: `MOVE`, `STOP`, `SET_MODE`. `dir` is one of `FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT`, `BACKWARD_RIGHT`; `mode` is one of `IDLE`, `MANUAL`, `LINE_TRACKING`; speed is integer 0–100.

Diagonal commands are independent values and must not be silently mapped to a cardinal direction. If the selected motor/chassis cannot support a diagonal command, return a clear unsupported-command error and keep the robot stopped.

The motor adapter, not the web handler, owns any chassis-specific wheel mixing required by diagonal motion.

## Safety contract

- Commands are rejected without changing state when validation fails.
- A moving command expires after the configured command timeout unless refreshed.
- STOP is processed ahead of queued movement and is safe to repeat.
- Client disconnect or stale session causes the robot to stop and enter `IDLE`.
- Releasing, cancelling or replacing a diagonal command stops the previously active motion before the replacement is applied.
