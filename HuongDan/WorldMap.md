# Phase C — hướng dẫn tự dựng WorldMap cho MyFarmEngine hoặc game 2D khác

Tài liệu này ghi lại cách Phase C hoạt động. Bản hiện hành Phase E thay phép kiểm tra điểm cuối bằng AABB, thêm static collider và ảnh toàn bản đồ: [CollisionVaDoHoa.md](CollisionVaDoHoa.md).

Tài liệu này giải thích cách Phase C hoạt động, công dụng các hàm và ý nghĩa từng thông số để bạn tự sửa hoặc làm lại. Source chính: `Gameplay/World/WorldMap.h/.cpp`, `Gameplay/World/DemoScene2D.cpp`, `Gameplay/Data/Objects.csv`, `Engine/Graphics/Camera2D.h`.

## 1. Cách các phần nối với nhau

`WorldMap` giữ dữ liệu gameplay của ô: vùng, loại nền, ô bị chặn. `WorldChunk` chia lưới thành các khối để chọn phần cần vẽ. `Camera2D` giữ tâm và kích thước vùng nhìn. `DemoScene2D` hỏi những chunk/ô nằm trong vùng nhìn rồi vẽ bằng `Canvas2D`. File `Objects.csv` giữ đối tượng với gameplay ID và visual ID độc lập. Renderer chỉ đọc dữ liệu; logic va chạm hỏi `CanMoveTo`.

Một frame: input → tính vị trí dự kiến → `CanMoveTo` → cập nhật camera → lấy `VisibleChunks`/`VisibleTiles` → vẽ các ô, đối tượng giao vùng nhìn → vẽ player/UI. `Render()` không sửa tuổi cây hoặc vị trí nhân vật.

## 2. Hệ tọa độ và thông số bản đồ

Gốc thế giới `(0,0)` nằm ở góc trên trái; X tăng sang phải, Y tăng xuống. `TileSize` là số đơn vị thế giới trên mỗi ô. Một vị trí pixel đổi thành ô bằng `floor(x/TileSize)`, `floor(y/TileSize)`.

Trong `WorldMap.h`:

| Hằng số | Giá trị | Công dụng; khi chỉnh |
|---|---:|---|
| `TileSize` | 64 | Cạnh ô, tính bằng world units/pixel ở zoom 1. Tăng số này làm ô và toàn thế giới lớn hơn; giữ tọa độ object/spawn tương ứng. |
| `ChunkTiles` | 16 | Số ô mỗi cạnh chunk; một chunk hiện là 1024×1024 world units. Đổi số này ảnh hưởng số chunk được xét, không đổi tile/collision. |
| `Width`, `Height` | 96, 64 | Số ô X/Y; kích thước thế giới là 6144×4096. Tăng kích thước làm tăng bộ nhớ lưới và số ô cần tạo lúc khởi động. |
| `TownEndX` | 32 | Ô X đầu tiên của vùng ruộng; các ô X nhỏ hơn là làng. |
| `FarmEndX` | 64 | Ô X đầu tiên của vùng hồ-rừng; từ đây tới cuối map là hồ-rừng. Phải lớn hơn `TownEndX` và nhỏ hơn `Width`. |
| `LakeCenterX/Y` | 79, 31 | Tâm hồ theo tọa độ ô, chỉ có ý nghĩa trong vùng hồ-rừng. |
| `LakeRadiusSquared` | 120 | Ô có `dx²+dy² < 120` là nước. Muốn bán kính khoảng 15 ô, dùng 225. Đây là bình phương bán kính, không phải số pixel. |

Ví dụ: muốn map 128×80 ô 48 px/ô, sửa `Width=128`, `Height=80`, `TileSize=48`; sau đó sửa ranh giới zone, vị trí spawn, tọa độ `Objects.csv`, vị trí ảnh vườn và player/NPC trong scene. Không chỉ sửa `Width/Height` rồi giữ nguyên mọi tọa độ pixel.

Trong `WorldMap.cpp`, vòng lặp khởi tạo từng ô. `x%12==0`, `y==31` tạo đường ở làng; `x%8==0`, `y%9==0` tạo đường ở ruộng; biểu thức `(x*17+y*13)%7<3` tạo mảng rừng có thể lặp lại. Đây là quy tắc mẫu, không phải bản đồ lưu từ editor. `Terrain::Water` hiện là tile bị chặn; các terrain khác đi được. Nếu muốn thêm cầu, cần đổi quy tắc blocked cho tile cầu.

