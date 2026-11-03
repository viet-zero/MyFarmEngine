# Lộ trình đồ án MyFarmEngine

## Giai đoạn 1 — nền engine

Đã triển khai cửa sổ, input, thời gian, viewport, VSync, vòng đời game, build và test. Debug đã đạt 3/3 test. Release build và hai test engine đã đạt; còn xác minh game Release sau khi được Windows cho phép chạy. Trước buổi demo, kiểm tra thủ công ESC, nút đóng, resize và minimize/restore. Xem chi tiết tại [báo cáo giai đoạn 1](GiaiDoan1.md).

## Giai đoạn 2 — nhân vật và bản đồ nhỏ

**Cập nhật mới nhất:** nền kỹ thuật giai đoạn 2 đã hoàn thành: shader GLSL ngoài, GLM camera, PNG/alpha, sprite atlas, WASD và cảnh vườn mẫu; Debug/Release 5/5 test. [Hướng dẫn đầy đủ](../HUONG_DAN_GIAI_DOAN_2.md). Các mục camera/WASD “chưa làm” trong ghi chú cũ bên dưới đã được thay thế bởi bản này. Tilemap logic, va chạm với cảnh và animation nhiều hướng vẫn chưa có.

Cập nhật 03/11/2026: đã có shader/quad, texture PNG, canvas 2D và ảnh nền giữ tỉ lệ; thêm F1/F2 để xem tài nguyên/lịch sử. Đã có test pixel GPU và 4/4 test Debug/Release. Camera thế giới, sprite animation, nhân vật WASD và tilemap vẫn chưa được triển khai. Bước tiếp theo thực tế là camera trực giao và player di chuyển, không cần viết lại phần quad/PNG đã có.

Mục tiêu: mở game, thấy bản đồ và điều khiển được nhân vật. Thực hiện theo thứ tự:

1. **Shader và hình chữ nhật:** tạo chương trình vertex/fragment shader OpenGL 3.3; quản lý VAO/VBO/EBO, báo lỗi compile/link, vẽ một quad. Giải phóng tài nguyên trước khi context bị hủy.
2. **Texture và sprite:** thêm texture loader bằng stb_image, nạp PNG mẫu, bật alpha blending; thống nhất gốc tọa độ, đơn vị thế giới và kích thước tile. Chỉ nạp texture một lần, tái sử dụng mỗi frame.
3. **Camera 2D:** dùng GLM và ma trận chiếu trực giao; bảo toàn tỷ lệ khi resize. Chuột màn hình phải đổi được sang tọa độ thế giới nếu dùng chọn ô đất.
4. **Player:** Position/Velocity có dữ liệu thực, đọc WASD, cập nhật vị trí theo deltaTime. Chuẩn hóa vector di chuyển chéo để không chạy nhanh hơn. Giữ player trong vùng bản đồ.
5. **Tilemap nhỏ:** bản đồ ví dụ 20 x 15 ô, phân biệt ô đi được và bị chặn. Làm va chạm đơn giản với ô cản và camera bám nhân vật.
6. **Kiểm chứng:** test phép tính chuyển tọa độ/va chạm nếu có logic độc lập; chạy thử nhiều tốc độ khung hình, resize, di chuyển chéo, ESC và quay video demo ngắn.

Tiêu chí xong: clone/build được, hiện map và nhân vật; WASD hoạt động ổn định theo thời gian; resize không làm sai phép chiếu; không đi xuyên ô cản; không có GL error. Sau đó mới đánh dấu có prototype di chuyển.

Chưa cần xây ECS hoàn chỉnh, editor hay tối ưu batching ngay ở bước này. Giữ hệ thống nhỏ để hoàn thành một luồng sử dụng có thể demo.

## Giai đoạn 3 — một vòng chơi trồng trọt

1. Chọn ô đất trong tầm tương tác của player.
2. Cuốc đất, gieo hạt và tưới; có trạng thái thật cho từng ô.
3. Cây tăng trưởng theo thời gian mô phỏng do game quản lý; không dựa vào FPS.
4. Thu hoạch vào túi đồ; sử dụng và tiêu hao số lượng hạt.
5. Hiển thị công cụ, ô được chọn và số vật phẩm tối thiểu.

Tiêu chí xong: người dùng tự hoàn thành gieo -> tưới -> chờ lớn -> thu hoạch -> kiểm tra vật phẩm. Các thao tác sai trạng thái không tạo vật phẩm trùng hoặc số lượng âm.

## Giai đoạn 4 — lưu/tải và giao diện

Lưu player, đất, cây, inventory và thời gian game; kiểm tra save file thiếu/hỏng, restart và load lại. Làm menu, HUD, hướng dẫn điều khiển. Bổ sung nội dung mẫu có nguồn/quyền sử dụng rõ ràng.

Tiêu chí xong: thoát game, mở lại và tiếp tục đúng trạng thái. Có gói chạy cùng assets và hướng dẫn để máy khác dùng được.

## Giai đoạn 5 — mở rộng có chọn lọc

Chỉ thêm động vật, NPC, cửa hàng, crafting, câu cá, nhiệm vụ hoặc các vùng world khi luồng chính đã ổn định. Chọn theo yêu cầu môn học và thời gian còn lại; không xem file/header rỗng là tính năng đã xong.

## Giai đoạn 6 — hoàn thiện đồ án

Profiling Release trên cảnh cố định, sửa lỗi ưu tiên cao, thử trên máy khác, hoàn thiện tài liệu thiết kế và video demo. Ghi rõ tính năng thực hiện/giới hạn, các thư viện sử dụng, lệnh build và kết quả đo có thể lặp lại.

Bước ngay sau đợt thay đổi này: **tilemap/va chạm và sprite animation nhiều hướng**, rồi xây vòng trồng trọt của giai đoạn 3. Camera và WASD đã có trong bản giai đoạn 2.
