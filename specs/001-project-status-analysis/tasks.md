---

description: "Danh sách tác vụ triển khai firmware xe thông minh với D-pad 8 hướng"
---

# Danh sách tác vụ: Phân tích kiến trúc và trạng thái dự án xe thông minh

**Tài liệu đầu vào**: Các tài liệu thiết kế trong `specs/001-project-status-analysis/`

**Điều kiện tiên quyết**: [`plan.md`](./plan.md), [`spec.md`](./spec.md), [`research.md`](./research.md), [`data-model.md`](./data-model.md), [`contracts/`](./contracts/), [`quickstart.md`](./quickstart.md)

**Mục tiêu triển khai**: ESP32-S3 DevKitC-1, framework Arduino, PlatformIO, LittleFS, một phiên điều khiển SoftAP cục bộ và điều khiển thủ công 8 hướng.

## Giai đoạn 1: Chuẩn bị

**Mục đích**: Chốt phần cứng, hợp đồng lệnh và môi trường kiểm thử trước khi viết logic.

- [X] T001 Xác nhận loại khung xe, mạch điều khiển động cơ, sơ đồ nối dây ESP32-S3, năm cảm biến dò line, HC-SR04, DHT11 và đo pin; ghi sơ đồ GPIO/PWM được duyệt vào `docs/hardware-pin-map.md`; DoD: mọi tín hiệu cần thiết có đúng một GPIO, các giả định điện áp và khả năng chuyển động chéo được ghi lại, xung đột với chân khởi động được rà soát và sơ đồ được phê duyệt.
- [X] T002 [P] Cấu hình các môi trường build/kiểm thử PlatformIO trong `platformio.ini`; DoD: `pio run -e esp32-s3-devkitc-1` vẫn khả dụng, môi trường kiểm thử native có thể được phát hiện và chưa thêm phụ thuộc WebSocket trước cổng quyết định giao thức.
- [X] T003 [P] Cố định hợp đồng lệnh có phiên bản trong `specs/001-project-status-analysis/contracts/commands.md`; DoD: `MOVE` chỉ chấp nhận đúng tám hướng `FORWARD|BACKWARD|LEFT|RIGHT|FORWARD_LEFT|FORWARD_RIGHT|BACKWARD_LEFT|BACKWARD_RIGHT`, `STOP` có tính idempotent, hành vi khi hướng chéo không được hỗ trợ được mô tả rõ và khả năng tương thích với `F|B|L|R|S` cũ được ghi lại.
- [X] T004 [P] Cố định hợp đồng trạng thái và telemetry trong `specs/001-project-status-analysis/contracts/telemetry.md` và `specs/001-project-status-analysis/data-model.md`; DoD: `motion` hỗ trợ `STOP` cùng cả tám hướng, các trường trạng thái/tính hợp lệ và số lượng cảm biến line không còn mơ hồ, hướng chéo không bao giờ bị hạ cấp âm thầm.
- [X] T005 [P] Ghi lại cách thiết lập board, yêu cầu điều khiển 8 hướng và các lệnh kiểm tra trong `README.md`; DoD: người đóng góp mới có thể xác định board, phụ thuộc sơ đồ chân, cách build, nạp filesystem, tốc độ serial, cách truy cập AP, tám tên lệnh và các lệnh kiểm thử.

---

## Giai đoạn 2: Nền tảng

**Mục đích**: Tạo các kiểu dữ liệu, interface và điểm nối kiểm thử dùng chung; không bắt đầu công việc của user story trước checkpoint này.

