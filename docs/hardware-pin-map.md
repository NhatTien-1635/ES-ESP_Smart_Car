# Sơ đồ chân phần cứng ESP32-S3 Smart Car

## Trạng thái

- **Board**: ESP32-S3 DevKitC-1
- **Chassis**: 4 bánh omni/mecanum, bố trí hình chữ X theo hướng chuẩn của bộ bánh
- **Motor driver**: 2 module L298N, mỗi module điều khiển 2 động cơ DC
- **Khả năng hướng chéo**: hỗ trợ ở mức thiết kế bằng cách phối hợp tốc độ bốn bánh
- **Trạng thái sơ đồ**: đề xuất triển khai; phải kiểm tra continuity, mức điện áp và chiều quay trước khi cấp tải

## Sơ đồ GPIO/PWM đề xuất

| Chức năng | Tín hiệu | GPIO | Loại | Ghi chú |
|---|---|---:|---|---|
| Động cơ trước trái | L298N-A IN1 | 11 | Digital output | Chiều quay |
| Động cơ trước trái | L298N-A IN2 | 12 | Digital output | Chiều quay |
| Động cơ trước trái | L298N-A ENA | 13 | PWM output | Tháo jumper ENA để điều khiển PWM |
| Động cơ trước phải | L298N-A IN3 | 14 | Digital output | Chiều quay |
| Động cơ trước phải | L298N-A IN4 | 15 | Digital output | Chiều quay |
| Động cơ trước phải | L298N-A ENB | 16 | PWM output | Tháo jumper ENB để điều khiển PWM |
| Động cơ sau trái | L298N-B IN1 | 17 | Digital output | Chiều quay |
| Động cơ sau trái | L298N-B IN2 | 18 | Digital output | Chiều quay |
| Động cơ sau trái | L298N-B ENA | 21 | PWM output | Tháo jumper ENA để điều khiển PWM |
| Động cơ sau phải | L298N-B IN3 | 38 | Digital output | Chiều quay |
| Động cơ sau phải | L298N-B IN4 | 39 | Digital output | Chiều quay |
| Động cơ sau phải | L298N-B ENB | 40 | PWM output | Tháo jumper ENB để điều khiển PWM |
| Cảm biến line 1 | LINE_0 | 41 | Digital input | Thứ tự trái sang phải |
| Cảm biến line 2 | LINE_1 | 42 | Digital input | Thứ tự trái sang phải |
| Cảm biến line 3 | LINE_2 | 43 | Digital input | Thứ tự trái sang phải |
| Cảm biến line 4 | LINE_3 | 44 | Digital input | Thứ tự trái sang phải |
| Cảm biến line 5 | LINE_4 | 47 | Digital input | Thứ tự trái sang phải |
| HC-SR04 | TRIG | 48 | Digital output | Xung kích hoạt |
| HC-SR04 | ECHO | 4 | Digital input | Phải hạ mức 5 V xuống tối đa 3.3 V |
| DHT11 | DATA | 5 | Digital input/output | Dùng điện trở kéo lên theo module/cảm biến |
| Đo điện áp pin | BATTERY_ADC | 1 | ADC input | Bắt buộc dùng cầu phân áp trước ADC |

## Sơ đồ kết nối L298N

- `L298N-A` điều khiển động cơ trước trái và trước phải.
- `L298N-B` điều khiển động cơ sau trái và sau phải.
- `ENA`, `ENB` của cả hai module nhận PWM từ ESP32-S3.
- Các chân `IN1..IN4` nhận mức logic điều khiển chiều quay.
- ESP32-S3, hai module L298N và nguồn cảm biến phải dùng chung `GND`.
- Không cấp nguồn động cơ từ chân 3.3 V của ESP32-S3.
- Jumper `ENA/ENB` phải được tháo khi dùng PWM; giữ jumper sẽ làm kênh chạy ở mức bật cố định.

## Giả định điện áp và an toàn

- GPIO ESP32-S3 là logic 3.3 V. Tín hiệu `HC-SR04 ECHO` 5 V không được nối trực tiếp vào GPIO4; phải dùng cầu phân áp hoặc bộ chuyển mức.
- Nguồn logic L298N phải chấp nhận mức HIGH 3.3 V; cần kiểm tra datasheet/module thực tế trước khi nối.
- Nguồn động cơ phải tách khỏi nguồn USB/logic khi chạy tải. Tất cả nguồn vẫn phải nối chung mass.
- `BATTERY_ADC` chỉ được nối qua cầu phân áp có tính toán để điện áp tại GPIO1 không vượt quá giới hạn ADC. Tỉ lệ cầu phân áp và hệ số hiệu chuẩn phải được ghi bổ sung trước khi coi số đo là chính xác.
- Các cảm biến line phải xuất mức logic không vượt quá 3.3 V. Nếu module xuất 5 V, phải thêm chuyển mức hoặc chia áp.

## Các chân ESP32-S3 cố ý không sử dụng

- GPIO0, GPIO3, GPIO45 và GPIO46: tránh ảnh hưởng đến strapping/khởi động.
- GPIO19 và GPIO20: dành riêng cho USB-JTAG/USB serial trên các cấu hình board phổ biến.
- GPIO26–GPIO37: dành cho flash/PSRAM tích hợp và không dùng cho ngoại vi ứng dụng.
- GPIO2 hiện đang dùng cho LED tích hợp trong `src/main.cpp`; không tái sử dụng cho phần cứng xe.
- GPIO6–GPIO10 và GPIO22–GPIO25 được giữ dự phòng để thay thế khi cần sau khi kiểm tra board thực tế.

## Kiểm tra bắt buộc trước khi triển khai driver

1. Đối chiếu tên chân in trên board ESP32-S3 DevKitC-1 thực tế với bảng trên.
2. Dùng đồng hồ đo xác nhận không chập giữa nguồn động cơ, logic và GPIO.
3. Xác nhận `HC-SR04 ECHO` sau mạch hạ mức không vượt quá 3.3 V.
4. Chạy từng kênh L298N ở PWM thấp, không gắn bánh, để xác nhận chiều quay và thứ tự động cơ.
5. Ghi lại cực tính thực tế của bốn động cơ; nếu chiều quay ngược, đảo cặp dây động cơ hoặc đảo logic trong adapter, không sửa ở lớp web.
6. Kiểm tra bốn mẫu hướng chéo trên chassis và xác nhận xe không bị xoay ngoài ý muốn trước khi bật điều khiển tải đầy đủ.