`WorldMap::Size()` trả về kích thước thế giới theo pixel. `Camera2D::SetWorldSize(Size())` dùng kích thước này để clamp camera ở mép. Trong `Camera2D.h`, `ViewSize()` tính chiều cao nhìn thấy bằng `720/Zoom()` rồi nhân tỉ lệ cửa sổ cho chiều rộng. Zoom lớn hơn nhìn gần hơn. `Resize(width,height)` nhận kích thước framebuffer thực; gọi lại khi render để xử lý đổi cỡ cửa sổ.

## 3. Công dụng từng hàm public của WorldMap

| Hàm | Đầu vào | Trả về và lúc dùng |
|---|---|---|
| `WorldMap()` | Không có | Tạo lưới 96×64 ô và gán zone/terrain/blocked theo quy tắc; chưa nạp object CSV. |
| `LoadObjects(csv)` | Đường dẫn file `Objects.csv` | `true` nếu header, các số, ID và tọa độ hợp lệ. Khi lỗi trả `false`, giữ danh sách object cũ; khởi động lần đầu thì danh sách rỗng. |
| `TryCell(x,y)` | Chỉ số ô nguyên | Con trỏ `const WorldCell*`; `nullptr` nếu ra khỏi map. Dùng để đọc `zone`, `terrain`, `blocked`, không lưu con trỏ lâu qua thay đổi cấu trúc map trong tương lai. |
| `CanMoveTo(point)` | Tâm vị trí thế giới dạng `glm::vec2` | `false` nếu NaN/vô hạn, ngoài map, ở nước hoặc AABB 24×24 quanh điểm giao object `blocked=1`. Phase C mới kiểm tra vị trí cuối, chưa có sweep/sliding chuẩn Phase E. |
| `VisibleTiles(center,viewSize,padding)` | Tâm camera, kích thước vùng nhìn, số ô nới thêm (mặc định 1) | `TileRange` với `left/top` bao gồm và `right/bottom` loại trừ. Giới hạn vào map. Padding làm bớt khe hở do làm tròn ở mép. |
| `VisibleChunks(center,viewSize)` | Tâm camera và kích thước vùng nhìn | Range chỉ số chunk cần vẽ, tính từ `VisibleTiles`. Tránh duyệt toàn bộ map mỗi frame. |
| `ChunkAt(x,y)` | Chỉ số chunk | `WorldChunk` gồm tọa độ và giới hạn ô của chunk. Chỉ gọi với chỉ số từ `VisibleChunks`; hàm này chưa tự kiểm tra chỉ số sai. |
| `ZoneAt(point)` | Vị trí thế giới | Zone của ô; vị trí ngoài map trả Farm để có fallback. Trước khi dùng cho hành động gameplay, kiểm tra `TryCell` nếu cần phân biệt ngoài map. |
| `Spawn(zone)` | `Town`, `Farm`, `LakeForest` | Vị trí spawn mẫu theo world units. Sau khi chỉnh map, dùng `CanMoveTo(Spawn(zone))` để chắc chỗ spawn trống. |
| `Objects()` | Không có | Danh sách object chỉ đọc, được nạp từ CSV. |
| `Intersects(a,aSize,b,bSize)` | Góc trên trái và kích thước của hai hình chữ nhật | `true` khi AABB giao nhau; dùng culling object/ảnh và kiểm tra vật cản. Mép chạm nhau không tính là giao. |

`WorldCell` chứa `zone`, `terrain`, `blocked`; `WorldChunk` chứa chỉ số chunk và `TileRange`. `TileRange::right/bottom` là exclusive, nên vòng lặp dùng `x<right`, `y<bottom`.

## 4. Objects.csv: thêm cây, đá, bụi hoặc vật khác

Mỗi dòng: `gameplay_id,visual_id,x,y,width,height,blocked`. `x,y` là góc trên trái theo world units; width/height phải dương; `blocked` là `0` hoặc `1`. ID không được rỗng hoặc trùng; gameplay ID không được bằng visual ID. Tọa độ và toàn bộ hình phải nằm trong map. Không có dấu phẩy bên trong trường; CSV loader này chưa hỗ trợ quoted fields.

Ví dụ `tree.oak.02,visual.tree.oak,4700,1000,72,88,1` tạo một cây chặn đường. `gameplay_id` dành cho luật chơi/save về sau; `visual_id` dành cho hình vẽ và có thể đổi skin mà không đổi đối tượng gameplay. Hiện scene vẽ hình chữ nhật màu dựa trên visual ID/blocked; thêm ID mới sẽ dùng màu fallback. Khi đã có PNG sạch, thêm asset vào pipeline Phase A và ánh xạ visual ID ở lớp render, không đưa texture GPU vào dữ liệu gameplay.