- [X] T006 Tạo các kiểu điều khiển dùng chung trong `include/robot_types.h`; DoD: `mode` chính xác là `IDLE|MANUAL|LINE_TRACKING`, `motion` chính xác là `STOP|FORWARD|BACKWARD|LEFT|RIGHT|FORWARD_LEFT|FORWARD_RIGHT|BACKWARD_LEFT|BACKWARD_RIGHT`, tốc độ nằm trong khoảng 0–100 và `ControlCommand` phân biệt lệnh không hợp lệ/không được hỗ trợ với lệnh được chấp nhận.
- [X] T007 Định nghĩa kết quả điều khiển động cơ và interface phối hợp tốc độ trong `include/motor_driver.h`; DoD: interface chấp nhận mọi giá trị chuyển động, cung cấp kết quả rõ ràng khi hướng chéo không được hỗ trợ, hỗ trợ `stopAll()` và không phụ thuộc web/API.
- [X] T008 Định nghĩa interface adapter cảm biến và snapshot trong `include/sensors.h`, `include/telemetry.h` và `include/system_context.h`; DoD: cảm biến được đọc theo chu kỳ thay vì theo yêu cầu, snapshot có thể được sao chép trước khi serialize và code web không trực tiếp sở hữu GPIO.
- [X] T009 [P] Thêm motor giả, cảm biến giả và clock xác định trong `test/support/fake_motor_driver.h`, `test/support/fake_sensors.h` và `test/support/test_clock.h`; DoD: kiểm thử host có thể quan sát ý định PWM cardinal/chéo, kết quả không hỗ trợ, `STOP`, các giá trị cảm biến không hợp lệ được chèn vào và các mốc timeout mà không cần phần cứng ESP32.
- [X] T010 [P] Thêm các helper chẩn đoán trong `include/diagnostics.h` và `src/diagnostics.cpp`; DoD: lỗi lệnh không hợp lệ, hướng chéo không được hỗ trợ, filesystem, Wi-Fi, task và cảm biến có thông báo quan sát được riêng biệt, không có fallback giả dạng thành công.

**Checkpoint**: sơ đồ chân/khả năng phần cứng, các hợp đồng, enum 8 hướng, adapter giả và ranh giới sở hữu đã được phê duyệt.

---

## Giai đoạn 3: User Story 1 - Nắm được kiến trúc và ranh giới điều khiển (Ưu tiên: P1) 🎯 MVP

**Mục tiêu**: Có controller độc lập với web, quản lý lệnh 8 hướng, thay thế lệnh an toàn và xử lý timeout.

**Kiểm thử độc lập**: Kiểm thử native chứng minh mọi hướng hợp lệ/không hỗ trợ, thay thế lệnh, `STOP` và timeout; build ESP32 không phụ thuộc giao diện.

### Kiểm thử cho User Story 1

- [X] T011 [P] [US1] Viết kiểm thử xác thực lệnh 8 hướng trong `test/test_robot_controller/test_direction_validation.cpp`; DoD: cả tám giá trị hướng đều được chấp nhận, hướng không biết/thiếu bị từ chối, tốc độ ngoài khoảng 0–100 bị từ chối và hướng chéo không hỗ trợ khiến trạng thái cùng động cơ dừng.
- [X] T012 [P] [US1] Viết kiểm thử chuyển trạng thái và thay thế lệnh trong `test/test_robot_controller/test_state_transitions.cpp`; DoD: `IDLE -> MANUAL`, cardinal sang chéo, chéo sang chéo, `MANUAL -> LINE_TRACKING`, `STOP` và thay thế lệnh không bao giờ để chuyển động cũ tiếp tục.
- [X] T013 [P] [US1] Viết kiểm thử an toàn khi timeout/mất kết nối trong `test/test_robot_controller/test_safety_timeout.cpp`; DoD: timeout và mất kết nối gọi `stopAll()`, đặt tốc độ về 0, chuyển sang `IDLE`, cung cấp đúng lý do và có tính idempotent.

### Triển khai cho User Story 1

