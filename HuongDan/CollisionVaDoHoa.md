# Phase E — bản đồ, animation và va chạm

Tài liệu này là bản hiện hành để tự làm lại ở game 2D khác. Xem `README.md` để build và `TIEN_DO.txt` để biết phần nào đã kiểm chứng. Source: `Gameplay/Components/ColliderComponent.h`, `Gameplay/Systems/CollisionSystem.*`, `Gameplay/World/WorldMap.*`, `Gameplay/World/DemoScene2D.*`.

## 1. Hình ảnh và tọa độ

`world_phase_e.png` là **một ảnh liên tục** 1536×1024, trải trên thế giới 6144×4096 (tỉ lệ 4 world units cho 1 pixel ảnh). `DemoScene2D::Render` lấy `VisibleChunks`, rồi vẽ từng phần của cùng một texture bằng UV tính từ vị trí chunk. Vì vậy không còn ranh giới chữ nhật giữa tile phẳng và ảnh vườn. Nếu ảnh thiếu, code dùng màu tile làm fallback. Texture được nạp một lần trong `Initialize`, không nạp mỗi frame.

Sprite `farmer_linh_walk.png`, `farmer_an_walk.png`, `farm_dog_walk.png`, `turnip_growth.png` có lưới 2×2, bốn frame, mỗi ô 627×627 pixel và nền alpha. `Gameplay/Data/Assets.csv` mô tả lưới; `Gameplay/Data/VisualClips.csv` đặt frame đầu, số frame, FPS và loop. `DrawActor` dùng ảnh một tư thế ở idle/action/hurt và sheet thật ở walk/run; `DrawDog` dùng frame động khi đi/chạy; `DrawCrop` chọn frame bằng **tuổi cây**, không bằng tốc độ render. Hiện các pose nhân vật/chó nhìn từ phía trước; `Facing` đã có trong logic nhưng chưa có artwork bốn hướng. Muốn có bốn hướng thật, tạo sheet riêng cho mỗi hướng, thêm record asset và clip direction `up/down/left/right`, rồi cho renderer lấy texture từ `clip->assetId`.

## 2. Collider khác sprite

`ColliderComponent::halfSize` là nửa chiều rộng/cao của **vùng chân** tính theo world units. Player dùng `{18,12}`, chó `{22,13}`, NPC `{18,12}` trong `DemoScene2D.cpp`. Ảnh người được vẽ 156×156 vì mũ/đầu cao; không dùng toàn bộ ảnh làm hitbox. Tăng halfSize làm nhân vật đứng xa vật cản hơn; giảm quá nhỏ sẽ trông như xuyên qua mép. Sửa các hằng này rồi chạy `gameplay.collision`.

`WorldMap::CanOccupy(center,collider)` nhận tâm chân và collider, trả `false` khi NaN/vô hạn, chạm biên map, tile nước, object còn active hoặc collider tĩnh. Nó xét **mọi ô tile phủ bởi AABB**, không chỉ ô chứa tâm. `CanMoveTo(point)` là wrapper tương thích cũ, với halfSize `{12,12}`; code di chuyển mới dùng `CanOccupy`.

`Gameplay/Data/StaticColliders.csv` có `id,x,y,width,height`: các hình chữ nhật của nhà, đài phun nước, hàng rào. `x,y` là góc trên trái theo world units; width/height dương và toàn hình phải nằm trong map. Chúng chỉ cản đường, không bị cuốc phá. `Objects.csv` vẫn dành cho vật thể tương tác, có `gameplay_id` và `visual_id` riêng; `SetObjectActive(id,false)` bỏ collider của object sau khi phá. Sửa CSV khi thay ảnh nền: nhân tọa độ pixel trong ảnh với 4 để có world units, rồi kiểm tra bằng F4/F5/F6. Hồ được xấp xỉ bằng ellipse trên lưới: `LakeCenterX/Y={81,19}`, `LakeRadiusX/Y={15,10}` trong `WorldMap.h`. Đây là collider gần đúng theo bờ nước vẽ tay, không phải pixel-perfect.

## 3. Di chuyển không kẹt góc

`CollisionSystem::MoveAndSlide(world,position,direction,speed,deltaTime,collider,otherActors)` trả về vị trí mới. `direction` là vector input (không cần chuẩn hóa); `speed` là world units/giây; `deltaTime` là giây/frame; `otherActors` là danh sách hitbox động như NPC/chó. Hàm chuẩn hóa đi chéo, giới hạn `deltaTime` tới 0.1 giây và `speed` tới 2000, chia bước dịch chuyển sao cho mỗi trục tối đa 8 units/bước. Mỗi bước thử X rồi Y: nếu X đụng rào mà Y trống thì vẫn đi theo Y, tức trượt dọc rào. Bước nhỏ ngăn nhảy xuyên qua rào khi frame lâu. NaN, dt âm, speed không dương hoặc vị trí ban đầu không hợp lệ trả nguyên vị trí. Ở mép map, tâm được clamp theo nửa hitbox nên sprite không kẹt vì điểm tâm chạm biên.

