# Data Model: Robot control and telemetry

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
| `line` | array of boolean/enum | cardinality phải khớp cấu hình; invalid sample có validity flag |
| `distance_cm` | number | non-negative; unavailable được biểu diễn bằng validity/error |
| `temperature_c` | number | chỉ hợp lệ khi DHT sample thành công |
| `humidity_percent` | number | 0–100 khi hợp lệ |
| `battery_v` | number | non-negative; cần calibration metadata trước khi coi là chính xác |
| `sampled_at` | timestamp | thời điểm snapshot |
| `validity` | flags | phân biệt missing, stale và invalid |

## TelemetrySnapshot

Payload đọc bởi UI.

- `schema_version`: số nguyên dương.
- `device_id`: định danh robot.
- `state`: `RobotState` rút gọn.
- `sensors`: `SensorReading`.
- `connection`: firmware-side session/safety status.

Snapshot phải tự nhất quán; không trộn state từ thời điểm này với sensor reading quá cũ mà không đánh dấu stale.

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
