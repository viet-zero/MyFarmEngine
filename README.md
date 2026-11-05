# MyFarmEngine

**P2-002C.** Nhấn **M** mở menu mua/bán. **1–4** chọn sản phẩm, **F** mua hạt, **C** bán từ túi, **K** bán từ kho trong vườn, **Space** chọn hạt đã có. **M** đóng menu. [Tiến độ](Docs/PROGRESS.md).

**P2-002B — 05/11/2026:** plot củ cải trồng lại được, cần tưới mới lớn; thu hoạch trong vườn tự vào kho. **K** mở/đóng bảng số lượng kho trong vườn; hotbar 4 ô tự ẩn sau 4s không thao tác. Chọn **3/F** gieo, **1/F** tưới, chín thì **F** thu. Debug/Release **26/26 PASS**. Kho hiện chỉ RAM, chưa giữ qua Reset/tắt game. [Luật và giới hạn](Docs/FARMING_HOME_STORAGE.md) · [Tiến độ](Docs/PROGRESS.md). Các mốc cập nhật bên dưới là lịch sử.

**Cập nhật 05/11 — ưu tiên logic:** P2-002A đã có inventory ItemId/stack/12 ô, typed seed/crop/apple và số lượng hotbar. Debug/Release **25/25 PASS**. SQL Express local đã kết nối chỉ đọc thành công; chưa save/load. Tiếp theo crop/tưới/growth, hình ảnh bổ sung sau. [Tiến độ](Docs/PROGRESS.md) · [Inventory contract](Docs/TYPED_INVENTORY.md).

Engine C++20 dùng GLFW + GLAD/OpenGL cho đồ án game nông trại.

**Branch hiện tại: Gameplay10 Phase F — Y-sorting, animation bốn hướng và hái quả.** Hướng dẫn tự làm, công dụng hàm và biến được tập trung trong [HuongDan/](HuongDan/README.md). Multiplayer, chat chữ và thoại nằm trong [kế hoạch online](Docs/KeHoachOnline.md).

**Nền đồ họa giai đoạn 2, phiên bản 0.2.0:** OpenGL 3.3, shader GLSL ngoài, texture PNG/alpha, camera trực giao bám player và sprite atlas. Phase F sắp thứ tự người, chó, cây và vật phẩm theo Y của chân; thêm atlas idle/walk/run/action bốn hướng, collider thân cây trên ảnh nền và F hái quả. Save game và online là các bước sau.

![Giai đoạn 2 đang chạy](Docs/Screenshots/phase2_world.png)

**Tiến độ 05/11/2026:** hoàn tất cross-validator P1-001, animation impact P1-002 và fixed timestep P1-003; Debug/Release **24/24 tests PASS**. [Tiến độ hiện tại](Docs/PROGRESS.md) · [Contract và kiểm thử dữ liệu](Docs/ASSET_ANIMATION_VALIDATION.md). Game vẫn là prototype, chưa hoàn tất vertical slice.

**P1-002 — animation impact:** F bắt đầu thao tác; item/stock/FX chỉ đổi tại marker IMPACT. **C** hủy; khi mở preview hoặc mất focus game pause. [Cách hoạt động và giới hạn](Docs/ACTION_ANIMATION_EVENTS.md). Animation hiện vẫn bốn hướng và action art một pose.

**Audit artwork 05/11:** [P0-001 và evidence](Docs/P0-001_ARTWORK_PIVOT_AUDIT.md) đã hoàn tất phần audit; chưa bộ nào đủ tám hướng/outfit, P2-001 đang chờ artwork. P0-002 đã sửa 11 rows direction/flip, Debug/Release 24/24 PASS; Original giữ nguyên.

**P1-003:** gameplay 60 Hz, input không lặp qua catch-up, nội suy actor/camera; pause không fast-forward. [Contract và kiểm thử](Docs/FIXED_TIMESTEP.md).

## Gameplay11 — runtime và demo controls

Release đã dùng Windows GUI subsystem: mở trực tiếp `out/build/phase-f-release/MyFarmEngine.exe` để không có console phụ. Debug giữ console. Log nằm ở `%LOCALAPPDATA%/MyFarmEngine/Logs/`, có thể đổi bằng `MYFARM_LOG_DIR`.

Mặc định không có NPC demo/corner portraits, pet follow tắt. F9 bật overlay, F10 hiện demo actor, F11 bật patrol NPC (cần F10), F12 bật pet follow. CLI: `--demo --npc-ai --pet-follow --debug-overlay`. Tab/Space/H/F4–F8 và F1/F2/Home chỉ dùng khi F9 hoặc F10 bật. F3 trở về gameplay.

[Nguyên nhân, thay đổi và kiểm thử Gameplay11](Docs/GAMEPLAY11.md) · [Character Scale & Style Standard](Docs/CHARACTER_SCALE_STYLE_STANDARD.md). Bảng điều khiển Phase F bên dưới vẫn áp dụng với debug gate nói trên.

## Build và chạy trên Windows

Cài Visual Studio có **Desktop development with C++** và **C++ CMake tools for Windows**.
GPU/driver phải hỗ trợ OpenGL 3.3 core; test đồ họa cần phiên desktop, không dùng môi trường headless.

