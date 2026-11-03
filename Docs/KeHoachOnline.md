# Kế hoạch chơi online nhiều người, chat chữ và thoại

Mục tiêu sản phẩm: nhiều người cùng vào một thế giới nông trại, nhìn thấy và tương tác với nhau, nhắn tin trong phòng và nói chuyện bằng giọng nói. Đây là lộ trình sau Phase C, chưa phải tính năng đã triển khai.

## Thứ tự triển khai

1. **Sau Phase D/E — chuẩn hóa luật chơi và save:** hoàn thiện thao tác, va chạm, ID ổn định cho object/player, serialization có version. Chỉ server hoặc chủ phòng có quyền quyết định thay đổi inventory, crop, vật thể và thời gian thế giới.
2. **Mốc online 1 — kết nối và phòng chơi:** tạo/join phòng, danh sách người chơi, tên hiển thị, spawn và rời phòng. Bắt đầu với một phòng nhỏ; giới hạn số người cấu hình được và test mất kết nối.
3. **Mốc online 2 — đồng bộ gameplay:** gửi input/hành động lên server, server xác nhận vị trí và trạng thái thế giới, client nội suy player khác. Có snapshot lúc mới vào, cập nhật theo nhịp cố định, phát hiện lệch và đồng bộ lại. Chỉ gửi chunk/đối tượng gần người chơi khi map lớn.
4. **Mốc online 3 — chat chữ:** kênh phòng và tin nhắn gần người chơi; kiểm tra độ dài, tốc độ gửi, UTF-8, chặn spam, mute/block và báo cáo. Tin nhắn không được dùng để thực thi lệnh game.
5. **Mốc online 4 — thoại:** bật/tắt micro, chọn thiết bị, push-to-talk hoặc voice activity, điều chỉnh âm lượng từng người, mute/block. Người dùng phải chủ động bật micro; hiển thị rõ ai đang nói. Kiểm tra độ trễ, mất gói và quyền micro.
6. **Mốc vận hành:** xác thực/phiên, khôi phục kết nối, log lỗi không chứa nội dung thoại, giới hạn tài nguyên phòng, thử nghiệm mạng yếu và nhiều người. Chọn transport/hosting sau khi đo nhu cầu thực tế và số người mỗi phòng.

## Tác động tới source hiện tại

- `WorldMap` có zone/chunk và gameplay ID tách visual ID; đó là nền để gửi state có ID ổn định, lọc theo vùng nhìn và dựng lại object khi client vào phòng.
- `Animator` là presentation state; có thể suy từ velocity/hành động nhận qua mạng. Không gửi texture hoặc con trỏ GPU qua mạng.
- `Gameplay/Save` và hệ thống interaction hiện chưa hoàn thiện. Cần định nghĩa dữ liệu version và quyền server trước khi đồng bộ túi đồ/cây trồng.
- Chat và thoại cần module riêng với vòng đời kết nối riêng; không nhét vào render loop hoặc CSV asset.

## Tiêu chí kiểm thử cho từng mốc

Hai client vào/ra phòng, reconnect, player di chuyển/chuyển zone, cập nhật crop/object nhất quán, thao tác đồng thời không nhân đôi item, chat Unicode/rate limit, mute/block, micro bị từ chối, packet loss/latency. Sau đó đo băng thông, CPU server, độ trễ input/chat/voice với số người mục tiêu. Chưa tuyên bố game online cho đến khi các mốc này có build và test thực tế.
