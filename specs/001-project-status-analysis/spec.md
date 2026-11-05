# Feature Specification: Phân tích kiến trúc và trạng thái dự án xe thông minh

**Feature Branch**: `001-project-status-analysis`

**Created**: 2026-11-05

**Status**: Draft

**Input**: User description: "Hãy phân tích toàn bộ cấu trúc thư mục, các file mã nguồn hiện có và file README của dự án này. Mục tiêu của tôi là tiếp tục phát triển phần dang dở. Hãy lập một tài liệu đặc tả (specification) tóm tắt: 1. Kiến trúc tổng quan và công nghệ đang sử dụng. 2. Các module/tính năng đã hoàn thành và trạng thái hiện tại. 3. Các phần việc còn dang dở hoặc chưa được triển khai. Hãy lưu đặc tả này vào thư mục specs/ để chúng ta làm căn cứ trước khi viết code mới."

**Change Request**: Cập nhật D-pad từ 4 hướng hiện tại thành 8 hướng, bổ sung `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT` và `BACKWARD_RIGHT`.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Nắm được kiến trúc hiện tại (Priority: P1)

Là người tiếp tục phát triển dự án, tôi cần một bản mô tả ngắn gọn về cấu trúc phần cứng/phần mềm, luồng điều khiển và công nghệ hiện có để có thể chọn đúng điểm mở rộng.

**Why this priority**: Nếu không có baseline kiến trúc, các thay đổi tiếp theo dễ trùng lặp hoặc phá vỡ luồng khởi động hiện tại.

**Independent Test**: Một người mới đọc đặc tả có thể xác định firmware, giao diện web, dữ liệu tĩnh, cấu hình build và các điểm giao tiếp chính mà không cần đọc toàn bộ mã nguồn trước.

**Acceptance Scenarios**:

1. **Given** đặc tả đã được tạo, **When** người đọc xem phần kiến trúc, **Then** họ thấy rõ vai trò của `src/`, `include/`, `data/`, `platformio.ini` và môi trường ESP32-S3.
2. **Given** người đọc cần hiểu luồng chạy, **When** họ xem mô tả runtime, **Then** họ biết firmware khởi tạo hai task, mở SoftAP và phục vụ giao diện web qua LittleFS.

### User Story 2 - Xác định chức năng đã có và mức độ hoàn thành (Priority: P1)

Là người bảo trì, tôi cần biết chức năng nào đang chạy được, chức năng nào chỉ là giao diện hoặc mô phỏng, để tránh coi mã mẫu là tính năng đã hoàn thiện.

**Why this priority**: Phân biệt trạng thái hiện tại là cơ sở để lập kế hoạch triển khai tiếp theo và kiểm thử đúng phạm vi.

**Independent Test**: Đối chiếu từng mục trong đặc tả với mã nguồn hiện có, người đọc có thể đánh dấu chức năng là đã tích hợp, một phần, mô phỏng hoặc chưa có.

**Acceptance Scenarios**:

1. **Given** giao diện web hiện tại, **When** người đọc kiểm tra danh sách tính năng, **Then** D-pad 4 hướng hiện tại, nút dừng, chuyển tab, line tracing toggle và hiển thị telemetry mẫu được ghi nhận đúng trạng thái.
2. **Given** các endpoint firmware, **When** người đọc kiểm tra trạng thái thực thi, **Then** họ thấy lệnh hiện chỉ ghi log, telemetry trả dữ liệu cố định và chưa điều khiển cơ cấu chấp hành.

### User Story 3 - Có backlog rõ ràng để phát triển tiếp (Priority: P1)

Là người lập kế hoạch, tôi cần danh sách phần việc còn thiếu được sắp xếp theo nhóm và phụ thuộc để có thể chuyển sang thiết kế và triển khai mà không bỏ sót yêu cầu cốt lõi.

