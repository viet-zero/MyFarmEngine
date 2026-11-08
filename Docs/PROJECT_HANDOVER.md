# MyFarmEngine — Sổ tay chỉnh tay và bàn giao

Bản chỉnh đường/rào/va chạm mới nhất: [COLLISION_VISUAL_ALIGNMENT.md](COLLISION_VISUAL_ALIGNMENT.md). F9 hiện collider thật; nhà farm hiện đặt chân (2848,2260), các lối vào được giữ thông.

Cập nhật 2026-11-08. Repository duy nhất: D:/my_code/MyFarmEngine. Đây là prototype kỹ thuật có farming thật; chưa là game hoàn chỉnh. Mục lịch sử trong PROGRESS/ROADMAP không thay thế trạng thái mới nhất ở đầu tài liệu.

## Tiến độ hiện tại

| Nhóm | Trạng thái | Đã có / phần còn thiếu |
|---|---|---|
| Build/OpenGL/console/log | DONE kỹ thuật | Debug console, Release GUI, file log LocalAppData/MyFarmEngine/Logs |
| Fixed timestep P1-003 | DONE kỹ thuật | 60 Hz, bounded catch-up, input buffer, interpolation |
| Asset pipeline/validation P1-001 | DONE kỹ thuật | Manifest/hash/metadata validation; art acceptance vẫn PARTIAL |
| Action transaction P1-002 | DONE kỹ thuật | F → IMPACT → model/FX một lần, cancel và recovery |
| Nhân vật P2-001 | PARTIAL | 8 hướng hiển thị local, 2 walk frames/hướng; thiếu action art, outfit layers và identity NPC |
| Farming P2-002B | DONE trong prototype | 3 cây, gieo lại, tưới, tăng trưởng, thu về kho trong vườn |
| Túi/kho/giao dịch | PARTIAL | Item typed/stack, chuyển kho, mua bán; UI hoàn thiện còn thiếu |
| Hai cách chơi | PARTIAL có thể dùng | Nhân vật có tầm với; quản lý vườn bằng chuột không phụ thuộc vị trí nhân vật; menu kho/shop vẫn có phím riêng |
| Map/terrain/art | PARTIAL | Lưới 64, culling, 47 mask bờ nước, 4 palette review, ảnh CozyV1; chưa chốt biome/map art |
| Save SQL Server | PARTIAL | Save/load thủ công J/L, async ODBC và codec đã tích hợp; chưa autosave/cloud/multiplayer |
| NPC/animals | PARTIAL demo | AI patrol và pet follow opt-in; chưa lịch trình, quan hệ hoặc chăn nuôi |
| Lighting/shadows/weather/audio | TODO phần gameplay | Bóng chân đơn giản; chưa hệ ánh sáng, thời tiết, âm thanh hoàn chỉnh |
| ECS tổng quát/world streaming | TODO/PARTIAL | Có thư mục khung; gameplay đang dùng model/controller cụ thể và culling, chưa streaming world |

Không quy đổi thành phần trăm toàn dự án: roadmap mở rộng chưa có tổng khối lượng cố định.

## Chạy và điều khiển

Build: mở PowerShell tại repo, chạy `./Scripts/Build.ps1 -Phase phase-f -Configuration Debug` hoặc Release. Cần Visual Studio C++/CMake/Ninja; pipeline CozyV1 cần Python + Pillow. Script stage ảnh sinh mới tự động, và stage v4 khi thư viện local đã giải nén tồn tại; không ghi vào Original. Local v4 chưa nằm trên Git: clone mới cần tự có thư viện đó để xem đúng hero/terrain v4; thiếu sẽ có cảnh báo và fallback cũ.

Bản chạy: out/build/phase-f-release/MyFarmEngine.exe. Double click để chạy GUI không console phụ. Log lỗi trong %LOCALAPPDATA%/MyFarmEngine/Logs. Không mở hai bản cùng lúc khi kiểm tra input.

