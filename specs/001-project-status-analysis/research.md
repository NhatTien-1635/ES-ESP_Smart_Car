# Research: Kế hoạch kỹ thuật xe thông minh

## Decision 1: Tách domain controller khỏi web task

- **Decision**: Dùng `RobotController` làm nguồn sự thật cho mode, chuyển động, timeout và safety stop; `task_webapp` chỉ parse request và gọi controller.
- **Rationale**: Handler hiện tại chỉ ghi log và trả thành công giả. Tách domain logic cho phép test native không cần board, ngăn web task điều khiển GPIO trực tiếp và làm rõ ownership của hardware.
- **Alternatives considered**: Giữ toàn bộ logic trong `task_webapp.cpp` (bị từ chối vì khó test và dễ block); dùng backend trung gian (ngoài phạm vi SoftAP/offline v1).

## Decision 2: Shared snapshot bất biến giữa task

- **Decision**: Controller/sensor task cập nhật state và sensor readings; telemetry tạo snapshot rồi serialize ngoài vùng lock.
- **Rationale**: Tránh JSON serialization hoặc đọc cảm biến giữ lock lâu; snapshot giúp UI nhận dữ liệu nhất quán. Timeout và STOP phải idempotent để xử lý lặp do retry.
- **Alternatives considered**: Global variables không đồng bộ (bị từ chối vì race condition); gửi từng field qua nhiều queue (bị từ chối vì snapshot dễ lệch thời điểm).

## Decision 3: HTTP polling trước, WebSocket sau

- **Decision**: Giữ endpoint HTTP hiện tại làm bước tích hợp đầu; đo latency thực tế rồi mới thêm WebSocket adapter cùng payload.
- **Rationale**: Đây là thay đổi ít rủi ro, tương thích UI hiện có và cho phép hoàn thiện safety/domain trước. WebSocket phù hợp mục tiêu hai chiều dưới 100 ms nhưng thêm dependency, lifecycle reconnect và test surface.
- **Alternatives considered**: Chuyển ngay toàn bộ sang WebSocket (bị hoãn vì chưa có domain contract và test); tăng tần suất polling vô hạn (bị từ chối vì tạo request chồng lấn và tốn tài nguyên).

## Decision 4: Test logic bằng native/fake adapters, xác nhận cuối trên board

- **Decision**: Viết unit tests cho validation/state/serialization với fake motor và sensor; dùng integration/hardware tests cho GPIO, LittleFS, Wi-Fi và latency.
- **Rationale**: Logic an toàn có thể kiểm tra nhanh, lặp lại trên host; hành vi phụ thuộc ESP32 vẫn cần board acceptance.
- **Alternatives considered**: Chỉ test thủ công trên board (bị từ chối vì regression chậm và không bao phủ edge cases); chỉ native test (bị từ chối vì không xác nhận wiring/peripheral).

## Decision 5: Contract versioning và lỗi HTTP rõ ràng

- **Decision**: Payload có `schema_version`, trường validity/error; command sai trả 4xx thay vì `200 ok`.
- **Rationale**: UI hiện coi mọi response là thành công và telemetry mẫu không thể phân biệt với dữ liệu thật. Contract version giúp thay đổi payload có kiểm soát.
- **Alternatives considered**: Giữ JSON tối giản không version (bị từ chối vì khó tương thích); trả success-shaped fallback (bị cấm bởi yêu cầu surface errors).

## Decision 6: Diagonal commands remain explicit domain values

- **Decision**: Model all four diagonal directions as independent `direction` enum values. Preserve `F/B/L/R/S` only for the existing compatibility endpoint; use explicit names for diagonals in versioned commands.
- **Rationale**: The UI must not silently turn a diagonal request into a cardinal request. Keeping diagonal identity through parser, controller and telemetry makes validation, safety stop and hardware acceptance observable.
- **Alternatives considered**: Encode diagonals as two sequential cardinal commands (rejected because timing/race behavior can produce unintended motion); infer diagonals from simultaneous button presses (rejected because it complicates pointer release and network ordering).

## Decision 7: Chassis-specific diagonal mixing belongs in motor adapter

- **Decision**: `MotorDriver` owns the conversion from diagonal intent to wheel PWM. The controller accepts the domain command, but an unsupported driver returns an explicit failure and the controller keeps the robot stopped.
- **Rationale**: Omni/mecanum and differential chassis have different physical capabilities. Keeping the decision at the hardware boundary prevents web/API code from making unsafe assumptions and supports safe rejection until the pin map is approved.
- **Alternatives considered**: Hard-code a four-wheel PWM matrix in the web handler (rejected due to layering and hardware coupling); fallback to nearest cardinal direction (rejected because it violates the requirement to preserve direction identity).

## Open design inputs resolved by plan

- **Pin mapping**: chưa có trong repository; coi là một artifact bắt buộc ở bước 1, không tự chọn GPIO.
- **WebSocket library**: chưa thêm ở bước đầu; chỉ chọn sau benchmark và kiểm tra tương thích PlatformIO/ESP32-S3.
- **Backend/cloud**: không triển khai trong feature này; tài liệu yêu cầu đã xác định cloud history ngoài phạm vi.
