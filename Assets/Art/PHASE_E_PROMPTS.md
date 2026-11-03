# Nguồn ảnh Phase E

Tạo bằng **built-in imagegen**, ngày 03/11/2026. PNG trong `Art/` được chép nguyên byte sang `Original/` và `Export/`; SHA256 ở `Art/manifest.csv` và `Original/manifest.csv`. Không dùng Python để sửa ảnh. Các ảnh này là artwork mới; thư viện SNHR cũ vẫn giữ `needs_review`.

1. `World/world_phase_e.png`: “Continuous 3:2 top-down hand-painted cozy farming landscape, left village, center farm, right lake and forest connected by winding paths, matching the existing garden_world painting style. No rectangular scene borders, no HUD, text or player.” 1536×1024, nền đặc.
2. `Characters/farmer_linh_walk.png`: “Preserve farmer Linh's original hat, face and clothes. Exactly four distinct full-body front-facing walking contact/passing poses in a strict 2×2 grid, same scale and baseline, transparent alpha, no grid or text.” 1254×1254.
3. `Characters/farmer_an_walk.png`: “Preserve farmer An's original identity and costume. Exactly four distinct full-body front-facing walking contact/passing poses in a strict 2×2 grid, same scale and baseline, transparent alpha, no grid or text.” 1254×1254.
4. `Characters/farm_dog_walk.png`: “Small golden-brown floppy-eared farm dog, warm hand-painted storybook art, exactly four distinct 3/4 front-view walk poses in a strict 2×2 transparent grid, same identity and ground baseline.” Bản đầu có canvas 2:1, dùng imagegen edit với prompt “Preserve the four poses and make a square 1:1 transparent 2×2 atlas with equal cells.” Bản xuất dùng 1254×1254.
5. `World/turnip_growth.png`: “Four stages of the same turnip: small sprout, young leaves, maturing bulb, ripe white-purple bulb; strict 2×2 equal cells, square transparent canvas, same soil baseline, cozy painted farming style, no labels.” 1254×1254.

Prompt mô tả ở trên đủ để tái tạo quy trình nhưng kết quả sinh mới sẽ không giống từng pixel. Nếu thay ảnh, phải tính lại SHA256, kích thước/grid trong `Gameplay/Data/Assets.csv`, chạy `Scripts/PrepareAssets.ps1` rồi `Scripts/CheckAssets.ps1` và `AssetAudit`.