- B hoặc nhấp bảng góc trái trên: chuyển nhân vật ↔ quản lý vườn.
- Nhân vật: WASD đi, Shift chạy, 1 bình tưới, 2 rìu, 3 hạt, F thao tác ô phía trước. Chỉ di chuyển theo input; NPC demo mặc định tắt.
- Quản lý: nhân vật/pet ẩn, WASD pan camera, Q/E zoom; chọn dụng cụ bằng click hotbar hoặc 1–4 rồi click ô để gieo/tưới/thu. Chỉ crop trong vườn đúng chủ được dùng từ xa. Tài nguyên/stamina/cooldown/kho dùng chung, không tự trồng ô ngoài vườn.
- G bật lưới. T đổi palette mùa review; chưa phải thời gian/mùa gameplay. Xuân có CozyV1 mới, ba palette khác còn art v4.
- K mở kho khi vị trí nhân vật đang ở nhà; F rút, C cất, 1–4 chọn hàng. M mở shop; F mua, C bán túi, K bán kho, Space chọn hạt. Menu hiện còn điều khiển bàn phím.
- J lưu, L tải SQL đã cấu hình. B và camera review là trạng thái transient, không lưu. Không nhấn J trong lượt test nếu không định ghi slot của mình.
- F9 overlay, F10 demo/gallery, F11 AI, F12 pet follow. R reset lượt chơi. NPC cần cả demo và AI; pet follow bật riêng.

## Chỗ chỉnh nhanh, không sửa engine

| Muốn đổi | File / vị trí | Cách chỉnh và áp dụng |
|---|---|---|
| Chiều cao nhân vật/cây/nhà/cổng/rào | Gameplay/Data/VisualScale.csv | human=112, tree=336, house=420, gate=80, fence=64 world units. Giữ key, số hữu hạn 16..1024; đủ 5 dòng. Build/deploy rồi khởi động lại. File sai giữ toàn bộ mặc định, có log |
| Preview tỷ lệ nhanh không compile | out/build/phase-f-release/Assets/Data/VisualScale.csv | Đổi bản deployed rồi mở lại game. Lần build kế tiếp sẽ chép lại từ Gameplay/Data |
| Vị trí/footprint vật thể | Gameplay/Data/Objects.csv | x/y góc trái collider, width/height footprint; ID gameplay khác ID visual. Render root ở đáy giữa; đừng dùng ảnh để quyết định va chạm |
| Nhà và rào cố định | Gameplay/Data/StaticColliders.csv | farm.home là footprint nhà mới. Collider town hiện thu về chân tường; xem COLLISION_VISUAL_ALIGNMENT.md |
| Cây orchard legacy | Gameplay/Data/ForegroundTrees.csv | Điểm tương tác/fruits và metadata canopy cũ. Khi LocalV4 bật, sprite whole-tree dùng root từ object footprint; canopy baked dùng khi fallback |
| Sáu luống / vị trí bắt đầu | Gameplay/World/DemoScene2D.cpp: GardenCells / WorldMap.cpp: Spawn | Chỉnh world units, đặt giữa tile 64; cần rebuild. Giữ ID crop ổn định để save còn khớp |
| Biên vườn/kho | Gameplay/Inventory/HomeStorage.h: PlayerGarden | origin và size; vùng half-open, đúng ownerId. Cả chuột lẫn harvest đều kiểm quyền |
| Thời gian lớn/giữ nước | Gameplay/Crops/Crop.cpp | Turnip 5s, carrot 8s, potato 12s, water 6s. Đây là thời gian demo; tăng để chơi chậm/chill. Rebuild |
| Giá hàng/shop | Gameplay/Systems/Trade.cpp + Inventory.cpp | Giá/loại món/stack; ItemId ổn định, không đổi ID save tùy tiện |
| Tầm với/stamina/cooldown | Gameplay/Systems/InteractionSystem.h | Nhân vật range 100, cooldown .35s, cost 2. Chuột bỏ range/facing nhưng giữ quyền và tài nguyên |
| Tốc độ đi/pan camera | DemoScene2D.cpp: MoveAndSlide / garden camera | Nhân vật 180/300; camera 400/650 world units/s, normalized direction |
| Zoom/viewport | Engine/Graphics/Camera2D.h + DemoScene2D::Reset | Zoom mặc định .85, clamp .5..2.5; ScreenToWorld dùng đúng camera đã render, gồm DPI |
| Kích thước cửa sổ | Game/GameApp/GameConfig.h | 1280×720 mặc định |
| Hotbar/anchor/icon | Gameplay/Graphics/SpriteGeometry.cpp | Layout, FitSprite, PlaceSprite. Pivot tác giả khác alpha bounds; không kéo méo ảnh |
| Chữ/màu UI | Gameplay/Graphics/HudText.h + DemoScene2D::Render | Font raster prototype, badge/hint/hotbar; đừng cho UI lấy click xuống world |
| Nền cỏ/hoa/hồ | DemoScene2D::Render + Assets/Workspace/CozyV1 | Một texture trải 8×8 tile, tint địa lý biến thiên chậm, hoa thưa và khác mật độ giữa farm/town/forest. Không tint theo random Forest cell để tránh checkerboard. Hồ dùng ellipse cùng center/radius WorldMap, bờ mềm và gợn lấy simulation time, không đổi model |