- [X] T014 [US1] Triển khai phân tích/xác thực lệnh và chuyển trạng thái trong `src/robot_controller.cpp` và `include/robot_controller.h`; DoD: mọi quy tắc `ControlCommand` trong `data-model.md` được thực thi, định danh hướng chéo được giữ nguyên, lệnh không hợp lệ không cập nhật trạng thái/động cơ và `STOP` được ưu tiên.
- [ ] T015 [US1] Triển khai phối hợp tốc độ cardinal/chéo theo loại khung xe trong `src/motor_driver.cpp` và `include/motor_driver.h` dựa trên `docs/hardware-pin-map.md`; DoD: mỗi hướng chéo được hỗ trợ ánh xạ thành tổ hợp bánh/PWM đã duyệt, hướng chéo không hỗ trợ trả về kết quả rõ ràng mà không tạo chuyển động, ánh xạ cardinal vẫn đúng và `STOP` đưa mọi kênh về 0.
- [ ] T016 [US1] Thêm xử lý tick timeout/mất kết nối của controller trong `src/robot_controller.cpp`; DoD: trạng thái đang chuyển động dừng sau timeout đơn điệu đã cấu hình hoặc phiên bị cũ, lý do an toàn được giữ lại và lệnh cardinal/chéo được làm mới sẽ ngăn dừng sớm.
- [ ] T017 [US1] Kết nối controller, motor driver và quyền sở hữu task trong `src/main.cpp`, `include/task_webapp.h` và `include/system_context.h`; DoD: một controller duy nhất sở hữu chuyển động, LED task hiện có vẫn hoạt động, code web không thể bỏ qua controller và lỗi khởi động có thể quan sát được.
- [ ] T018 [US1] Chạy kiểm thử native của controller và build ESP32 cho `test/test_robot_controller/` và `platformio.ini`; DoD: `pio test -e native` và `pio run -e esp32-s3-devkitc-1` thành công, không xuất hiện cảnh báo controller mới.

**Checkpoint**: chuyển động 8 hướng được mô hình hóa và hoặc được driver đã duyệt thực thi, hoặc bị từ chối an toàn mà không phụ thuộc trình duyệt.

---

## Giai đoạn 4: User Story 2 - Xác định và tích hợp chức năng hiện có (Ưu tiên: P1)

**Mục tiêu**: Kết nối API/telemetry thực với controller mà không làm mất hợp đồng 8 hướng hoặc hành vi an toàn.

**Kiểm thử độc lập**: Kiểm thử hợp đồng endpoint chứng minh các lệnh cardinal cũ, lệnh 8 hướng có phiên bản, việc từ chối hướng chéo và dữ liệu telemetry.

### Kiểm thử cho User Story 2

- [ ] T019 [P] [US2] Thêm kiểm thử hợp đồng endpoint lệnh trong `test/test_web_contract/test_commands.cpp`; DoD: `/api/cmd` chỉ chấp nhận `F|B|L|R|S` cũ, bộ phân tích lệnh có phiên bản chấp nhận cả tám hướng, dữ liệu sai/không được hỗ trợ trả về 4xx và phản hồi hợp lệ xác nhận chuyển động đã chấp nhận.
- [ ] T020 [P] [US2] Thêm kiểm thử serialize telemetry trong `test/test_telemetry/test_serialization.cpp`; DoD: JSON có `schema_version` dương, `device_id`, trạng thái, mọi trường tính hợp lệ của cảm biến và các giá trị `motion` bao gồm hướng chéo mà không thay đổi ngữ nghĩa payload cảm biến hiện có.
- [ ] T021 [P] [US2] Thêm kiểm thử xử lý cảm biến trong `test/test_sensors/test_readings.cpp`; DoD: khoảng cách âm, độ ẩm ngoài 0–100, lần đọc DHT bị thiếu, mẫu cũ và lần đọc pin không hợp lệ được đánh dấu không hợp lệ/cũ thay vì bịa dữ liệu.

### Triển khai cho User Story 2

