# GAMEPLAY12 — CozyV1 và điều khiển song song

2026-11-08. Implementation vòng này DONE kỹ thuật; art acceptance tổng thể PARTIAL.

- Đã chạy game thật, phát hiện/correct scale cổng/nhà/hero, giảm nhãn luống, bỏ tint rừng từng ô và trộn shore v4 trong xuân.
- 5 source PNG mới tạo bằng built-in imagegen, giữ prompt/hash và bản nguồn Workspace; Original giữ nguyên.
- Scale CSV có validation nguyên tử, giữ aspect/root, không đổi collider theo ảnh. Thêm footprint farm.home.
- B/nút UI chuyển giữa avatar/F có range + action impact và quản lý chuột/camera. Chỉ crop trong vườn đúng chủ; farming/kho/cooldown/stamina/events dùng chung. Không replay click qua catch-up và không xuyên UI/focus.
- Native verification: gieo/tưới/thu cùng ô từ xa; túi 0/kho 1 củ cải; xem cây/nhà/hồ/zoom; không ghi slot SQL. Tests callback thực, ownership, camera, buffer, scale, pixel ellipse đều tích hợp trong 27 tests hiện có.
- Debug 27/27, Release 27/27. CheckAssets 204 Original nguyên SHA256, 189 imported/18 art cũ nguyên hash. Không benchmark FPS mới.
- Bàn giao: PROJECT_HANDOVER.md, FILE_INDEX.csv và GenerateProjectIndex.py.

Còn tiếp: town footprints/đường/rào/biome và mép collision hồ; art action/outfit/NPC; UI chuột shop/kho và feedback/balance. T chỉ là palette review, chưa ngày đêm/mùa/thời tiết. NPC AI/follow vẫn opt-in.