Muốn thêm một loại object: tạo dòng CSV → mở rộng code vẽ `DemoScene2D::Render` hoặc lớp visual sau này → thêm quy tắc tương tác/collider khi sang Phase D/E → test object xuất hiện và bị chặn đúng. File CSV được CMake chép tới `Assets/Data/Objects.csv` cạnh EXE.

## 5. Scene, chuyển vùng và các biến hay chỉnh

`DemoScene2D::Initialize` nạp catalog animation, `Objects.csv` và ảnh. Nếu object CSV lỗi thì scene tiếp tục với object rỗng và ghi warning. `Reset` đặt player về spawn Farm, dog gần player, camera dùng `WorldMap::Size()`. `Update(dt)` nhận thời gian giây/frame, xử lý input, tính movement, hỏi `CanMoveTo`, chọn animation và cho camera đi theo. `Render(canvas,width,height)` nhận framebuffer pixel; hỏi vùng nhìn và chỉ vẽ các chunk/tile/object giao vùng nhìn. `Shutdown` bỏ con trỏ texture mượn từ asset cache.

`WASD` di chuyển, Shift chạy, `F4` tới làng, `F5` tới ruộng, `F6` tới hồ-rừng, `R` reset. Các phím F4–F6 là lối kiểm tra nhanh ba zone; muốn bỏ teleport trong game cuối, xóa ba nhánh `ConsumeKeyPress` ở `Update` và dùng cổng/chuyển cảnh thật.

Trong scene, tốc độ player là 180/300 world units/giây; dog là 120/210. `MoveWithinBounds(position,direction,speed,dt,WorldSize)` chuẩn hóa direction và clamp biên ngoài. `CanMoveTo` quyết định tile/object có được bước vào; thay `speed` không đổi collider. Mốc `paintedFarm={2300,1500}` và `paintedSize={1600,1067}` xác định vị trí ảnh vườn đã duyệt; ảnh chỉ được vẽ khi giao vùng nhìn. NPC, crop và 4 item hiện có tọa độ cố định gần đó; khi dời ảnh vườn, dời các tọa độ này cùng nhau.

`Camera2D::Follow(target,dt)` làm camera tiến gần mục tiêu theo `1-exp(-8*dt)`; số `8` càng lớn thì bám càng nhanh. Camera tự clamp sau Follow, Resize, SetZoom và SetWorldSize. Đừng cập nhật camera trong `Render()` trừ Resize; logic theo thời gian phải ở `Update()`.

## 6. Tự làm lại trong game khác

1. Chọn world units, `TileSize`, `Width`, `Height` và ranh giới zone; xác định tọa độ spawn hợp lệ.
2. Tạo `WorldCell`, `WorldMap::TryCell` và quy tắc terrain/blocked thuần CPU. Viết test biên âm, biên cuối và tile nước.
3. Thêm `WorldChunk` và `VisibleChunks` dựa trên camera; test camera ở bốn góc, cửa sổ rộng/hẹp và zoom.
4. Dùng camera để lấy `VisibleTiles`; chỉ vẽ các ô/object giao viewport. Texture lớn nếu có phải được cull trước khi draw.
5. Tách gameplay ID khỏi visual ID; loader dữ liệu validate trước khi đưa vào map. Asset thiếu dùng fallback.
6. Chạy Debug/Release với warning thành lỗi và test smoke. Sau đó đo draw calls, CPU update và thời gian GPU riêng nếu cần tối ưu.

Để mở rộng map rất lớn, Phase C hiện vẫn cấp phát cả lưới khi khởi động; chunk mới là đơn vị culling, chưa streaming/load-unload. Nếu map hàng triệu ô, cần lưu chunk thưa và nạp theo nhu cầu. Collider hiện chỉ kiểm tra tâm cuối bước, chưa giải quyết trượt cạnh/góc; phần đó thuộc Phase E. Save thế giới và multiplayer chưa có serializer; [kế hoạch online](../Docs/KeHoachOnline.md) ghi các mốc kế tiếp.

## 7. Kiểm tra lại sau khi đổi thông số

```powershell
cd D:\my_code\MyFarmEngine
.\Scripts\Build.ps1 -Phase phase-c -Configuration Debug
.\Scripts\Build.ps1 -Phase phase-c -Configuration Release
.\out\build\phase-c-release\MyFarmEngine.exe
```

Test `gameplay.world` kiểm tra ba spawn, tile nước, object chắn, camera ở hai mép, giới hạn tile/chunk. Toàn bộ suite còn có smoke OpenGL và asset fallback. Nếu đổi map size/zone boundary, sửa kỳ vọng test liên quan rồi chạy lại; không bỏ test để che lỗi.