- [ ] T022 [P] [US2] Triển khai lấy mẫu định kỳ cho line, HC-SR04, DHT11 và pin trong `src/sensors.cpp` và `include/sensors.h`; DoD: kết quả đọc có metadata timestamp/tính hợp lệ, lỗi phần cứng được chuyển đến chẩn đoán và việc lấy mẫu không diễn ra một lần cho mỗi request HTTP.
- [ ] T023 [US2] Triển khai snapshot telemetry bất biến và serialize JSON trong `src/telemetry.cpp` và `include/telemetry.h`; DoD: đầu ra tuân theo `contracts/telemetry.md`, serialize diễn ra bên ngoài khóa trạng thái dùng chung và không có mẫu hard-code nào được gắn nhãn là phép đo thực.
- [ ] T024 [US2] Thay thế các handler chỉ ghi log và telemetry cố định trong `src/task_webapp.cpp`; DoD: `/api/cmd` ủy quyền cho controller, lệnh có phiên bản giữ nguyên cả tám hướng, `/api/linetrace` dừng trước khi đổi mode, `/api/telemetry` trả về snapshot hiện tại và dữ liệu sai/không hỗ trợ trả về 4xx.
- [ ] T025 [US2] Thêm kiểm thử tích hợp HTTP trong `test/test_web_contract/test_http_integration.cpp`; DoD: phục vụ index, lệnh cũ, lệnh cardinal/chéo có phiên bản, hướng chéo không hỗ trợ, line tracing, telemetry và hành vi not-found khớp các hợp đồng đã viết.
- [ ] T026 [US2] Thêm xử lý lỗi khởi tạo LittleFS, Wi-Fi và task trong `src/task_webapp.cpp` và `src/main.cpp`; DoD: lỗi filesystem, thiếu `/index.html`, lỗi AP và lỗi tạo task được phân biệt, không bao giờ báo hệ thống sẵn sàng điều khiển khi chưa thực sự sẵn sàng.

**Checkpoint**: ranh giới API/UI cung cấp đủ tám hướng, báo rõ hướng chéo không hỗ trợ và trả telemetry trung thực.

---

## Giai đoạn 5: User Story 3 - Điều khiển 8 hướng và phát triển tiếp (Ưu tiên: P1)

**Mục tiêu**: Hoàn thiện UI 8 hướng, chế độ tự động, khả năng phục hồi và nghiệm thu phần cứng.

**Kiểm thử độc lập**: Các kịch bản trên trình duyệt và board xác minh tám nút, thay thế lệnh, các đường dẫn `STOP`, telemetry, mất kết nối và khả năng/hành vi từ chối hướng chéo.

### Kiểm thử cho User Story 3

- [ ] T027 [P] [US3] Thêm kiểm thử hồi quy UI 8 hướng trong `test/test_web_contract/test_ui_8way.js`; DoD: tám điều khiển hiển thị các lệnh khác nhau, mỗi tương tác pointer/bàn phím gửi đúng hướng riêng và không hướng chéo nào bị chuyển âm thầm thành hướng thẳng.
- [ ] T028 [P] [US3] Thêm kiểm thử hồi quy an toàn UI trong `test/test_web_contract/test_ui_safety.js`; DoD: thả nút, hủy pointer, chuyển tab, đưa trang ra nền, blur, `STOP` và bật/tắt line tracing đều dừng chuyển động cardinal hoặc chéo đang hoạt động mà không tạo các vòng lặp lệnh chồng lấn.
- [ ] T029 [P] [US3] Thêm kiểm thử an toàn chế độ tự động/vật cản trong `test/test_robot_controller/test_autonomy_safety.cpp`; DoD: các mẫu bám line tạo ra đánh lái xác định, cảm biến không hợp lệ/vật cản khiến xe dừng và lệnh thủ công cardinal/chéo bị từ chối khi mode tự động đang sở hữu chuyển động.
- [ ] T030 [P] [US3] Thêm quy trình nghiệm thu khả năng phục hồi và độ trễ trong `test/test_web_contract/test_resilience.md`; DoD: quy trình ghi nhận thời gian từ timeout đến dừng, hành vi kết nối lại và độ trễ lệnh p95, với yêu cầu dừng an toàn trong timeout đã cấu hình và dưới 100 ms p95 khi mục tiêu được bật.

### Triển khai cho User Story 3