## Bản đồ file và trách nhiệm

| Nhóm file đang chạy | Trách nhiệm / điểm vào |
|---|---|
| Game/main.cpp, Game/RuntimeLog.* | Entry point, CLI flags, xử lý lỗi, log tee, Release WinMain CRT |
| Game/GameApp/Game.*, GameConfig.h | Lifecycle game và thông số cửa sổ/options |
| Engine/Core/Application.*, Window.*, Time.* | GL context, vòng lặp, fixed clock, focus/minimize/resize |
| Engine/Input/Input.* | GLFW callback cho phím/chuột, edge consume một lần, clear khi mất focus |
| Gameplay/Player/SceneInput.h | Buffer held/edge qua frame không có fixed tick; pointer position đóng băng theo lệnh click |
| Gameplay/States/GameplayState.* | Input ownership giữa gameplay/gallery, toggle debug, nối scene với Canvas |
| Gameplay/World/DemoScene2D.* | Điều phối scene thực tế: model, camera, controllers, input modes, render/UI, SQL jobs. Đây là file đầu tiên khi chỉnh trải nghiệm prototype |
| Gameplay/World/WorldMap.*, ForegroundTrees.* | Tile/zone/blocking/LOS/culling và dữ liệu object/tree |
| Gameplay/Graphics/WorldArt.* | Atlas CSV/PNG, mask bờ nước 47 loại, hướng art 8, scale config, atlas Cozy |
| Gameplay/Graphics/SpriteGeometry.*, RenderOrder.* | Tỷ lệ/pivot/bounds và Y-sort theo chân |
| Gameplay/Graphics/AssetTextures.*, VisualCatalog.* | Nạp ảnh theo manifest, cache bounds, chọn clip/controller từng entity |
| Engine/Graphics/Canvas2D.*, Texture2D.*, Shader.* | Instancing/batching, upload PNG, linear+mipmap, GLSL. Không decode ảnh/SQL trong render loop |
| Engine/Animation/* + Gameplay/Data/VisualClips.csv, VisualCoverage.csv | Timeline, marker IMPACT/COMPLETE, validation coverage; action logic không đổi position theo frame |
| Gameplay/Systems/InteractionSystem.* | Check/Start/Impact/Apply, farming, harvest và UseGarden; dùng chung mutation, không nhân đôi farming |
| Gameplay/Systems/CollisionSystem.* | MoveAndSlide với static/dynamic colliders |
| Gameplay/Crops/Crop.* | Danh mục ba crop và ánh xạ seed/produce |
| Gameplay/Inventory/Inventory.*, HomeStorage.h | Stack, capacity, atomic add/remove, quyền kho |
| Gameplay/Systems/Trade.* | Mua/bán/chuyển kho và anti-duplicate transaction |
| Gameplay/Effects/EffectPool.*, Engine/Event/* | Events model → FX, shake/floating feedback; pool hữu hạn |
| Gameplay/Save/Save.* | Codec/validation, ODBC SQL Server, atomic save; scene chạy job async |
| Gameplay/Assets/AssetManifest.*, Gameplay/Data/Assets.csv | Stable asset IDs, source/export path, frame metadata, acceptance gates |
| Assets/Shaders/* | Shader runtime. Assets/Original chỉ đọc; Workspace bản làm việc; Export ảnh deploy cũ |
| Scripts/Build.ps1, PrepareAssets.ps1, CheckAssets.ps1 | Configure/build/CTest, bảo vệ Original/hash, xác minh ảnh |
| Scripts/StageV4Assets.py | Đóng gói local v4 từ AssetLibrary_Review; thư viện nguồn này chưa lên Git |
| Scripts/StageCozyAssets.py | Đóng gói ảnh tạo mới CozyV1, padding/extrusion và provenance SHA256; không sửa source PNG |
| Scripts/SetupLocalSave.ps1 | Khởi tạo SQL Express local đã dùng; không cần tạo lại DB khi chỉnh art |
| Tests/* | Unit, integration, pixel OpenGL, fallback, lifecycle. Test chạy không thay thế duyệt hình bằng mắt |
| Docs/TASKS/*, PROGRESS.md, ROADMAP.md | Acceptance/tasks và lịch sử; trạng thái mới ở đầu PROGRESS |

Danh sách từng file và file rỗng: FILE_INDEX.csv (UTF-8 BOM, mở Excel được). Tạo lại bằng `python Scripts/GenerateProjectIndex.py`. Có file không có nghĩa tính năng đã tích hợp; chú ý file 0 byte và source không nằm trong CMake target. Thư viện local AssetLibrary_Review được để riêng vì nhiều tài nguyên nguồn, không phải tất cả đang render. ThirdParty là vendor, tránh sửa trực tiếp.

Tìm hàm đang dùng: `rg -n "CaptureInput|UpdateSimulation|Render|DrawWorldObject|DrawStaticObject" Gameplay/World/DemoScene2D.cpp`. Tìm chỗ điều chỉnh một cơ chế bằng tên key/ItemId/gameplayId, rồi xem caller và test tương ứng trước sửa.

## Quy trình sửa an toàn

1. `git status --short`, sửa file nguồn hoặc tạo bản PNG có hậu tố v2 trong Workspace; không ghi đè Original hay ảnh source v1.
2. Đổi binding/staging tới v2 trong StageCozyAssets.py; `python Scripts/StageCozyAssets.py --configuration Release` để đóng gói. Đổi LocalV4 bằng StageV4Assets.py riêng nếu cần.
3. Chạy Build.ps1 Debug/Release để deploy Data + stage Cozy + build + toàn bộ tests. Nếu dùng CMake trực tiếp, phải tự chạy stage Cozy khi ảnh thay đổi.
4. Mở executable, xem spawn, gần nhà/cây, hồ, chuyển B và thử cùng một luống bằng cả hai cách. Bật F9 xem root, G xem grid. Không coi resize hoặc filtering là sửa phong cách artwork.
5. `./Scripts/CheckAssets.ps1`, `git diff --check`, xem diff và commit đúng các file đã thử. Không git add toàn bộ thư viện review, không commit build/output/log/DB secrets.

## Art mới và giới hạn

CozyV1 được tạo bằng built-in imagegen: meadow sage nhẹ, cây táo, cottage mái ngói, hồ xanh với gợn và mảng hoa/clover. Prompt/spec và provenance trong Assets/Workspace/CozyV1. Các bản nhà có glow đã bị loại khỏi runtime. Source generated v1 giữ nguyên; atlas là bản dẫn xuất để chạy. Root đo từ alpha cho props mới vẫn là prototype, cần duyệt bằng mắt từng object; hero có root tác giả (96,180).

Chưa làm lại toàn bộ sprite nhân vật để tránh mất identity/frame consistency: vẫn dùng bộ chibi v4 cùng tỷ lệ; cần bộ hành động/outfit chuẩn riêng. Hồ xuân dùng ảnh mới với mask ellipse/bờ mềm; collision vẫn theo tile nên rìa nhìn thấy có sai khác nhỏ trong nửa ô. Shop/tường/rào/đường và một số bụi/đá còn art v4; ba palette ngoài xuân chưa đồng bộ bộ Cozy mới. Nền hồ/gợn là trình bày, chưa có vật lý nước hoặc phản chiếu. Footprint town đã thu về chân tường; layout/art town vẫn cần polish; chưa có cửa nhà/interior.

## Ba bước tiếp theo hợp lý

1. Duyệt và tinh chỉnh biome/đường/rìa hồ, footprint nhà town, roots từng cây; acceptance: không seam/hào màu rõ, đường đi khớp collision, scale hợp lý tại zoom .5/.85/1.5.
2. Chuẩn hóa bộ nhân vật tám hướng: idle/walk/action, frame root, outfit layers và NPC identity; acceptance: tất cả action marker khớp pose, không rung chân/méo ảnh.
3. Hoàn thiện thao tác chuột cho shop/kho, phản hồi ô bị từ chối và cân bằng thời gian/stamina; acceptance: không xuyên UI/ngoài quyền, cùng kết quả với điều khiển nhân vật và save/load.
