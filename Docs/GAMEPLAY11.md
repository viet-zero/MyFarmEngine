# Gameplay11 — Runtime, character consistency và demo behavior

Ngày 2026-11-04, repo D:/my_code/MyFarmEngine. Phạm vi A–F kỹ thuật đã triển khai và kiểm thử; artwork vẫn cần review, animation vẫn bốn hướng.

## Nguyên nhân và sửa chữa

| Vấn đề | Nguyên nhân thực tế | Thay đổi |
| --- | --- | --- |
| Console phụ | add_executable dùng console subsystem cho mọi configuration | WIN32_EXECUTABLE chỉ Release; WinMain gọi Run với argv của CRT, không ép /ENTRY. Debug giữ main/console |
| Mất log khi GUI | stdout/stderr có thể không có console | RuntimeLog tee ra file theo PID, vẫn giữ pipe/console; fatal init/exception có MessageBox ở Release |
| Nhân vật không đều | Quad 156×156 cố định, pivot lấy portrait cho cả sheet, transparent margins/poses khác nhau | Cached alpha bounds, shared HumanHeight, uniform scale/aspect và ground pivot. Culling dùng cùng placement, Y-sort vẫn feet |
| NPC tự đổi pose | fmod(m_NpcTime) chọn walk/action dù không dịch chuyển | Xóa timer showcase khỏi update thường; demo actor chỉ hiện khi demoMode; npcAI bật mới patrol thực, idle nếu không đi được |
| Pet tự đi/ngủ méo | Follow luôn bật, timer chọn sleep rồi ép chiều cao ảnh | Follow opt-in; state theo dịch chuyển thật, không squeeze/sleep giả |
| Portrait góc trái | Draw UI luôn vẽ hai portrait | Chỉ vẽ khi debugOverlay, mặc định off |
| Gallery lẫn gameplay | F1/F2/Home luôn truy cập được | Chỉ ở demo/debug; tắt debug/demo cuối cùng tự về gameplay |
| UI thô | Slot/padding/icon theo công thức rời rạc | HotbarLayout chung và aspect-preserving icon fit, panel/viền/highlight/meter thống nhất |
| Độ sắc nét | Map source ×4 và sprite downscale, texture không mipmap | Thêm mipmap minification; phần thiếu chi tiết/perspective của nguồn ghi NEEDS_ART_REVIEW, không phóng ảnh giả chi tiết |
| State/input | NPC timer độc lập vị trí; pending key có thể tồn khi mất focus | Walk chỉ khi move-and-slide trả vị trí mới; mỗi entity controller riêng; khóa facing trong one-shot, reset controllers thật; focus-loss clear key events |

NPC demo mặc định ở (2360,1950), gần farm spawn để dễ bật xem. Patrol demo đi giữa x2360 và x2520, có collision; chưa phải production NPC AI/schedule. Hidden NPC không để lại dynamic collider trong scene. Pet vẫn là vật thể trong world khi follow off. Reset/teleport debug chủ động đặt lại pet cạnh người, không phải autonomous follow.

## Điều khiển và cấu hình

Mặc định demo=off, npcAI=off, petFollow=off, overlay=off. F9 overlay; F10 demo actor; F11 NPC patrol enable; F12 pet follow. NPC cần đồng thời demo và npcAI. CLI tương đương: --demo --npc-ai --pet-follow --debug-overlay. Giữ --smoke-test/--no-vsync. Options đi GameConfig → Game → GameplayState → DemoScene2D.

WASD/Shift điều khiển player, Q/E zoom, F tương tác, 1–4 chọn item, R reset. Tab/Space/H/F4–F8 là developer actions, cần F9 hoặc F10 bật. F1/F2/Home preview cũng gated; F3 về gameplay. Không có input replay hoặc movement timer cho player.

## Launch không console

Build: Scripts/Build.ps1 -Phase phase-f -Configuration Release. Sau đó mở trực tiếp out/build/phase-f-release/MyFarmEngine.exe trong File Explorer. Đây là Windows GUI executable, không tạo console phụ. Build.ps1 vẫn cần terminal để build/test; terminal của script/PowerShell/CMD đang mở không bị game tự đóng hay ẩn.