**Why this priority**: Mục tiêu trực tiếp của tài liệu là làm căn cứ cho các thay đổi code tiếp theo.

**Independent Test**: Từ phần backlog, người lập kế hoạch có thể chọn một hạng mục độc lập, xác định đầu vào/đầu ra cần hoàn thành và biết điều gì chưa thuộc phạm vi hiện tại.

**Acceptance Scenarios**:

1. **Given** tài liệu yêu cầu hệ thống trong `docs/`, **When** so sánh với mã nguồn, **Then** các khoảng trống như cảm biến, motor/FSM, WebSocket, dữ liệu môi trường và resilience được liệt kê riêng.
2. **Given** một hạng mục backlog, **When** người đọc chuẩn bị triển khai, **Then** hạng mục có trạng thái hiện tại, kết quả mong đợi và phụ thuộc liên quan.

### Edge Cases

- Nếu không có README ở thư mục gốc, đặc tả phải ghi rõ tình trạng này và sử dụng các README mẫu trong `include/`, `lib/` và `test/` để mô tả quy ước thư mục.
- Nếu LittleFS không khởi tạo được hoặc thiếu `index.html`, firmware phải có hành vi lỗi quan sát được thay vì coi giao diện đã sẵn sàng.
- Nếu Wi-Fi hoặc trình duyệt mất kết nối, trạng thái giao diện phải chuyển sang ngoại tuyến và hệ thống phải có cơ chế dừng an toàn trước khi tính năng điều khiển thật được bật.
- Nếu telemetry thiếu trường hoặc số lượng cảm biến khác với giao diện, giao diện không được hiển thị dữ liệu sai như thể đó là giá trị đo thật.
- Nếu người dùng chuyển sang line tracing hoặc rời khỏi trang điều khiển, lệnh chuyển động đang giữ phải được dừng trước khi đổi chế độ.
- Nếu người dùng chọn hướng chéo, robot phải phối hợp đồng thời thành phần tiến/lùi và trái/phải; nếu phần cứng không hỗ trợ phối hợp đó, lệnh phải bị từ chối an toàn thay vì chạy một hướng khác.
- Nếu người dùng chuyển nhanh giữa hai hướng hoặc nhả một nút chéo, chỉ lệnh của trạng thái điều khiển hiện tại được duy trì; lệnh cũ phải được dừng.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Đặc tả MUST mô tả cấu trúc thư mục và vai trò của mọi thư mục chứa mã nguồn, dữ liệu giao diện, cấu hình build và kiểm thử hiện có.
- **FR-002**: Đặc tả MUST ghi nhận công nghệ và cấu hình hiện tại gồm PlatformIO, ESP32-S3 DevKitC-1, Arduino, LittleFS, Wi-Fi SoftAP và thư viện ArduinoJson.
- **FR-003**: Đặc tả MUST mô tả luồng khởi động hiện tại, gồm khởi tạo Serial, task web, task LED và vòng xử lý rỗng trong `loop()`.
- **FR-004**: Đặc tả MUST liệt kê các endpoint web hiện có, dữ liệu đầu vào của chúng và hành vi quan sát được khi gọi.
- **FR-005**: Đặc tả MUST phân loại từng module/tính năng hiện có theo ít nhất một trạng thái: hoàn thành ở mức nền tảng, hoàn thành một phần, mô phỏng/chưa kết nối phần cứng hoặc chưa triển khai.
- **FR-006**: Đặc tả MUST phân biệt dữ liệu telemetry cố định trong firmware với dữ liệu cảm biến thật và không được mô tả giá trị mẫu là kết quả đo thực tế.
- **FR-007**: Đặc tả MUST liệt kê các phần việc còn dang dở hoặc chưa triển khai dựa trên chênh lệch giữa mã nguồn và tài liệu yêu cầu hệ thống hiện có.
- **FR-008**: Đặc tả MUST ghi rõ các giới hạn phạm vi đã được nêu: chưa có lưu trữ dữ liệu lịch sử dài hạn trên Cloud và chưa có backend trung gian hoạt động trong mã nguồn hiện tại.
- **FR-009**: Đặc tả MUST cung cấp các tiêu chí có thể kiểm tra để xác nhận một hạng mục backlog đã được hoàn thành trong các giai đoạn phát triển sau.
- **FR-010**: Các thay đổi tiếp theo MUST giữ hành vi an toàn của giao diện hiện tại: gửi lệnh dừng khi nhả điều khiển, khi mất focus/chuyển tab và khi hủy line tracing.
- **FR-011**: Giao diện điều khiển MUST cung cấp 8 hướng chuyển động: `FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT` và `BACKWARD_RIGHT`, cùng với nút `STOP`.
- **FR-012**: Mỗi hướng chéo MUST được biểu diễn rõ ràng trong UI, gửi đúng lệnh riêng biệt xuống firmware và không bị nhầm với một trong bốn hướng thẳng.
- **FR-013**: Firmware MUST validate và xử lý đủ 8 hướng; nếu hướng chéo chưa được phần cứng hoặc driver hỗ trợ, firmware MUST trả lỗi rõ ràng và giữ motor ở trạng thái dừng.
- **FR-014**: Khi một hướng chéo được giữ, giao diện MUST lặp lại đúng lệnh chéo theo cơ chế duy trì lệnh hiện tại và MUST gửi `STOP` khi nhả, hủy hoặc mất kết nối.

