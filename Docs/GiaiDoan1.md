# Báo cáo giai đoạn 1 — nền engine

## Bổ sung: sửa vùng đen khi kéo/resize cửa sổ (03/11/2026)

Windows có thể giữ luồng chính trong vòng lặp kéo/resize bên trong `glfwPollEvents`. Bản trước chỉ vẽ sau khi hàm đó trả về nên vùng cửa sổ vừa mở rộng chưa được tô lại kịp.

Đã thêm GLFW window refresh callback để vẽ lại trạng thái game hiện tại ngay khi Windows gửi yêu cầu repaint. Đường vẽ này cập nhật viewport từ framebuffer mới nhất và swap buffer, không chạy thêm Update. Chỉ đăng ký refresh callback để tránh vẽ/swap hai lần cho cùng một sự kiện resize. Chặn callback khi đang Update/Render nhằm tránh vẽ lồng nhau; tháo callback trước khi Shutdown. Reset đồng hồ sau repaint để thời gian kéo không thành bước nhảy mô phỏng. Game tạm ngừng mô phỏng trong vòng kéo cửa sổ của Windows; hỗ trợ animation liên tục khi kéo là công việc riêng nếu cần sau này.

Kiểm thử lifecycle đã bổ sung `WM_PAINT` được gửi vào hàng đợi Windows trước PollEvents, kiểm tra có repaint ngoài frame Update thông thường, viewport đúng kích thước ngay sau Update và resize không gây render lồng nhau.

**Kiểm chứng bản sửa:** Debug biên dịch thành công với cảnh báo dự án đã bật. Windows Application Control chặn cả ba EXE Debug mới build khi chạy trực tiếp; CTest báo không khởi chạy được. Vì vậy chưa xác minh runtime hoặc kéo chuột thực tế của bản sửa này. Các kết quả đạt ở bảng bên dưới thuộc phiên bản trước bản sửa repaint. Không thay đổi chính sách bảo mật để chạy test.

Sau khi EXE được máy cho phép chạy, chạy `Scripts/Build.ps1 -Configuration Debug`, rồi thử kéo cạnh trên/trái/phải, phóng to, thu nhỏ/khôi phục, đóng/ESC. Nội dung phải phủ kín client area và không để lại dải đen. Hiện game mới có màu nền nên kiểm tra này chưa chứng minh tốc độ animation/gameplay.

Ngày thực hiện: **03/11/2026**. Phiên bản CMake: **0.1.0**.

## 1. Mục tiêu và phạm vi

Đưa phần nền từ khung mã nguồn sang ứng dụng có vòng đời game được gọi thực sự, quản lý cửa sổ an toàn, thời gian có kiểm soát và build/test lặp lại được. Kết quả hiển thị vẫn là cửa sổ màu nền. Chưa triển khai sprite, nhân vật, tilemap hay nông trại.

Trước khi sửa, 60/89 file trong Engine, Game và Gameplay rỗng; 57/71 file gameplay rỗng. Đây là bằng chứng về mức triển khai ban đầu, không phải công thức tính phần trăm hoàn thành đồ án.

## 2. Các bước đã làm và lý do

### Bước 0 — giữ lại công việc có sẵn

Commit `adaadf3`: lưu nguyên các thay đổi có sẵn về Game, GameConfig, GameState/GameplayState và đường dẫn CMake. Các file state đã được chuyển khỏi thư mục lồng cũ. Giữ lại GameInitializer đang viết dở và không đưa vào build. Commit này tách công việc đã có của chủ dự án khỏi phần triển khai mới.

### Bước 1 — sửa cửa sổ và đường khởi tạo lỗi

Trong `Engine/Core/Window.cpp`, thay `if (m_Window = nullptr)` bằng phép so sánh. Phép gán cũ xóa con trỏ vừa tạo và làm mất context cần dùng để load OpenGL.

Kiểm tra kích thước/title đầu vào; log lỗi GLFW; dọn cửa sổ và GLFW nếu tạo window hoặc load GLAD thất bại. Theo dõi trạng thái khởi tạo để Destroy gọi lại an toàn. Cấm copy đối tượng sở hữu tài nguyên nhằm tránh giải phóng lặp.

Yêu cầu OpenGL 3.3 core thay vì 4.6 vì nền hiện chỉ sử dụng API trong phạm vi 3.3; kiểm tra GLAD_GL_VERSION_3_3 sau khi load. Không thay loader GLAD hoặc mã vendor. In phiên bản GL, GPU và VSync một lần lúc khởi tạo để hỗ trợ chẩn đoán.

