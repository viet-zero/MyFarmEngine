# MyFarmEngine

Engine C++20 dùng GLFW + GLAD/OpenGL cho đồ án game nông trại.

**Trạng thái ngày 03/11/2026: đã triển khai giai đoạn 1 — nền engine và vòng đời game.**
Hiện ứng dụng mở cửa sổ màu nền; chưa có nhân vật, sprite, bản đồ hoặc cơ chế trồng cây.
Kiểm chứng Debug hoàn tất; smoke test game Release còn bị Windows Application Control chặn trên máy kiểm tra.

## Build và chạy trên Windows

Cài Visual Studio có **Desktop development with C++** và **C++ CMake tools for Windows**.
GPU/driver phải hỗ trợ OpenGL 3.3 core; test đồ họa cần phiên desktop, không dùng môi trường headless.

Trong PowerShell tại thư mục repo:

```powershell
.\Scripts\Build.ps1 -Configuration Debug
.\out\build\phase1-debug\MyFarmEngine.exe
```

Thoát bằng ESC hoặc nút đóng cửa sổ. Đổi kích thước cửa sổ để kiểm tra viewport.

```powershell
# Bản tối ưu hóa và kiểm thử
.\Scripts\Build.ps1 -Configuration Release

# Chỉ test thời gian khi máy không có desktop/OpenGL
.\Scripts\Build.ps1 -Configuration Debug -SkipGraphicsTests

# Tự thoát sau 5 frame; VSync mặc định bật
.\out\build\phase1-debug\MyFarmEngine.exe --smoke-test

# Chỉ dùng khi đo hiệu năng không giới hạn bởi VSync
.\out\build\phase1-debug\MyFarmEngine.exe --no-vsync
```

Nếu PowerShell chặn script, có thể mở Developer PowerShell của Visual Studio với CMake/Ninja trên PATH và dùng các lệnh preset trong tài liệu. Nếu Windows Application Control chặn EXE, việc build thành công không có nghĩa ứng dụng đã chạy được; không tự tắt chính sách bảo mật.

## Tài liệu

- [Giai đoạn 1: thay đổi, cách làm, hiệu năng và kiểm chứng](Docs/GiaiDoan1.md)
- [Lộ trình và tiêu chí hoàn thành giai đoạn tiếp theo](Docs/LoTrinh.md)

## Cấu trúc

- `Engine/`: vòng đời ứng dụng, cửa sổ, thời gian, renderer và input.
- `Game/`: cấu hình và lớp game cụ thể.
- `Gameplay/`: trạng thái game và các khung chức năng dự kiến.
- `Tests/`: kiểm thử thời gian và vòng đời OpenGL/game.
- `Scripts/`: quy trình build/test có thể chạy lại.
- `Assets/`: hiện chưa có tài nguyên.
- `ThirdParty/`: thư viện có sẵn trong repo.

Các file gameplay rỗng và `GameInitializer.cpp` là khung có từ trước, chưa được xem là tính năng hoàn thành. `GameInitializer.cpp` chưa nằm trong target build.
