# Build, test và chạy trên Windows

Mở PowerShell ở `D:\my_code\MyFarmEngine`. Máy cần Visual Studio có C++ compiler, CMake và Ninja. Script `Scripts/Build.ps1` tự tìm Visual Studio, chuẩn bị asset, configure/build rồi chạy test. Mặc định là Phase F Debug.

```powershell
.\Scripts\Build.ps1 -Phase phase-f -Configuration Debug
.\Scripts\Build.ps1 -Phase phase-f -Configuration Release
.\Scripts\Build.ps1 -Phase phase-f -Asan
```

Mở trực tiếp `D:\my_code\MyFarmEngine\out\build\phase-f-release\MyFarmEngine.exe` trong File Explorer để chơi, không cần CMD. Giữ thư mục `Assets` cạnh EXE. `--smoke-test` tự thoát sau vài frame; `--no-vsync` chỉ dùng khi đo hiệu năng. Nếu Windows Application Control chặn EXE, xem thông báo Security; đừng tắt bảo vệ hệ thống chỉ để chạy game.

`CMakePresets.json` đặt đường dẫn build `out/build/phase-f-debug`, `phase-f-release`, `phase-f-asan`. `MYFARM_WARNINGS_AS_ERRORS=ON` làm warning dự án thành lỗi (`/W4 /WX` trên MSVC). `phase-f-asan` build/test các target CPU; chưa kiểm tra đường OpenGL/GPU bằng sanitizer. `BUILD_TESTING=ON` bật CTest. `Scripts/Build.ps1 -SkipGraphicsTests` bỏ nhóm test cần desktop/OpenGL, hữu ích khi dùng máy không có màn hình.

Muốn chỉ chạy test logic Phase F sau khi đã build:

```powershell
ctest --test-dir out/build/phase-f-debug -R "gameplay.render_order|gameplay.foreground_trees|gameplay.collision|gameplay.visuals|gameplay.interaction" --output-on-failure
```

Khi chỉnh tán cây, collider, atlas, tốc độ hoặc world size, chạy các test liên quan trước, rồi toàn bộ `ctest --preset phase-f-debug` và `ctest --preset phase-f-release`. `TIEN_DO.txt` ghi kết quả đã kiểm chứng gần nhất; kết quả test không phải số FPS. Xem [hướng dẫn Phase F](RenderOrderAnimationInteraction.md) để hiểu hàm và thông số.