### Bước 2 — kết nối engine và game

Thêm `Engine/Core/IGame.h` với Initialize, Update, Render, Shutdown. `MyGame::Game` triển khai interface; engine chỉ phụ thuộc interface. `main.cpp` tạo Game trước Application để game sống lâu hơn đối tượng gọi nó.

Thứ tự chạy:

```text
main tạo Game, Application
  -> tạo window/context, load GLAD, khởi tạo renderer/input
  -> Game.Initialize -> GameplayState.Enter
  -> mỗi frame:
       PollEvents -> kiểm tra đóng/ESC
       kiểm tra framebuffer; chờ sự kiện nếu thu nhỏ
       Time.Update -> Game.Update(deltaTime)
       cập nhật viewport -> BeginFrame -> Game.Render -> EndFrame
       SwapBuffers
  -> Game.Shutdown -> GameplayState.Exit
  -> bỏ input pointer -> dọn renderer -> hủy window/GLFW
```

`GameplayState` đã được gọi nhưng các hàm vẫn rỗng. Game được Shutdown khi GL context còn tồn tại, để các tài nguyên GPU tương lai có thể giải phóng đúng thứ tự. Initialize thất bại cũng gọi Shutdown của game để dọn phần đã tạo. Shutdown phải chịu được việc gọi nhiều lần; destructor đảm bảo dọn tài nguyên khi thoát scope.

Giai đoạn này hỗ trợ **một application/window đang hoạt động tại một thời điểm**. Input/Time và vòng đời GLFW chưa được thiết kế cho nhiều cửa sổ độc lập.

### Bước 3 — thời gian và input

Clock samples dùng double để tránh mất độ chính xác sớm khi chạy lâu; delta truyền vào gameplay vẫn là float. Reset ở lúc bắt đầu Run và sau khi chờ cửa sổ thu nhỏ. Giới hạn delta trong [0, 0.1] giây để giảm bước nhảy khi debug hoặc khựng frame. PollEvents trước Update để game nhận trạng thái phím mới trong frame đó. Input trả false khi chưa có window hoặc window đã bị dọn.

Đây là variable timestep có giới hạn, chưa phải fixed timestep. Việc giới hạn delta bỏ qua một phần thời gian mô phỏng khi khựng dài; không dùng trực tiếp cơ chế này làm đồng hồ ngày/đêm hoặc thời gian thực của game.

### Bước 4 — resize và hiệu năng nền

Đọc kích thước framebuffer thực tế để xử lý resize và mật độ điểm ảnh. Renderer chỉ gọi glViewport khi kích thước thay đổi. Khung hình sau sẽ bắt kịp nếu yêu cầu resize phát sinh ngay trong Game.Update.

Bật VSync bằng glfwSwapInterval(1), cấu hình qua GameConfig; `--no-vsync` cho phép chạy không chờ đồng bộ khi đo. Hiệu lực VSync còn phụ thuộc driver/cài đặt máy.

Khi framebuffer bằng 0, không Update/Render/SwapBuffers; dùng glfwWaitEventsTimeout(0.1) để tránh vòng lặp chờ bận. Không log và không cấp phát tài nguyên mới chủ động trong vòng lặp engine thông thường.

### Bước 5 — build và test

Tách MyFarmEngineCore thành static library để ứng dụng và test dùng chung mã. Tắt build ví dụ/test/tài liệu GLFW theo mặc định trên cache mới; bật cảnh báo /W4, /permissive-, /utf-8 cho mã dự án MSVC. Không sửa cảnh báo bên trong thư viện vendor.

CMakePresets có Debug và Release, build tối đa 4 tác vụ song song. Scripts/Build.ps1 tìm Visual Studio bằng vswhere, chuẩn bị môi trường compiler/CMake/Ninja và chạy configure, build, test. Build tăng dần tái sử dụng object không đổi. CMake tối thiểu 3.20 cho cấu hình thủ công; preset version 3 yêu cầu 3.21 trở lên.

## 3. Kiểm chứng thực tế

Máy kiểm tra dùng MSVC 19.51, Visual Studio 2026; OpenGL 3.3.0 NVIDIA 566.07; GPU NVIDIA GeForce RTX 3050 Laptop GPU.