### Current Architecture and Status Baseline

#### Kiến trúc tổng quan

Đây là dự án firmware PlatformIO cho **ESP32-S3 DevKitC-1**, dùng framework Arduino và C++. Thiết bị hiện tạo một Wi-Fi SoftAP tên `ESP32_WebModule`, chạy web server cổng 80, DNS wildcard cho captive-portal-like redirect, và phục vụ `data/index.html` từ LittleFS. Giao diện trình duyệt cung cấp lớp điều khiển và hiển thị; firmware hiện chưa có lớp điều khiển motor hay lớp thu thập cảm biến thật.

#### Cấu trúc và công nghệ

- `platformio.ini`: môi trường `esp32-s3-devkitc-1`, nền tảng Espressif32, Arduino, LittleFS, tốc độ Serial 115200 và ArduinoJson `^7.2.2`.
- `src/main.cpp`: điểm vào firmware; tạo task web và task LED.
- `src/task_webapp.cpp`: SoftAP, DNS, web server, phục vụ file, các endpoint lệnh/telemetry.
- `include/task_webapp.h`: khai báo task web, hằng số SSID/mật khẩu và include phụ thuộc.
- `data/index.html`: giao diện điều khiển landscape-first, D-pad, STOP, line tracing, telemetry, trạng thái kết nối và cơ chế polling.
- `include/README`, `lib/README`, `test/README`: README mẫu do PlatformIO tạo cho header, thư viện riêng và test runner; không có README ở thư mục gốc.
- `.vscode/`: cấu hình debug và khuyến nghị extension PlatformIO; không phải module runtime.
- `docs/TÀI LIỆU ĐẶC TẢ YÊU CẦU HỆ THỐNG.docx`: yêu cầu mục tiêu về line following, tránh vật cản, DHT11, HC-SR04, WebSocket, telemetry môi trường và điều khiển xe.

#### Module/tính năng hiện tại