`WorldMap::HasLineOfSight(from,to)` lấy mẫu đường nối mỗi 8 units; trả `false` nếu gặp tile nước hoặc collider tĩnh. Khi ấn F, scene gọi `InteractionSystem::Nearest` rồi kiểm tra đường nhìn trước `Use`, nên không tương tác qua nhà/hàng rào. `InteractionSystem::Use` vẫn kiểm tra range, hướng, cooldown, stamina, công cụ, inventory và target còn active. Game hiện chưa có click lên thế giới hoặc UI con trỏ; gallery/menu tạm dừng cập nhật scene.

## 4. Hàm và thông số cần chỉnh

| Vị trí | Tham số/hàm | Công dụng |
|---|---|---|
| `WorldMap.h` | `TileSize=64`, `Width=96`, `Height=64` | Quy đổi ô và kích thước thế giới; đổi cả UV, spawn, CSV khi thay map. |
| `WorldMap.h` | `ChunkTiles=16` | Giới hạn phần ảnh được vẽ theo camera; ít chunk hơn giảm draw calls nhưng chunk rộng hơn. |
| `WorldMap.cpp` | `Spawn(zone)` | Điểm xuất hiện F4/F5/F6; đặt trên lối đi và test `CanOccupy`. |
| `WorldMap.cpp` | `LoadStaticColliders(csv)`, `LoadObjects(csv)` | Validate toàn bộ file trước khi thay danh sách hiện tại; CSV lỗi trả `false`. |
| `WorldMap.cpp` | `TryCell(x,y)`, `VisibleTiles`, `VisibleChunks`, `ChunkAt` | Tra ô an toàn và chọn phần map trong camera. Range right/bottom là exclusive. |
| `CollisionSystem.cpp` | `8.0f` | Quãng dịch chuyển tối đa một trục mỗi substep; nhỏ hơn chính xác hơn nhưng tốn CPU hơn. |
| `CollisionSystem.cpp` | `0.1f`, `2000.0f` | Trần dt/speed khi gặp input bất thường; sửa khi game có tốc độ/slow-motion khác. |
| `DemoScene2D.cpp` | `180`, `300`, `120`, `210` | Tốc độ đi/chạy của player/chó theo world units/giây. |
| `VisualClips.csv` | `fps=6/10/7/11` | Tốc độ frame walk/run của người/chó; chỉ ảnh hưởng hình, không đổi vận tốc. |
| `Gameplay/Data/Assets.csv` | `columns`, `rows`, `frameWidth/Height` | Cắt atlas thành ô; kích thước ảnh phải đúng tổng lưới hoặc AssetAudit báo lỗi. |

`Render(canvas,width,height)` chỉ vẽ, `Update(dt)` mới đổi vị trí/tuổi/clip. Muốn mang sang game khác: định đơn vị world và footprint chân → dựng tile/collider CSV → viết `CanOccupy` → `MoveAndSlide` → nối input/camera → thêm ảnh/clip → viết test ở biên, góc, dt xấu, object đã phá. Đừng lấy màu ảnh để làm nguồn collision duy nhất, vì thay art sẽ làm đổi luật chơi.

## 5. Kiểm tra

```powershell
cd D:\my_code\MyFarmEngine
.\Scripts\Build.ps1 -Phase phase-e -Configuration Debug
.\Scripts\Build.ps1 -Phase phase-e -Configuration Release
ctest --test-dir out/build/phase-e-debug -R "gameplay.world|gameplay.collision|gameplay.visuals" --output-on-failure
.\out\build\phase-e-release\MyFarmEngine.exe
```

Trong game thử WASD + Shift, đi chéo sát rào, nhà, đài phun nước và bờ ao; F4/F5/F6 kiểm tra ba điểm spawn, Tab đổi người, F tương tác. Test logic kiểm tra mép map, trượt góc, bước lớn, đi chéo, actor động, nước/nhà/rào, đường nhìn, object bị phá và dt NaN. Smoke/OpenGL chỉ chứng minh game mở và render không lỗi trong vài frame; kiểm tra cảm giác di chuyển và hình ảnh vẫn cần chơi thật. Save/load thế giới và đồng bộ online chưa có trong Phase E.
