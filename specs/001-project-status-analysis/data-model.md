# Data Model: Robot control and telemetry

## Contract invariants

- The current hardware configuration contains exactly five line sensors.
- Motion direction is an explicit enum with `STOP` plus eight directions; diagonal values are never downgraded to cardinal values.
- A telemetry snapshot is copied atomically from controller and sensor state before JSON serialization.
- Sensor validity distinguishes `VALID`, `MISSING`, `STALE` and `INVALID`; unavailable data is not represented as a fabricated zero.

## RobotState

Đại diện trạng thái vận hành duy nhất của robot.

| Field | Type | Rules |
|---|---|---|
| `mode` | enum | `IDLE`, `MANUAL`, `LINE_TRACKING`; chỉ chuyển qua command hợp lệ |
| `motion` | enum | `STOP`, `FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT`, `BACKWARD_RIGHT` |
| `speed` | integer | 0–100; bằng 0 khi `STOP` hoặc `IDLE` |
| `last_command_at` | monotonic timestamp | cập nhật với command hợp lệ |
| `safety_reason` | enum/string | `NONE`, `TIMEOUT`, `DISCONNECTED`, `INVALID_SENSOR`, `OBSTACLE` |

## ControlCommand

Đại diện yêu cầu từ UI hoặc WebSocket.

| Field | Type | Rules |
|---|---|---|
| `kind` | enum | `MOVE`, `STOP`, `SET_MODE` |
| `direction` | enum, optional | bắt buộc với `MOVE`; một trong 8 hướng `FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT`, `BACKWARD_RIGHT` |
| `speed` | integer, optional | 0–100; mặc định không được ngầm coi là tốc độ an toàn nếu thiếu |
| `mode` | enum, optional | bắt buộc với `SET_MODE` |
| `received_at` | monotonic timestamp | do firmware gán khi nhận |

Invalid command không được cập nhật state hoặc motor và phải trả lỗi cho caller.

## SensorReading

Snapshot đọc từ phần cứng.

| Field | Type | Rules |
|---|---|---|
| `line` | array of boolean/enum | exactly 5 elements; invalid sample has validity/error metadata |
| `distance_cm` | number/null | non-negative when valid; unavailable is represented by validity/error metadata |
| `temperature_c` | number/null | valid only when the DHT sample succeeds |
| `humidity_percent` | number/null | 0–100 when valid |
| `battery_v` | number/null | non-negative; calibration metadata is required before treating it as accurate |
| `sampled_at` | timestamp | thời điểm snapshot |
| `validity` | per-sensor enum | each sensor is `VALID`, `MISSING`, `STALE` or `INVALID` |
| `errors` | per-sensor code, optional | explains missing, stale or invalid readings without fabricating a value |

## TelemetrySnapshot

Payload đọc bởi UI.

- `schema_version`: số nguyên dương.
- `device_id`: định danh robot.
- `state`: `RobotState` rút gọn.
- `sensors`: `SensorReading`.
- `connection.status`: `CONNECTED`, `DISCONNECTED` hoặc `STALE`.
- `connection.last_client_at`: monotonic timestamp hoặc `null`.

Snapshot phải tự nhất quán; không trộn state từ thời điểm này với sensor reading quá cũ mà không đánh dấu `STALE`. Không serialize trong vùng lock bảo vệ state.

## State transitions

| Current | Event | Next | Side effect |
|---|---|---|---|
| Any | `STOP` | `IDLE` or current mode with stopped motion | `stopAll()` |
| `IDLE` | valid `MOVE` | `MANUAL` | start requested motion |
| `MANUAL` | valid `MOVE` | `MANUAL` | refresh timeout and motion |
| `MANUAL` | `SET_MODE(LINE_TRACKING)` | `LINE_TRACKING` | stop manual motion |
| `LINE_TRACKING` | `SET_MODE(MANUAL)` | `MANUAL` | remain stopped until next MOVE |
| Any moving mode | command timeout | `IDLE` | `stopAll()`, reason `TIMEOUT` |
| Any moving mode | disconnect | `IDLE` | `stopAll()`, reason `DISCONNECTED` |
| `LINE_TRACKING` | obstacle/invalid sensor | `IDLE` | `stopAll()`, expose reason |

## Direction semantics

- `FORWARD_LEFT` và `FORWARD_RIGHT` phối hợp thành phần tiến với thành phần rẽ tương ứng.
- `BACKWARD_LEFT` và `BACKWARD_RIGHT` phối hợp thành phần lùi với thành phần rẽ tương ứng.
- Mỗi hướng chéo là một giá trị command độc lập; không được tự động hạ xuống hướng thẳng gần nhất.
- Nếu driver/chassis chưa hỗ trợ phối hợp hướng chéo, controller phải từ chối command với lỗi rõ ràng và giữ `motion=STOP`.

## Telemetry sampling and safety invariants

- Environment and battery samples refresh every 2–5 seconds.
- Motion and safety state update after every accepted command and safety tick.
- Repeated telemetry requests do not trigger a new hardware sensor read.
- When a client becomes stale or disconnected, the robot stops and telemetry must not claim that motion continues.