Trong PowerShell tại thư mục repo:

```powershell
.\Scripts\Build.ps1 -Configuration Debug
.\out\build\phase-f-debug\MyFarmEngine.exe
```

Thoát bằng ESC hoặc nút đóng cửa sổ. Đổi kích thước cửa sổ để kiểm tra viewport.

| Phím | Chức năng |
|---|---|
| WASD / Shift trái | Đi / chạy trong cảnh vườn |
| F4 / F5 / F6 | Chuyển nhanh tới làng / ruộng / hồ-rừng để kiểm tra map |
| F7 / F8 | Đến điểm hái táo / điểm thử che khuất bởi tán cây |
| Space / H | Action / Hurt animation state |
| F | Tương tác với vật phía trước; phím 2 chọn hoe, phím 3 chọn seeds |
| Q / E / Tab | Zoom ra / vào / đổi nhân vật |
| 1–4 / R | Chọn vật phẩm / đặt lại player và camera |
| F3 | Về cảnh vườn giai đoạn 2 |
| F1 | Xem 5 ảnh đại diện các nhóm tài nguyên đã nhập |
| F2 | Xem 8 ảnh lịch sử từ các giai đoạn trong archive cũ |
| Trái / phải | Đổi ảnh trong chế độ xem |
| Home | Quay về phong cảnh |
| ESC | Thoát |

177 PNG thư viện được giữ nguyên và có bản làm việc trong Assets/Workspace. Phase A đánh dấu chúng needs_review, nên F1 dùng placeholder cho đến khi ảnh được duyệt/export. F2 xem 8 ảnh lịch sử trong Export; đây là tư liệu quá khứ, không phải gameplay có thể tương tác.

```powershell
# Bản tối ưu hóa và kiểm thử
.\Scripts\Build.ps1 -Configuration Release

# Chỉ test thời gian khi máy không có desktop/OpenGL
.\Scripts\Build.ps1 -Configuration Debug -SkipGraphicsTests

# Tự thoát sau 5 frame; VSync mặc định bật
.\out\build\phase-f-debug\MyFarmEngine.exe --smoke-test

# Chỉ dùng khi đo hiệu năng không giới hạn bởi VSync
.\out\build\phase-f-debug\MyFarmEngine.exe --no-vsync
```

Nếu PowerShell chặn script, có thể mở Developer PowerShell của Visual Studio với CMake/Ninja trên PATH và dùng các lệnh preset trong tài liệu. Nếu Windows Application Control chặn EXE, việc build thành công không có nghĩa ứng dụng đã chạy được; không tự tắt chính sách bảo mật.

**Audit hiện hành 04/11/2026:** [trạng thái kiểm chứng](Docs/PROGRESS.md), [kiến trúc](Docs/ARCHITECTURE.md), [roadmap và task](Docs/ROADMAP.md). Báo cáo giai đoạn cũ bên dưới giữ làm lịch sử.

## Tài liệu

- [Phase A: kiểm kê, schema asset, pipeline, kiểm thử và giới hạn](Docs/PhaseA/README.md)

- [Phase B: Animator, VisualCatalog, state thực thể và kiểm thử](Docs/PhaseB/README.md)

- [Thư mục hướng dẫn riêng: build, WorldMap và Interaction](HuongDan/README.md)

- [Phase E: cách chỉnh bản đồ, animation, collider và từng hàm/thông số](HuongDan/CollisionVaDoHoa.md)

- [Phase F: Y-sorting, animation bốn hướng, va chạm và thu hoạch cây táo; hàm và thông số](HuongDan/RenderOrderAnimationInteraction.md)

- [Kế hoạch chơi online, chat chữ và thoại](Docs/KeHoachOnline.md)

- [Hướng dẫn giai đoạn 2: build, điều khiển và cách viết đồ họa](HuongDan/Phase2.md)

- [Giai đoạn 1: thay đổi, cách làm, hiệu năng và kiểm chứng](Docs/GiaiDoan1.md)
- [Lộ trình và tiêu chí hoàn thành giai đoạn tiếp theo](Docs/LoTrinh.md)
- [Tích hợp ảnh, cách làm, hiệu năng và kiểm chứng mới](Docs/TichHopHinhAnh.md)
- [Ảnh các giai đoạn từ chat cũ](Docs/History/README.md)
- [Tiến độ dạng text](TIEN_DO.txt)
- [Nguồn và trạng thái tài nguyên](Assets/README.md)

## Cấu trúc

- `Engine/`: vòng đời ứng dụng, cửa sổ, thời gian, renderer và input.
- `Game/`: cấu hình và lớp game cụ thể.
- `Gameplay/`: trạng thái game và các khung chức năng dự kiến.
- `Tests/`: kiểm thử thời gian và vòng đời OpenGL/game.
- `Scripts/`: quy trình build/test có thể chạy lại.
- `Assets/`: nền mới, thư viện PNG và manifest nguồn/SHA256.
- `ThirdParty/`: thư viện có sẵn trong repo.

Các file gameplay rỗng và `GameInitializer.cpp` là khung có từ trước, chưa được xem là tính năng hoàn thành. `GameInitializer.cpp` chưa nằm trong target build.
