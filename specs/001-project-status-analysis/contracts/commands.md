# Hợp đồng lệnh điều khiển

## Phạm vi và nguyên tắc

- Hợp đồng này là nguồn sự thật chung cho UI, web adapter, controller và motor adapter.
- Lệnh hợp lệ phải được xác nhận rõ; lệnh sai không được thay đổi trạng thái hoặc đầu ra động cơ.
- Bốn hướng chéo là các giá trị domain độc lập. Không được suy ra từ hai request cardinal, ánh xạ thành hướng cardinal gần nhất hoặc gửi tuần tự hai lệnh cardinal.
- `STOP` luôn an toàn, có tính idempotent và được xử lý trước lệnh chuyển động đang chờ.
- Hành vi hỗ trợ hướng chéo phụ thuộc motor adapter/chassis. Khi không hỗ trợ, phải trả lỗi rõ ràng và giữ robot ở `STOP`.

## Endpoint tương thích hiện tại

### `GET /api/cmd?d=<command>`

Endpoint này chỉ nhận các ký hiệu cardinal cũ:

| Giá trị `d` | Lệnh domain |
|---|---|
| `F` | `MOVE` với `FORWARD` |
| `B` | `MOVE` với `BACKWARD` |
| `L` | `MOVE` với `LEFT` |
| `R` | `MOVE` với `RIGHT` |
| `S` | `STOP` |

Quy tắc:

- `d` phải là một ký tự ASCII viết hoa thuộc `F|B|L|R|S`.
- Thiếu `d`, nhiều hơn một ký tự hoặc giá trị khác danh sách trên phải trả HTTP `400`.
- Endpoint tương thích không dùng để biểu diễn hướng chéo; client phải dùng command có phiên bản bên dưới.
- Lệnh hợp lệ phải trả HTTP `200` và acknowledgement chứa lệnh đã chấp nhận cùng mode hiện tại.
- `S` phải có thể gọi lặp lại mà không tạo lỗi hoặc chuyển động.

Ví dụ:

```text
GET /api/cmd?d=F
```

```json
{
  "schema_version": 1,
  "accepted": true,
  "cmd": "MOVE",
  "dir": "FORWARD",
  "mode": "MANUAL"
}
```

```text
GET /api/cmd?d=X
```

```json
{
  "schema_version": 1,
  "accepted": false,
  "error": "INVALID_COMMAND"
}
```

### `GET /api/linetrace?on=0|1`

| Giá trị `on` | Hành vi |
|---|---|
| `1` | Dừng chuyển động thủ công trước, sau đó yêu cầu `LINE_TRACKING` |
| `0` | Yêu cầu `MANUAL` hoặc trạng thái sẵn sàng thủ công nhưng vẫn dừng |

Quy tắc:

- Thiếu `on` hoặc giá trị khác `0|1` phải trả HTTP `400`.
- Khi bật line tracing, lệnh manual đang hoạt động phải đi qua đường `STOP` trước khi đổi mode.
- Endpoint này là adapter tương thích; client mới có thể dùng command có phiên bản.

## Command có phiên bản

### Schema request

```json
{
  "schema_version": 1,
  "cmd": "MOVE",
  "dir": "FORWARD_LEFT",
  "speed": 80
}
```

Các trường:

| Trường | Kiểu | Bắt buộc | Ràng buộc |
|---|---|---|---|
| `schema_version` | integer | Có | Phải là `1` |
| `cmd` | string | Có | `MOVE`, `STOP` hoặc `SET_MODE` |
| `dir` | string | Với `MOVE` | Một trong đúng tám hướng bên dưới |
| `speed` | integer | Với `MOVE` | 0–100; không tự mặc định khi thiếu |
| `mode` | string | Với `SET_MODE` | `IDLE`, `MANUAL` hoặc `LINE_TRACKING` |

Tám giá trị hợp lệ của `dir`:

```text
FORWARD
BACKWARD
LEFT
RIGHT
FORWARD_LEFT
FORWARD_RIGHT
BACKWARD_LEFT
BACKWARD_RIGHT
```

Ý nghĩa hướng chéo:

- `FORWARD_LEFT`: phối hợp thành phần tiến và rẽ trái.
- `FORWARD_RIGHT`: phối hợp thành phần tiến và rẽ phải.
- `BACKWARD_LEFT`: phối hợp thành phần lùi và rẽ trái.
- `BACKWARD_RIGHT`: phối hợp thành phần lùi và rẽ phải.

### Validation và response

- JSON sai cú pháp, thiếu `schema_version`, sai kiểu dữ liệu, command không biết, hướng không biết, mode không biết hoặc speed ngoài 0–100 phải trả HTTP `400` với mã lỗi ổn định.
- Lệnh không hợp lệ không được cập nhật `RobotState`, `last_command_at` hoặc motor output.
- Nếu request hợp lệ về schema nhưng motor adapter không hỗ trợ hướng chéo, phải trả HTTP `409`, mã lỗi `UNSUPPORTED_DIRECTION`, gọi `stopAll()` và giữ `motion=STOP`.
- Không được trả HTTP `200` cho lỗi validation, lỗi không hỗ trợ hoặc lỗi phần cứng.
- Response thành công phải chứa `schema_version`, `accepted: true`, command đã chấp nhận và trạng thái/mode hiện tại.

Ví dụ response lỗi:

```json
{
  "schema_version": 1,
  "accepted": false,
  "error": "UNSUPPORTED_DIRECTION",
  "dir": "FORWARD_LEFT",
  "motion": "STOP"
}
```

## Safety contract

- Lệnh bị từ chối không được thay đổi state.
- Lệnh chuyển động hết hạn sau command timeout đã cấu hình nếu không được refresh.
- `STOP` được xử lý trước lệnh chuyển động đang chờ và có thể lặp lại an toàn.
- Client disconnect hoặc session stale khiến robot dừng và chuyển sang `IDLE`.
- Nhả, hủy hoặc thay thế một lệnh chéo phải dừng chuyển động trước đó trước khi áp dụng lệnh mới.
- Chuyển từ cardinal sang diagonal hoặc giữa hai diagonal phải giữ đúng identity của lệnh mới; không phát sinh lệnh cardinal trung gian.

## Quyết định phân lớp

- Web handler chỉ parse, validate schema và chuyển `ControlCommand` cho controller.
- Controller sở hữu state transition, timeout và safety stop.
- Motor adapter sở hữu việc phối hợp thành phần thành PWM theo chassis.
- UI không được truy cập GPIO và không được tự biến một hướng chéo thành hai request cardinal.