| Module/tính năng | Trạng thái | Bằng chứng và giới hạn |
|---|---|---|
| Firmware khởi động ESP32 và Serial | Hoàn thành ở mức nền tảng | `setup()` mở Serial và tạo task; chưa có kiểm tra lỗi task. |
| LED nhấp nháy | Hoàn thành độc lập | GPIO 2 đổi trạng thái mỗi 1 giây; chưa liên quan đến trạng thái xe. |
| Wi-Fi SoftAP và DNS | Hoàn thành ở mức nền tảng | SoftAP và DNS wildcard được khởi động; thông tin xác thực đang hard-code. |
| Web server và phục vụ giao diện | Hoàn thành ở mức nền tảng | Có route `/` và fallback redirect; LittleFS chưa được kiểm tra kết quả khởi tạo. |
| D-pad thủ công trên web | Đã có 4 hướng thẳng, còn thiếu 4 hướng chéo; firmware chưa tích hợp | Có pointer/keyboard, lặp lệnh mỗi 150 ms và dừng khi nhả; cần bổ sung 4 hướng chéo; endpoint chỉ ghi log. |
| STOP và cơ chế dừng an toàn phía UI | Hoàn thành ở UI | Gửi lệnh dừng hai lần, dừng khi mất focus, ẩn trang hoặc đổi tab; chưa có watchdog an toàn phía xe. |
| Bật/tắt line tracing | Giao diện và endpoint khung đã có | UI khóa D-pad và gửi `on=0/1`; firmware chỉ in log, chưa có thuật toán bám vạch. |
| Telemetry | Mô phỏng/chưa kết nối phần cứng | `/api/telemetry` trả JSON cố định `speed`, `battery`, `distance`, `mode`, `line`; chưa đọc cảm biến. |
| Hiển thị telemetry | Giao diện đã có | Poll 500 ms khi mở tab và 2 giây khi ẩn; không có biểu đồ nhiệt độ/độ ẩm như tài liệu mục tiêu. |
| Motor driver, 4 động cơ, omni wheel | Chưa triển khai | Không có chân GPIO, PWM, driver hoặc điều khiển chuyển động trong mã nguồn. |
| Cảm biến line, HC-SR04, DHT11 | Chưa triển khai | Không có đọc GPIO, xử lý đo hoặc tích hợp dữ liệu. |
| FSM và chế độ IDLE/MANUAL/LINE_TRACKING | Chưa triển khai | Chỉ có chuỗi `mode` cố định trong telemetry; chưa có state machine. |
| WebSocket/backend | Chưa triển khai | Hiện dùng HTTP polling; không có Node.js/Spring Boot hay WebSocket. |
| Test tự động | Chưa triển khai | `test/` chỉ có README mẫu, chưa có test case. |

#### Giao tiếp hiện có

- `GET /`: trả `index.html` từ LittleFS; trả 404 nếu file không tồn tại.
- `GET /api/cmd?d=<ký tự>`: nhận lệnh một ký tự và ghi log nếu độ dài bằng 1; luôn trả `200 ok`, kể cả khi đầu vào không hợp lệ.
- `GET /api/linetrace?on=0|1`: ghi trạng thái ON/OFF theo chuỗi; luôn trả `200 ok`.
- `GET /api/telemetry`: trả JSON telemetry mẫu.
- Route không xác định: redirect về `http://escp32.setup/` với mã 302; tên miền có vẻ là lỗi đánh máy cần xác minh trước khi dùng thực tế.

#### Phần việc còn dang dở

1. **Điều khiển cơ cấu chấp hành**: định nghĩa chân, PWM, driver và 8 lệnh hướng (gồm 4 hướng thẳng và 4 hướng chéo); liên kết endpoint với motor.
2. **Máy trạng thái an toàn**: thiết kế IDLE, MANUAL và LINE_TRACKING; timeout lệnh, dừng khi mất kết nối và ưu tiên STOP.
3. **Cảm biến và telemetry thật**: tích hợp cụm cảm biến line, HC-SR04, DHT11 và đo pin; chuẩn hóa payload theo tài liệu yêu cầu.
4. **Line following và tránh vật cản**: triển khai thuật toán, xử lý mất line/vật cản và kiểm thử trên phần cứng.
5. **Giao tiếp thời gian thực**: đánh giá chuyển từ polling HTTP sang WebSocket để đạt mục tiêu độ trễ điều khiển dưới 100 ms.
6. **Đồng bộ UI với yêu cầu mục tiêu**: bổ sung nhiệt độ, độ ẩm, trạng thái pin thực, tốc độ và mode thật; hiển thị rõ Disconnected.
7. **Resilience và bảo mật cấu hình**: kiểm tra lỗi LittleFS/Wi-Fi/task, tránh credential hard-code trong bản triển khai, và xác minh captive redirect.
8. **Kiểm thử và tài liệu phát triển**: thêm unit/integration/hardware test, README gốc, sơ đồ chân, hướng dẫn nạp filesystem và tiêu chí nghiệm thu.