PowerShell có thể dùng Start-Process -FilePath .\out\build\phase-f-release\MyFarmEngine.exe. Để đợi smoke exit và đọc mã trả về, dùng Start-Process ... -ArgumentList --smoke-test -Wait -PassThru hoặc pipeline & executable --smoke-test | Out-Host. Đọc LASTEXITCODE ngay sau GUI launch bất đồng bộ có thể đọc giá trị cũ/null; lần kiểm tra đầu gặp lỗi harness này, log sau đó xác nhận game vẫn shutdown bình thường; đã chạy lại có wait.

Logs Windows: %LOCALAPPDATA%/MyFarmEngine/Logs/MyFarmEngine-<PID>.log, append theo PID. MYFARM_LOG_DIR có thể đổi nơi ghi. Stdout/stderr pipe vẫn có thông tin để CTest/fallback tools đọc. Nếu không mở được log, báo lỗi và dừng; fatal GUI hiện MessageBox. Không tắt/chỉnh chính sách bảo mật Windows.

## Kết quả kiểm chứng

- Debug: Scripts/Build.ps1 -Phase phase-f -Configuration Debug, build thành công, 19/19 CTest PASS; lượt cuối 14.15 giây.
- Release: cùng script -Configuration Release, build thành công, 19/19 PASS; lượt cuối 7.34 giây.
- gameplay.runtime dùng scene/renderer thật với input data được inject, không dùng bản logic giả: default đứng yên, no autoplay, NPC/pet enable/disable, blocked-input idle, controller độc lập, developer gate, invalid dt, overlay pixel regression, gallery keys và clear input khi mất focus.
- gameplay.sprite_geometry: alpha frame isolation, aspect ratio, ground/mirror placement, icon fit và toolbar trong 1280×720, 640×360, 200×100. gameplay.render_order/foreground_trees/canvas giữ PASS.
- game.windows_launch đọc PE subsystem Debug=3, Release=2; chạy native Start-Process --smoke-test và invalid CLI, kiểm exit, file và redirected stdout/stderr, Release Console attached: no.
- Chạy Release qua PowerShell và cmd /d /c có wait, exit 0/Normal shutdown, Console attached: no.
- Mở Release trực tiếp bằng computer-use, quan sát cảnh mặc định không corner portraits/demo NPC; F9 overlay có/tắt được; F10 hiện NPC; nhiều snapshot AI off giữ idle; F11 patrol đổi vị trí và walk, tắt về idle; F8 rồi F12 cho thấy pet tiến lại gần; F7/F8 kiểm front/back canopy. Đã đóng game bằng Escape.
- Key tap WASD từ công cụ desktop quá ngắn để kết luận held-input thủ công; di chuyển player/release/blocked state được xác minh bằng integration test cùng đường update thật. Chưa chơi thủ công toàn loop ở nhiều render rates; không nhận PASS cho phần chưa làm.
- CheckAssets: 189 imported files, 18 artwork và 204 Original đúng SHA256 sau sửa. Không đổi PNG/Original/CSV.
- Audit 14 character/animal textures, 50 frames; một portrait Linh chạm mép ảnh. Xem CHARACTER_SCALE_STYLE_STANDARD.

Cảnh nền còn soft khi zoom gần; props baked/collider coverage/ellipse mask chưa hoàn chỉnh. Không coi cây chặt hoặc lighting động đã xong. Không đo FPS/GPU frame time/VRAM; mipmaps có chi phí bộ nhớ, chưa có benchmark. Release/Debug đã kiểm trên MSVC/NVIDIA hiện tại; toolchain khác chưa kiểm.

## File thay đổi

CMakeLists.txt; Game/main.cpp, RuntimeLog.h/.cpp, GameApp/Game.h, GameConfig.h; Engine/Input/Input.cpp, Graphics/Texture2D.cpp; Gameplay/Graphics/AssetTextures.h/.cpp, SpriteGeometry.h/.cpp; Gameplay/States/GameplayState.h/.cpp; Gameplay/World/DemoScene2D.h/.cpp, SceneOptions.h; Tests/GameplayRuntimeTests.cpp, SpriteGeometryTests.cpp, WindowsLaunchTests.ps1; tài liệu/README liên quan. Không thêm renderer/ECS/save system mới.

Tiếp theo: duyệt anatomy/pivots/source detail, sau đó cross-validator và impact events. Tám hướng nằm ngoài Gameplay11.