| Kiểm tra | Kết quả |
|---|---|
| Debug build | Thành công, không còn cảnh báo trong lần build cuối |
| engine.time, Debug | Đạt: reset, frame thường, khựng dài, clock lùi, độ chính xác khi uptime lớn |
| engine.lifecycle, Debug | Đạt: thứ tự Update/Render, viewport sau resize, GL error, frame budget, khởi tạo lại, Shutdown lặp, dọn khi Game.Initialize thất bại |
| game.smoke, Debug | Đạt: game thật khởi tạo, chạy 5 frame và thoát |
| Toàn bộ CTest Debug | 3/3 đạt; lần chạy cuối khoảng 0.67 giây |
| Release build | Thành công, không có cảnh báo trong output build |
| engine.time và engine.lifecycle, Release | 2/2 đạt; khoảng 1.53 giây tổng thời gian test |
| game.smoke, Release | Chưa xác minh thành công: Windows Application Control chặn chạy MyFarmEngine.exe |
| git diff --check trước commit | Đạt |

Thời gian CTest ở trên là thời gian kiểm thử, **không phải FPS hoặc benchmark hiệu năng**. CTest từng báo không khởi chạy được test lifecycle; chạy trực tiếp và lần kiểm tra cuối đã thành công. Riêng game Release có thông báo chặn rõ ràng từ Windows. Không thay đổi hoặc vô hiệu hóa chính sách bảo mật để vượt chặn.

Kiểm thử tự động chưa bao phủ thao tác ESC bằng người dùng, kéo cửa sổ liên tục, thu nhỏ/khôi phục bằng chuột, driver không hỗ trợ GL 3.3 hoặc lỗi GLAD thật. Cần kiểm tra thủ công các mục đó trước buổi demo.

## 4. Cách build và kiểm tra lại

```powershell
.\Scripts\Build.ps1 -Configuration Debug
.\Scripts\Build.ps1 -Configuration Release
```

Hoặc trong Developer PowerShell đã có CMake/Ninja trên PATH:

```powershell
cmake --preset phase1-debug
cmake --build --preset phase1-debug
ctest --preset phase1-debug
.\out\build\phase1-debug\MyFarmEngine.exe
```

Test lifecycle và smoke cần desktop/OpenGL. Có thể chạy riêng test không đồ họa bằng `ctest --preset phase1-debug -LE graphics`. Để chạy lại hai test Release đã xác minh, dùng `ctest --preset phase1-release -E game.smoke`; lệnh này không kiểm tra khả năng khởi chạy game Release.

Nếu Windows chặn EXE, ghi lại lỗi và thực hiện quy trình phê duyệt ứng dụng của máy trước khi kiểm tra lại. Không đánh dấu smoke test đạt chỉ vì build thành công.

## 5. Cách đo hiệu năng khi làm tiếp

Chưa đo mức giảm CPU/GPU và chưa có số FPS đại diện cho gameplay. Những thay đổi VSync, chờ sự kiện và tránh gọi viewport thừa mới là cơ chế kiểm soát tải nền; không tuyên bố nhanh hơn một tỷ lệ cụ thể.

Quy trình đo đề xuất sau khi có cảnh game:

1. Dùng Release đã được phép chạy; cố định bản đồ, độ phân giải, nguồn điện và driver.
2. Chạy làm nóng 10 giây, đo ít nhất 60 giây, lặp lại 3 lần với cùng thao tác/cảnh.
3. Đo riêng VSync bật và tắt. Ghi CPU/GPU trong Task Manager hoặc profiler; lấy frame time bằng bộ đếm dựa trên đồng hồ không bị clamp, không suy FPS từ delta gameplay đã giới hạn.
4. Ghi frame time trung bình, p95/p99, bộ nhớ và số draw call khi renderer có sprite; giữ số liệu gốc để so sánh cùng điều kiện.
5. So sánh cửa sổ đang hiện và thu nhỏ để kiểm chứng chờ sự kiện. Lập baseline trước khi tối ưu; chỉ thêm batching hoặc cấu trúc dữ liệu phức tạp khi profiler chỉ ra nút thắt.

## 6. Các commit để tra cứu

- `adaadf3`: giữ lại khung game/state có sẵn.
- `c140b47`: triển khai nền engine, IGame và vòng đời ứng dụng.
- `2faf167`: CMake core library, presets, script build và kiểm thử.
- Commit tài liệu đi kèm: README, báo cáo này và lộ trình; hash xem bằng `git log --oneline`.

Mỗi commit có phần mô tả lý do, thay đổi và giới hạn kiểm chứng; xem đầy đủ bằng `git show <hash>`.
