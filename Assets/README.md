# Tài nguyên MyFarmEngine

## Nền mới đang tích hợp

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

Thư viện được giữ nguyên pixel, chưa được kiểm tra thủ công toàn bộ: một số ảnh có nền caro dính sẵn, viền cần sửa, hoặc sprite sheet chưa tách frame. Manifest gốc giữ trạng thái `needs_visual_review`/`not_sliced_into_frames`. Hiện chỉ ảnh nền mới được dùng trong runtime; các ảnh thư viện được lưu làm nguyên liệu cho các bước sau.

Nguồn của bộ SNHR là ZIP người dùng đã tạo/tải trong các chat trước, không kèm bằng chứng đầy đủ về giấy phép từng ảnh gốc. Tài liệu này không gán giấy phép CC0 hoặc quyền phân phối thương mại cho bộ SNHR; cần kiểm tra nguồn gốc trước khi phát hành game.

`Scripts/ImportChatAssets.ps1` tái hiện cách chọn file từ archive và từ chối ghi đè tài nguyên đang có. Các ZIP nguồn trong Downloads không bị sửa/xóa.
