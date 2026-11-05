# Implementation Plan: Phân tích kiến trúc và trạng thái dự án xe thông minh

**Branch**: `001-project-status-analysis` | **Date**: 2026-11-05 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/001-project-status-analysis/spec.md`

## Summary

Đây là kế hoạch kỹ thuật để chuyển baseline hiện tại thành firmware xe robot có điều khiển an toàn, telemetry thật và giao tiếp thời gian thực. Kế hoạch giữ giao diện web hiện có làm lớp trình bày, tách logic điều khiển/cảm biến khỏi web task, dùng một trạng thái robot trung tâm có timeout lệnh, và duy trì hợp đồng HTTP hiện tại trong giai đoạn chuyển tiếp trước khi bổ sung WebSocket. Thay đổi mới mở rộng manual control từ 4 hướng cardinal thành 8 hướng, trong đó 4 hướng chéo là command độc lập.

## Technical Context

**Language/Version**: C++ trên Arduino framework cho ESP32-S3 (phiên bản do PlatformIO quản lý)

**Primary Dependencies**: PlatformIO Espressif32, Arduino, LittleFS, WiFi/WebServer/DNSServer, ArduinoJson 7.2.x; WebSocket chỉ thêm sau khi chốt thư viện tương thích

**Storage**: LittleFS chỉ phục vụ tài sản web; không có lưu trữ telemetry dài hạn

**Testing**: PlatformIO native/unit tests cho logic thuần; integration test trên board hoặc simulator tối thiểu cho endpoint, state machine và safety behavior

**Target Platform**: ESP32-S3 DevKitC-1, mạng Wi-Fi SoftAP cục bộ, trình duyệt landscape trên thiết bị điều khiển

**Project Type**: Firmware IoT/robot điều khiển thời gian thực kèm web UI tĩnh

**Performance Goals**: Lệnh điều khiển đến phản hồi xe dưới 100 ms trong mạng SoftAP; telemetry môi trường mỗi 2–5 giây; UI không để request điều khiển chồng lấn

**Constraints**: Hoạt động không cần Internet; dừng an toàn khi mất kết nối/timeout; không coi telemetry mẫu là dữ liệu thật; tránh block web task bằng đọc cảm biến hoặc điều khiển motor; không tự ánh xạ hướng chéo sang hướng thẳng khi phần cứng không hỗ trợ

**Scale/Scope**: Một robot và một phiên điều khiển chính trong v1; 5 cảm biến line theo UI hiện tại; không bao gồm cloud history

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Constitution hiện vẫn là template chưa ratify: không có nguyên tắc hoặc gate dự án cụ thể để áp dụng. Kế hoạch tự áp dụng các gate an toàn cần thiết từ spec: không viết code trong pha plan, mọi thay đổi điều khiển phải có test dừng/timeout, và chỉ bật phần cứng sau khi contract dữ liệu/đấu nối được chốt. **PASS** với trạng thái governance hiện tại; cần ratify constitution trước khi mở rộng quy mô.

## Project Structure

### Documentation (this feature)

```text
specs/001-project-status-analysis/
├── plan.md              # This file (/speckit-plan command output)
├── research.md          # Phase 0 output (/speckit-plan command)
├── data-model.md        # Phase 1 output (/speckit-plan command)
├── quickstart.md        # Phase 1 output (/speckit-plan command)
├── contracts/           # Phase 1 output (/speckit-plan command)
└── tasks.md             # Phase 2 output (/speckit-tasks command - NOT created by /speckit-plan)
```

### Source Code (repository root)

```text
src/
├── main.cpp                 # khởi tạo task và wiring hệ thống
├── task_webapp.cpp          # HTTP/WebSocket adapter và static file serving
├── robot_controller.cpp     # state machine, command validation, timeout
├── motor_driver.cpp         # PWM/dir abstraction cho motor driver
├── sensors.cpp              # line, HC-SR04, DHT11, battery sampling
└── telemetry.cpp             # snapshot và JSON serialization
include/
├── task_webapp.h
├── robot_controller.h
├── motor_driver.h
├── sensors.h
└── telemetry.h
data/
└── index.html               # controller/monitor UI
test/
├── test_robot_controller/
├── test_telemetry/
└── test_web_contract/
```

**Structure Decision**: Giữ một PlatformIO project duy nhất. Tách các trách nhiệm bằng module C++ nhỏ trong `src/` và header tương ứng trong `include/`; không tạo backend/cloud trong phạm vi này. UI vẫn là tài sản LittleFS trong `data/`, còn test được tổ chức theo logic, serialization và contract endpoint.

## Architecture & Impact

### Files to modify

- `platformio.ini`: thêm dependency WebSocket chỉ khi Phase 4 được duyệt; cấu hình test environment và các define chân phần cứng sau khi sơ đồ đấu nối được chốt.
- `src/main.cpp`, `include/task_webapp.h`: wiring các module, khởi tạo state/controller và truyền dependency thay vì dùng trạng thái global không kiểm soát.
- `src/task_webapp.cpp`: giữ static file route; chuyển handler sang gọi controller/telemetry snapshot; chuẩn hóa HTTP status và lỗi đầu vào; bổ sung WebSocket adapter ở phase riêng.
- `data/index.html`: giữ cơ chế STOP hiện có, đổi payload/telemetry sang contract chính thức, hiển thị nhiệt độ/độ ẩm và trạng thái disconnected khi được bật.
- D-pad UI trong `data/index.html`: mở rộng từ 4 nút cardinal hiện tại thành 8 nút, dùng command identity riêng cho từng hướng chéo và giữ keyboard/pointer release safety.
- Keyboard/pointer mapping trong `data/index.html`: bổ sung tổ hợp chéo và cơ chế single-active-command để không gửi lẫn lệnh cũ khi chuyển nhanh giữa các hướng.

### Files to add

- `src/robot_controller.cpp`, `include/robot_controller.h`: state machine, command validation, last-command timestamp, safe stop.
- `src/motor_driver.cpp`, `include/motor_driver.h`: interface motor driver, mapping hướng/tốc độ sang PWM và stop.
- `src/sensors.cpp`, `include/sensors.h`: đọc và validate line/HC-SR04/DHT11/battery theo chu kỳ.
- `src/telemetry.cpp`, `include/telemetry.h`: immutable snapshot và JSON serialization.
- `test/test_robot_controller/`, `test/test_telemetry/`, `test/test_web_contract/`: test logic và contract.
- `specs/001-project-status-analysis/contracts/`: contract HTTP/WebSocket để UI và firmware cùng dùng.

### Dependency order

`motor_driver` và `sensors` là hardware adapters; `robot_controller` phụ thuộc chúng qua interface; `telemetry` đọc snapshot từ controller/sensors; `task_webapp` chỉ là adapter gọi command và trả snapshot; UI phụ thuộc contracts nhưng không được truy cập hardware trực tiếp.

Thay đổi 8-way ảnh hưởng theo chuỗi:

`data/index.html` → command contract/parser trong `task_webapp` → `ControlCommand.direction` → `RobotController` validation/state → `MotorDriver` diagonal mixing → PWM phần cứng.

Không thay đổi trực tiếp `sensors.cpp`, `telemetry.cpp` hoặc format sensor payload, ngoại trừ việc telemetry có thể báo `motion` là một trong 8 hướng.

## Technical Design

### Shared state and command flow

1. Web handler nhận `ControlCommand`, validate schema/range rồi chuyển vào controller.
2. Controller cập nhật `RobotState`, ghi `lastCommandAt`, và gọi motor adapter; command STOP luôn có ưu tiên cao nhất.
3. Một control tick định kỳ kiểm tra timeout. Nếu quá hạn hoặc mất phiên, chuyển sang `IDLE` và gọi `stopAll()`.
4. Sensor task đọc theo chu kỳ riêng, cập nhật `SensorReading`; telemetry tạo snapshot nhất quán và serialize theo [contracts/telemetry.md](./contracts/telemetry.md).
5. Web UI nhận snapshot qua polling trong giai đoạn đầu; WebSocket chỉ là adapter thay thế cùng payload, không thay đổi domain state.

### Main data types

- `ControlCommand`: `kind` (`MOVE`, `STOP`, `SET_MODE`), one of 8 directions (`FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT`, `BACKWARD_RIGHT`), speed, requested mode, received-at.
- `RobotState`: mode (`IDLE`, `MANUAL`, `LINE_TRACKING`), one of 8 motions plus `STOP`, speed, last-command time, connection/safety reason.
- `SensorReading`: line array, distance, temperature, humidity, battery voltage, sample timestamps and validity flags.
- `TelemetrySnapshot`: device id, `RobotState`, `SensorReading`, schema version.

### Diagonal motion design

- Cardinal directions retain existing compatibility symbols `F`, `B`, `L`, `R`; `S` remains STOP.
- Diagonal directions use explicit versioned values: `FORWARD_LEFT`, `FORWARD_RIGHT`, `BACKWARD_LEFT`, `BACKWARD_RIGHT`.
- `MotorDriver` owns the chassis-specific mixing from longitudinal/lateral components to wheel PWM. The controller does not know wheel pin details.
- For a chassis/driver without diagonal capability, `MotorDriver` returns an unsupported result; the controller calls `stopAll()`, preserves `motion=STOP`, and the web adapter returns a non-success error.
- A new diagonal command replaces the active command only after the previous motion is stopped or atomically superseded; release/cancel/disconnect always takes the STOP path.

### Safety and concurrency

Hardware access chỉ được thực hiện bởi controller/sensor task; web task không gọi GPIO trực tiếp. Shared snapshots phải được bảo vệ bằng copy dưới critical section/mutex hoặc queue phù hợp; tránh giữ lock trong serialization. Timeout và STOP phải idempotent. Handler không được trả `200 ok` cho command không hợp lệ.

## Constitution Check — Post-Design

**Result: PASS.** The repository constitution remains an unratified placeholder, so no project-specific gate is violated. The 8-way change remains within the existing single-project structure, adds no backend or persistent storage, keeps hardware-specific mixing behind the motor adapter, and requires explicit tests for unsupported diagonal commands and safe STOP behavior. No complexity exception is required.

## Step-by-step Implementation Plan

1. **Freeze contracts and pin map** — hoàn thiện [contracts/commands.md](./contracts/commands.md), [contracts/telemetry.md](./contracts/telemetry.md) và bảng GPIO được duyệt. Verify: contract tests bao phủ 8 giá trị direction, payload thiếu/sai kiểu và unsupported diagonal; chưa sửa firmware.
2. **Extract pure controller logic** — implement state transitions, 8-way command validation, timeout and STOP against fake motor adapter. Verify: native unit tests pass for cardinal/diagonal transitions, replacement, unsupported result and timeout.
3. **Add motor adapter** — map 8 directions/speed to chassis-specific mixing and ensure `stopAll()` drives all channels to zero. Verify: hardware bench test confirms 4 cardinal directions, 4 diagonal directions when supported, explicit rejection otherwise, STOP and no movement after timeout.
4. **Add sensor adapters and sampling** — integrate line, distance, environment and battery reads with validity/error state. Verify: raw readings and invalid/disconnected sensor cases are visible in serial diagnostics.
5. **Add telemetry snapshot/serialization** — replace fixed JSON with validated snapshot and schema version. Verify: JSON contract tests validate fields, units, line count and invalid-value representation.
6. **Integrate web handlers** — wire `/api/cmd`, `/api/linetrace`, `/api/telemetry` to domain services; return 4xx for invalid input and preserve safe STOP behavior. Verify: endpoint integration tests and browser smoke test.
7. **Update UI monitor/control** — consume official payload, add environment fields and explicit disconnected state, retain stop-on-release/background/tab change. Verify: UI smoke tests cover command release, line mode, STOP and telemetry fallback.
8. **Evaluate/enable WebSocket** — benchmark current HTTP path, then add a WebSocket adapter only if the measured control latency misses the target. Verify: local SoftAP p95 command latency under 100 ms and reconnect causes safe stop.
9. **Add regression and deployment documentation** — update root README, wiring/config guide and PlatformIO test instructions. Verify: clean build, filesystem upload, test suite and quickstart all pass.

## Test Plan

### Unit tests

- Command validation: accepted 8 directions, speed bounds, mode transitions, malformed/unknown commands, unsupported diagonal handling and active-command replacement.
- Safety: STOP idempotency, command timeout, session loss, line tracing disabling manual motion.
- Telemetry: snapshot consistency, units, validity flags, JSON field presence and line sensor cardinality.
- Sensor processing: line normalization, distance obstacle threshold, DHT11 invalid reads, battery conversion.
- Motor mapping: cardinal/diagonal direction-PWM output via fake GPIO, unsupported diagonal error and guaranteed zero output on stop.

### Integration tests

- Firmware startup: web task, controller and sensor task initialize without blocking; LittleFS failure is reported.
- HTTP contract: index serving, valid/invalid command responses, telemetry schema and not-found behavior.
- End-to-end safety: browser command -> controller -> fake motor; release/background/network timeout -> stop.
- Sensor-to-telemetry: injected readings appear in the web response at the expected sampling interval.
- WebSocket (if enabled): connect, command, telemetry, reconnect and safe stop.

### Hardware acceptance

- Verify GPIO mapping against the approved wiring, each cardinal direction, each supported diagonal direction or safe rejection path, obstacle stop and line-following recovery.
- Measure control latency on the actual SoftAP and confirm telemetry interval 2–5 seconds.
- Disconnect client/network while moving and confirm robot enters IDLE/stops within the configured timeout.