- [ ] T031 [US3] Mở rộng markup và bố cục D-pad thành tám điều khiển riêng biệt trong `data/index.html`; DoD: bốn nút cardinal và bốn nút chéo có `data-cmd` riêng, nhãn/icon hướng chéo hiển thị rõ, tên truy cập được và không xung đột với `STOP`.
- [ ] T032 [US3] Cập nhật xử lý pointer, bàn phím và lệnh đang hoạt động trong `data/index.html`; DoD: lệnh chéo được lặp theo chính sách keepalive hiện có, thay lệnh không để lệnh cũ tiếp tục, thả/hủy gửi `STOP` và line tracing vô hiệu hóa cả tám nút.
- [ ] T033 [US3] Cập nhật hiển thị telemetry và trạng thái mất kết nối trong `data/index.html`; DoD: các giá trị `motion` có phiên bản, bao gồm hướng chéo, được hiển thị đúng, giá trị cảm biến không hợp lệ/cũ khác biệt với 0 và mất kết nối được hiển thị mà không ngụ ý có thể tiếp tục chuyển động an toàn.
- [ ] T034 [US3] Triển khai bám line và dừng khi gặp vật cản trong `src/robot_controller.cpp` và `src/sensors.cpp`; DoD: số liệu line hợp lệ điều khiển chế độ tự động, line bị mất/không hợp lệ và khoảng cách nhỏ hơn ngưỡng đã duyệt gọi `stopAll()`, lệnh thủ công 8 hướng không thể bỏ qua quyền sở hữu của chế độ tự động.
- [ ] T035 [US3] Triển khai theo dõi client cũ và dừng an toàn trong `src/task_webapp.cpp`; DoD: lệnh/heartbeat cũ khiến trạng thái thành `DISCONNECTED` và `IDLE`, đầu ra động cơ bằng 0 và kết nối lại bắt đầu ở trạng thái dừng thay vì tiếp tục cardinal hoặc chéo.
- [ ] T036 [US3] Đo benchmark điều khiển HTTP và chỉ thêm adapter WebSocket khi cần trong `src/task_webapp.cpp` và `platformio.ini`; DoD: giao thức được chọn dùng cùng hợp đồng 8 hướng, kết nối lại an toàn và p95 đo được dưới 100 ms hoặc mục tiêu chưa đạt được ghi rõ.
- [ ] T037 [US3] Thực hiện nghiệm thu trên board theo `specs/001-project-status-analysis/quickstart.md`; DoD: AP/UI, bốn hướng cardinal, bốn hướng chéo được hỗ trợ hoặc phản hồi không hỗ trợ an toàn, `STOP`, line tracing, dừng vật cản, tính hợp lệ cảm biến, telemetry 2–5 giây và dừng khi mất kết nối được ghi nhận đạt/không đạt.

**Checkpoint**: cả ba user story đều có thể trình diễn; điều khiển 8 hướng không bỏ qua xác thực và không hoạt động theo cách mất an toàn.

---

## Giai đoạn 6: Hoàn thiện và các mối quan tâm xuyên suốt

**Mục đích**: Hoàn thiện hồi quy, tài liệu và bằng chứng phát hành.

- [ ] T038 [P] Cập nhật `README.md` và tài liệu liên quan `docs/TÀI LIỆU ĐẶC TẢ YÊU CẦU HỆ THỐNG.docx` với enum 8 hướng cuối cùng, khả năng của khung xe, sơ đồ chân, phiên bản payload và các giới hạn; DoD: tài liệu không tuyên bố hướng chéo không hỗ trợ, telemetry thực hoặc WebSocket trước khi nghiệm thu.
- [ ] T039 [P] Thêm hướng dẫn kiểm thử/xử lý sự cố có thể lặp lại trong `docs/testing.md`; DoD: các lệnh kiểm thử native, build, nạp filesystem, chẩn đoán serial, smoke test trình duyệt 8 hướng và nghiệm thu phần cứng có thể thực hiện lại.
- [ ] T040 Chạy đầy đủ ma trận xác thực trong `specs/001-project-status-analysis/quickstart.md`; DoD: `pio run`, các môi trường `pio test` liên quan, kiểm tra filesystem, kiểm tra hợp đồng 8 hướng và các kịch bản an toàn đều đạt hoặc có lỗi cần xử lý rõ ràng.
- [ ] T041 Rà soát `src/`, `include/`, `data/`, `test/` và `platformio.ini` để tìm giả định 4 hướng cũ cùng các mặc định không an toàn; DoD: không còn giả định UI bốn nút trong các luồng điều khiển mới, không hướng chéo nào bị ánh xạ âm thầm, lệnh không hợp lệ không trả về thành công và hành vi không liên quan được giữ nguyên.