## Key Entities

- **RobotState**: trạng thái vận hành, mode, lệnh chuyển động, tốc độ và trạng thái dừng an toàn.
- **Telemetry**: dữ liệu đo và trạng thái gồm tốc độ, điện áp pin, khoảng cách, mode, cảm biến line, nhiệt độ và độ ẩm khi được tích hợp.
- **ControlCommand**: yêu cầu từ UI, gồm một trong 8 hướng, tốc độ, STOP và chuyển mode.
- **SensorReading**: kết quả đọc từng cảm biến line, khoảng cách, nhiệt độ và độ ẩm cùng thời điểm đo.
- **WebSession**: kết nối giữa trình duyệt và SoftAP, gồm trạng thái online/offline và thời điểm nhận lệnh cuối.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Người phát triển mới có thể xác định điểm vào firmware, module web, dữ liệu giao diện và cấu hình build trong vòng 10 phút chỉ bằng cách đọc đặc tả.
- **SC-002**: 100% file mã nguồn hiện có (`src/`, `include/`, `data/`) được ánh xạ tới một module hoặc vai trò cụ thể trong tài liệu.
- **SC-003**: 100% tính năng được liệt kê có trạng thái và bằng chứng; không có dữ liệu mô phỏng nào bị mô tả là dữ liệu cảm biến thật.
- **SC-004**: Ít nhất 8 nhóm phần việc còn thiếu được nêu cùng giới hạn hiện tại và kết quả cần đạt, đủ để chuyển thành task triển khai.
- **SC-005**: Một reviewer độc lập có thể đối chiếu đặc tả với repository và xác nhận các nhận định chính với không quá 3 điểm mơ hồ chưa được ghi chú.
- **SC-006**: Mọi thay đổi triển khai từ backlog phải có kiểm thử chứng minh lệnh STOP, mất kết nối và chuyển mode không để xe tiếp tục chạy ngoài ý muốn.
- **SC-007**: Người dùng có thể phát lệnh và quan sát phản hồi đúng cho cả 8 hướng; không có hướng chéo nào bị ánh xạ nhầm sang hướng thẳng.
- **SC-008**: 100% thao tác giữ/nhả/hủy đối với 8 hướng đều kết thúc bằng trạng thái dừng khi điều kiện điều khiển không còn hợp lệ.

## Assumptions

- Người đọc đặc tả có quyền truy cập repository và tài liệu yêu cầu hệ thống hiện có.
- Bản đặc tả này mô tả baseline hiện tại, không khẳng định các phần cứng chưa xuất hiện trong mã nguồn đã được lắp ráp hoặc đấu dây.
- `docs/TÀI LIỆU ĐẶC TẢ YÊU CẦU HỆ THỐNG.docx` là nguồn yêu cầu mục tiêu; các điểm chưa khớp với mã nguồn được coi là backlog, không phải lỗi của tài liệu.
- Việc lưu lịch sử dữ liệu dài hạn trên Cloud nằm ngoài phạm vi giai đoạn hiện tại, phù hợp với tài liệu yêu cầu.
- Các mục yêu cầu chi tiết về chân GPIO, loại motor driver, giao thức WebSocket cuối cùng và topology phần cứng cần được chốt trong một đặc tả triển khai riêng trước khi viết code.
