# Hợp đồng telemetry

## Endpoint

`GET /api/telemetry`

Response là JSON snapshot nhất quán của robot và cảm biến. Endpoint chỉ đọc snapshot đã được task cảm biến/controller cập nhật; không được đọc phần cứng mới trong mỗi HTTP request.

## Schema phiên bản 1

```json
{
  "schema_version": 1,
  "device_id": "robot_01",
  "state": {
    "mode": "MANUAL",
    "motion": "STOP",
    "speed": 0,
    "last_command_at": 123456,
    "safety_reason": "NONE"
  },
  "sensors": {
    "temperature_c": 28.5,
    "humidity_percent": 65.0,
    "battery_v": 7.4,
    "distance_cm": 42.0,
    "line": [0, 0, 1, 0, 0],
    "sampled_at": 123400,
    "validity": {
      "temperature": "VALID",
      "humidity": "VALID",
      "battery": "VALID",
      "distance": "VALID",
      "line": "VALID"
    },
    "errors": {}
  },
  "connection": {
    "status": "CONNECTED",
    "last_client_at": 123456
  }
}
```

## Trường trạng thái

| Trường | Kiểu | Ràng buộc |
|---|---|---|
| `schema_version` | integer | Số nguyên dương; phiên bản hiện tại là `1` |
| `device_id` | string | Định danh ổn định của robot |
| `state.mode` | enum | `IDLE`, `MANUAL`, `LINE_TRACKING` |
| `state.motion` | enum | `STOP`, `FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT`, `BACKWARD_RIGHT` |
| `state.speed` | integer | 0–100; bắt buộc bằng 0 khi `motion=STOP` hoặc `mode=IDLE` |
| `state.last_command_at` | monotonic timestamp | Thời điểm firmware nhận command hợp lệ gần nhất |
| `state.safety_reason` | enum | `NONE`, `TIMEOUT`, `DISCONNECTED`, `INVALID_SENSOR`, `OBSTACLE` |

`motion` phải giữ đúng identity của cả bốn hướng chéo. Không được thay `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT` hoặc `BACKWARD_RIGHT` thành hướng cardinal gần nhất.

## Trường cảm biến

| Trường | Kiểu | Ràng buộc |
|---|---|---|
| `sensors.line` | array | Đúng 5 phần tử theo cấu hình hardware hiện tại |
| `sensors.distance_cm` | number/null | Không âm khi hợp lệ; không có dữ liệu phải dùng `null` hoặc validity khác `VALID` |
| `sensors.temperature_c` | number/null | Chỉ hợp lệ khi DHT11 đọc thành công |
| `sensors.humidity_percent` | number/null | 0–100 khi hợp lệ |
| `sensors.battery_v` | number/null | Không âm; chỉ coi là chính xác sau khi có calibration metadata |
| `sensors.sampled_at` | monotonic timestamp | Thời điểm lấy mẫu của snapshot cảm biến |
| `sensors.validity` | object | Có một trạng thái cho từng nhóm cảm biến |
| `sensors.errors` | object | Mã lỗi tùy chọn theo nhóm cảm biến |

Mỗi validity phải là một trong:

```text
VALID
MISSING
STALE
INVALID
```

- `VALID`: phép đo mới và nằm trong miền hợp lệ.
- `MISSING`: chưa từng có mẫu hợp lệ hoặc phần cứng không trả dữ liệu.
- `STALE`: có mẫu trước đó nhưng đã quá thời hạn.
- `INVALID`: có mẫu nhưng sai miền/không thể tin cậy.

Khi validity khác `VALID`, giá trị số không được dùng `0` giả tạo để biểu thị phép đo. Có thể dùng `null` hoặc giữ giá trị cuối cùng kèm validity/error rõ ràng.

## Trạng thái kết nối

| Trường | Kiểu | Ràng buộc |
|---|---|---|
| `connection.status` | enum | `CONNECTED`, `DISCONNECTED`, `STALE` |
| `connection.last_client_at` | monotonic timestamp/null | Thời điểm heartbeat/request hợp lệ gần nhất |

Khi client stale/disconnected, safety state phải phản ánh việc dừng theo contract command; telemetry không được báo robot đang tiếp tục chuyển động nếu motor đã bị dừng.

## Quy tắc snapshot và lấy mẫu

- `TelemetrySnapshot` phải tự nhất quán: state và sensor data được lấy từ một lần copy có đồng bộ.
- Không serialize JSON trong critical section/mutex bảo vệ state.
- Cảm biến môi trường và pin được làm mới mỗi 2–5 giây.
- Trạng thái chuyển động/safety được cập nhật sau mỗi command hợp lệ và mỗi safety tick.
- UI có thể polling nhanh hơn nhưng request lặp lại không được làm phát sinh lần đọc cảm biến mới.
- Dữ liệu hard-code hiện tại chỉ là placeholder và không được trả như phép đo thật sau khi contract này được tích hợp.

## Lỗi endpoint

- Snapshot hợp lệ trả HTTP `200` và `schema_version`.
- Lỗi hệ thống nghiêm trọng khi không thể tạo snapshot phải trả mã HTTP khác `200` cùng mã lỗi rõ ràng.
- Không được dùng payload telemetry thành công để che giấu lỗi filesystem, sensor hoặc trạng thái kết nối.
