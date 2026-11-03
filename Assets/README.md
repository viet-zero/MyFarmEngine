# Tài nguyên MyFarmEngine

## Pipeline hiện hành — Phase A

Ảnh gốc nằm trong Original (190 PNG, kiểm tra SHA256); 177 ảnh chưa duyệt có bản làm việc trong Workspace; 13 PNG hợp lệ nằm trong Export. Runtime chỉ bật record ready trong Gameplay/Data/Assets.csv sau khi kiểm tra path, decode, alpha, kích thước/grid/frame và anchor. PNG/metadata lỗi dùng fallback; F1 chưa bật lại các sprite needs_review. Các ghi chú Art/Library phía dưới mô tả kho cũ được giữ nguyên, không phải các thư mục ảnh runtime mới. Xem Docs/PhaseA/README.md để chạy pipeline.

## Bộ vẽ tay giai đoạn 2

`Art/` có hai nhân vật Linh/An, atlas bốn vật phẩm và bản đồ vườn mới tạo bằng imagegen tích hợp. Prompt được lưu ở `Art/PROMPTS.txt`, SHA256 ở `Art/manifest.csv`. Các ảnh này dùng trực tiếp trong cảnh mặc định giai đoạn 2. Nhân vật mới là một tư thế có alpha; chưa có walk cycle nhiều hướng. Xem `HUONG_DAN_GIAI_DOAN_2.md` để sử dụng camera và UV atlas.

## Nền mới đã tích hợp

`Backgrounds/cozy_farm_morning.png`: ảnh mới tạo bằng công cụ imagegen tích hợp ngày 03/11/2026, theo yêu cầu nền nông trại vẽ tay, mịn, ấm áp và thư giãn trong chat “Tạo thư mục snhr”. Prompt đầy đủ ở `Backgrounds/PROMPT.txt`. Ảnh là phong cảnh minh họa tĩnh; không phải tilemap hoặc cảnh có thể tương tác.

## Thư viện khôi phục từ Downloads

Đã đọc các chat liên quan qua công cụ đọc chat và tìm thấy bản ZIP tương ứng trên máy. Các link `sandbox:` trong chat cũ không được giả định là đường dẫn có thể đọc trực tiếp trên Windows.

| Thư mục | PNG | Nguồn |
|---|---:|---|
| Library/SNHR_v11/Characters | 18 | SNHR_Game_Assets_v11_Reclassified.zip |
| Library/SNHR_v11/Animals | 42 | SNHR_Game_Assets_v11_Reclassified.zip |
| Library/SNHR_v8/Crops_Plants | 49 | snhr_game_assets_v8.zip |
| Library/SNHR_v8/Environment | 56 | snhr_game_assets_v8.zip |
| Library/SNHR_v8/Items_Food | 12 | snhr_game_assets_v8.zip |

Tổng cộng 177 PNG thư viện, cùng hai JPG preview và metadata gốc v11. Giữ nguyên ID/tên nguồn để không đặt sai tên đối tượng. `Library/import_manifest.csv` ghi entry trong ZIP và SHA256 từng file nhập; file này còn ghi 8 ảnh lịch sử nằm trong Docs. Không nhập các bản crop trùng từ v8 hoặc code từ ZIP cập nhật cũ lên mã hiện tại.

Thư viện được giữ nguyên pixel, chưa được kiểm tra thủ công toàn bộ: một số ảnh có nền caro dính sẵn, viền cần sửa, hoặc sprite sheet chưa tách frame. Manifest gốc giữ trạng thái `needs_visual_review`/`not_sliced_into_frames`. Nền mới hiển thị mặc định; F1 xem 5 ảnh đại diện thư viện ở trạng thái gốc, F2 xem 8 ảnh giai đoạn lịch sử. Những chế độ này dùng để xem tài nguyên, chưa làm animation hoặc gameplay.

Nguồn của bộ SNHR là ZIP người dùng đã tạo/tải trong các chat trước, không kèm bằng chứng đầy đủ về giấy phép từng ảnh gốc. Tài liệu này không gán giấy phép CC0 hoặc quyền phân phối thương mại cho bộ SNHR; cần kiểm tra nguồn gốc trước khi phát hành game.

`Scripts/ImportChatAssets.ps1` tái hiện cách chọn file từ archive và từ chối ghi đè tài nguyên đang có. Các ZIP nguồn trong Downloads không bị sửa/xóa.
