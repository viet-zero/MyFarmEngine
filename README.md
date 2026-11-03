# MyFarmEngine

Engine C++20 dùng GLFW + GLAD/OpenGL cho đồ án game nông trại.

**Branch hiện tại: Gameplay10 Phase B — animation theo thực thể.** Đã có Animator an toàn, VisualCatalog theo profile/state/hướng, player/NPC/dog/crop state riêng và snapshot/rebuild timeline. Xem [báo cáo Phase B](Docs/PhaseB/README.md) và [pipeline Phase A](Docs/PhaseA/README.md). Phase C–E chưa được triển khai.

**Trạng thái ngày 03/11/2026: đã hoàn thiện nền đồ họa giai đoạn 2, phiên bản 0.2.0.**
Có OpenGL 3.3, shader GLSL ngoài, texture PNG/alpha, camera trực giao bám player và sprite atlas. Cảnh vườn mới có hai nhân vật đẹp hơn, di chuyển WASD, zoom và hotbar bốn vật phẩm. Debug và Release đều đạt 5/5 test. Va chạm với cảnh, animation nhiều hướng và cơ chế trồng cây là các bước tiếp theo.

![Giai đoạn 2 đang chạy](Docs/Screenshots/phase2_world.png)

## Build và chạy trên Windows

Cài Visual Studio có **Desktop development with C++** và **C++ CMake tools for Windows**.
GPU/driver phải hỗ trợ OpenGL 3.3 core; test đồ họa cần phiên desktop, không dùng môi trường headless.

Trong PowerShell tại thư mục repo:

```powershell
.\Scripts\Build.ps1 -Configuration Debug
.\out\build\phase-b-debug\MyFarmEngine.exe
```

Thoát bằng ESC hoặc nút đóng cửa sổ. Đổi kích thước cửa sổ để kiểm tra viewport.

| Phím | Chức năng |
|---|---|
| WASD / Shift trái | Đi / chạy trong cảnh vườn |
| Space / H | Action / Hurt animation state |
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
.\out\build\phase-b-debug\MyFarmEngine.exe --smoke-test

# Chỉ dùng khi đo hiệu năng không giới hạn bởi VSync
.\out\build\phase-b-debug\MyFarmEngine.exe --no-vsync
```

Nếu PowerShell chặn script, có thể mở Developer PowerShell của Visual Studio với CMake/Ninja trên PATH và dùng các lệnh preset trong tài liệu. Nếu Windows Application Control chặn EXE, việc build thành công không có nghĩa ứng dụng đã chạy được; không tự tắt chính sách bảo mật.

## Tài liệu

- [Phase A: kiểm kê, schema asset, pipeline, kiểm thử và giới hạn](Docs/PhaseA/README.md)

- [Phase B: Animator, VisualCatalog, state thực thể và kiểm thử](Docs/PhaseB/README.md)

- [Hướng dẫn giai đoạn 2: build, điều khiển và cách viết đồ họa](HUONG_DAN_GIAI_DOAN_2.md)

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
