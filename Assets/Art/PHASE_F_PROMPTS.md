# Phase F: prompt và xuất xứ sprite theo hướng

Tạo bằng built-in imagegen ngày 03/11/2026 từ `farmer_linh_walk.png` và `farmer_an_walk.png` của Phase E. Mỗi ảnh 1254×1254 PNG alpha, 2 cột × 2 hàng, ô 627×627. Các bản `Art`, `Original`, `Export` giữ nguyên byte; SHA256 nằm trong `Art/manifest.csv` và `Original/manifest.csv`. Không dùng Python hoặc công cụ khác để sửa pixel.

| Asset | Prompt chính | Ghi chú kiểm duyệt |
|---|---|---|
| `farmer_linh_walk_up.png` | Giữ hat/flower/braid/green overalls/bag/boots; nhìn từ sau đi lên, bốn contact/passing poses, 2×2 nền trong suốt. | Bốn bước nhìn từ sau. |
| `farmer_linh_walk_left.png` | Cùng nhân vật, full-body left profile, mặt và mũi giày sang trái, bốn bước, 2×2 alpha. | Hướng trái thật. |
| `farmer_linh_walk_right.png` | Cùng nhân vật, full-body right profile, mặt và mũi giày sang phải, bốn bước, 2×2 alpha. | Hướng phải thật. |
| `farmer_an_walk_up.png` | Giữ tóc nâu/khăn cam/yếm xanh/túi/giày; nhìn từ sau đi lên, bốn bước, 2×2 alpha. | Bốn bước nhìn từ sau. |
| `farmer_an_walk_side.png` | Cùng An, profile đi bên, bốn bước, 2×2 alpha. | Ảnh sinh quay sang phải dù prompt yêu cầu trái; renderer lật UV khi đi trái. |
| `farmer_linh_idle_dirs.png` | Bốn tư thế đứng chân chạm đất: down/left/right/up theo thứ tự 2×2, giữ thiết kế Linh. | Ô trái được lật UV vì ảnh sinh quay phải. |
| `farmer_an_idle_dirs.png` | Bốn tư thế đứng chân chạm đất: down/left/right/up theo thứ tự 2×2, giữ thiết kế An. | Ô trái được lật UV vì ảnh sinh quay phải. |
| `farmer_linh_action_dirs.png` | Bốn tư thế với tay hái quả: down/left/right/up theo thứ tự 2×2, giữ thiết kế Linh. | Mỗi hướng một tư thế; không phải clip nhiều frame. |
| `farmer_an_action_dirs.png` | Bốn tư thế với tay hái quả: down/left/right/up theo thứ tự 2×2, giữ thiết kế An. | Ô trái được lật UV vì ảnh sinh quay phải. |

Prompt chung: “cozy hand-painted storybook farming-game sprite style, same full-body scale and ground baseline in equal square cells, transparent alpha background, no scenery, labels, grid lines or borders.” Khi thay ảnh, cập nhật manifest SHA256, `Gameplay/Data/Assets.csv`, `Gameplay/Data/VisualClips.csv`, rồi chạy `Scripts/PrepareAssets.ps1`, `Scripts/CheckAssets.ps1`, `AssetAudit` và kiểm tra nhìn trực tiếp trong game.
