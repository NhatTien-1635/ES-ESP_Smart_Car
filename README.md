# ESP32-S3 Smart Car

Firmware PlatformIO cho xe robot 4 bánh omni/mecanum, giao diện điều khiển web cục bộ và telemetry cảm biến.

## Phần cứng mục tiêu

- **Board**: ESP32-S3 DevKitC-1.
- **Framework**: Arduino trên PlatformIO.
- **Motor driver**: hai module L298N, mỗi module điều khiển hai động cơ DC.
- **Chassis**: bốn bánh omni/mecanum bố trí hình chữ X theo cấu hình đã chọn.
- **Cảm biến**: năm cảm biến dò line, HC-SR04, DHT11 và mạch đo điện áp pin.
- **Mạng**: Wi-Fi SoftAP cục bộ; không cần Internet.
- **Filesystem**: LittleFS lưu giao diện web tĩnh.

Sơ đồ GPIO/PWM và các điều kiện điện áp nằm trong [docs/hardware-pin-map.md](docs/hardware-pin-map.md). Đây là sơ đồ đề xuất; phải kiểm tra board thực tế, mức điện áp, mass chung và chiều quay động cơ trước khi chạy tải.

## Cấu trúc dự án

```text
src/                  Firmware entry point và web task hiện có
include/              Header dùng chung
data/                 Tài sản web được nạp vào LittleFS
test/                 Native/unit test và test contract
docs/                 Tài liệu phần cứng, kiểm thử và vận hành
specs/                Requirement, plan, contract và task list
platformio.ini        Cấu hình build ESP32-S3 và test native
```

## Chuẩn bị môi trường

1. Cài PlatformIO Core/PlatformIO IDE.
2. Kết nối ESP32-S3 DevKitC-1 qua USB.
3. Kiểm tra sơ đồ nối dây trong [docs/hardware-pin-map.md](docs/hardware-pin-map.md).
4. Không nối trực tiếp tín hiệu `HC-SR04 ECHO` 5 V vào ESP32-S3; phải dùng mạch hạ mức.
5. Không cấp nguồn động cơ từ chân 3.3 V của ESP32-S3.

## Build và kiểm thử

Từ thư mục gốc repository:

```sh
pio run -e esp32-s3-devkitc-1
pio test -e native
```

Môi trường `native` hiện đã được cấu hình để không biên dịch source phụ thuộc ESP32. Khi chưa có test suite trong `test/`, `pio test -e native` sẽ báo chưa có test để build; đây chưa phải kết quả nghiệm thu tính năng.

## Nạp firmware và giao diện web

```sh
pio run -e esp32-s3-devkitc-1 -t upload
pio run -e esp32-s3-devkitc-1 -t uploadfs
pio device monitor -b 115200
```

Nếu PlatformIO không tự chọn đúng cổng USB, thêm `--upload-port <port>` hoặc cấu hình `upload_port` trong môi trường local, không commit cấu hình máy cá nhân vào repository.

## Kết nối AP và truy cập giao diện

Firmware tạo SoftAP với thông tin hiện tại:

- **SSID**: `ESP32_WebModule`
- **Mật khẩu**: `88888888`
- **Serial**: `115200 baud`
- **Giao diện**: mở địa chỉ IP được in trong Serial Monitor.

Kết quả smoke test tối thiểu:

1. Serial báo khởi động AP.
2. Thiết bị điều khiển kết nối được vào SSID.
3. `/` phục vụ `index.html` từ LittleFS.
4. Thiếu filesystem hoặc `index.html` phải tạo lỗi quan sát được, không báo hệ thống sẵn sàng giả.

## Điều khiển 8 hướng

Hợp đồng đầy đủ nằm trong [specs/001-project-status-analysis/contracts/commands.md](specs/001-project-status-analysis/contracts/commands.md).

Các hướng domain:

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

Endpoint tương thích hiện tại chỉ dùng:

| Ký hiệu | Hướng |
|---|---|
| `F` | `FORWARD` |
| `B` | `BACKWARD` |
| `L` | `LEFT` |
| `R` | `RIGHT` |
| `S` | `STOP` |

Ví dụ kiểm tra endpoint:

```sh
curl -i 'http://<device-ip>/api/cmd?d=S'
curl -i 'http://<device-ip>/api/cmd?d=F'
curl -i 'http://<device-ip>/api/cmd?d=X'
curl -i 'http://<device-ip>/api/linetrace?on=1'
curl -s 'http://<device-ip>/api/telemetry'
```

Lệnh sai phải trả lỗi `4xx` và không làm xe chuyển động. Hướng chéo là command độc lập; không được tự chuyển thành hướng thẳng. Nếu motor adapter không hỗ trợ hướng chéo, hệ thống phải trả lỗi rõ ràng và giữ motor ở `STOP`.

## Kiểm tra an toàn

- Nhả nút, hủy pointer hoặc nhấn `STOP` phải dừng chuyển động.
- Chuyển tab, đưa trang ra nền hoặc mất focus phải đi qua đường `STOP`.
- Bật line tracing phải dừng chuyển động manual trước khi đổi mode.
- Mất client/network quá command timeout phải đưa robot về `IDLE` và tắt toàn bộ PWM.
- Khi thay nhanh một lệnh bằng lệnh khác, lệnh cũ không được tiếp tục.

## Telemetry

Hợp đồng telemetry nằm trong [specs/001-project-status-analysis/contracts/telemetry.md](specs/001-project-status-analysis/contracts/telemetry.md).

Telemetry phải có `schema_version`, trạng thái robot, đúng năm phần tử line và validity của từng nhóm cảm biến. Các trạng thái cảm biến gồm `VALID`, `MISSING`, `STALE` và `INVALID`; dữ liệu không hợp lệ không được giả tạo bằng số 0.

## Tài liệu phát triển

- [Đặc tả](specs/001-project-status-analysis/spec.md)
- [Kế hoạch triển khai](specs/001-project-status-analysis/plan.md)
- [Danh sách task](specs/001-project-status-analysis/tasks.md)
- [Sơ đồ GPIO/PWM](docs/hardware-pin-map.md)
- [Hướng dẫn kiểm thử nhanh](specs/001-project-status-analysis/quickstart.md)

WebSocket, backend/cloud và lưu trữ telemetry dài hạn chưa thuộc phạm vi baseline hiện tại. Chỉ bổ sung WebSocket sau khi benchmark HTTP và phê duyệt dependency tương thích.