---

## Phụ thuộc và thứ tự thực hiện

### Phụ thuộc giữa các giai đoạn

- **Giai đoạn 1 - Chuẩn bị**: bắt đầu ngay; T001 phụ thuộc phần cứng, còn T002–T005 có thể thực hiện song song sau khi phạm vi được chấp thuận.
- **Giai đoạn 2 - Nền tảng**: phụ thuộc T001–T005; T006–T008 định nghĩa interface, T009–T010 có thể thực hiện song song sau khi các interface được thống nhất.
- **Giai đoạn 3 - US1**: phụ thuộc Giai đoạn 2; T011–T013 là các tài sản kiểm thử song song, sau đó T014–T018 triển khai và xác minh ranh giới controller/motor.
- **Giai đoạn 4 - US2**: phụ thuộc các kiểu trạng thái của US1 và T003–T004; T019–T021 là các tài sản kiểm thử song song, sau đó T022–T026 tích hợp cảm biến, telemetry và web handler.
- **Giai đoạn 5 - US3**: phụ thuộc trạng thái an toàn của US1 và các hợp đồng của US2; T027–T030 là các tài sản xác thực song song, sau đó T031–T037 tích hợp UI, chế độ tự động, khả năng phục hồi và giao thức tùy chọn.
- **Giai đoạn 6 - Hoàn thiện**: phụ thuộc mức độ hoàn thành mong muốn của US1–US3.

### Thứ tự hoàn thành user story

- **US1 (P1)**: phần triển khai đầu tiên; thiết lập controller và ranh giới motor an toàn cho 8 hướng.
- **US2 (P1)**: phụ thuộc US1; cung cấp API và telemetry trung thực.
- **US3 (P1)**: phụ thuộc US1 và US2; cung cấp UI tám nút và hành vi phần cứng hoàn chỉnh.

### Ví dụ thực hiện song song

```text
Sau Giai đoạn 1:
- T002 Cấu hình kiểm thử PlatformIO
- T003 Hợp đồng lệnh
- T004 Hợp đồng trạng thái/telemetry
- T005 Thiết lập README

Trong US1:
- T011 Kiểm thử xác thực hướng
- T012 Kiểm thử chuyển trạng thái/thay thế lệnh
- T013 Kiểm thử timeout/mất kết nối

Trong US2:
- T019 Kiểm thử endpoint lệnh
- T020 Kiểm thử serialize telemetry
- T021 Kiểm thử xử lý cảm biến

Trong US3:
- T027 Kiểm thử UI 8 hướng
- T028 Kiểm thử an toàn UI
- T029 Kiểm thử an toàn chế độ tự động
- T030 Quy trình nghiệm thu khả năng phục hồi/độ trễ
```

## Chiến lược triển khai

1. **MVP**: Hoàn thành Giai đoạn 1–3 (US1) với khả năng khung xe đã được phê duyệt; lệnh chéo hoặc được adapter động cơ thực thi, hoặc bị lỗi an toàn.
2. **Increment 2**: Hoàn thành US2 để cung cấp lệnh 8 hướng đã xác thực và telemetry trung thực qua ranh giới web hiện có.
3. **Increment 3**: Hoàn thành US3 cho UI 8 hướng, chế độ tự động, khả năng phục hồi và hiệu năng giao thức được đo lường.
4. **Hoàn thiện phát hành**: Hoàn thành Giai đoạn 6 và yêu cầu bằng chứng từ quickstart/phần cứng trước khi xem robot là sẵn sàng.
