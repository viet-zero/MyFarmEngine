# MyFarmEngine — Audit có kiểm chứng

## P2-002A — Inventory thật, logic-first — 2026-11-05

**DONE kỹ thuật trong phạm vi inventory core.** InteractionActor sở hữu một bag 12 ô với stable ItemIds, stack limits, add/remove all-or-nothing. Có watering can x1, axe x1, turnip seeds x3 ban đầu; tools max-stack 1. Gieo chỉ trừ seed.turnip tại impact; thu nhận crop.turnip, orchard nhận fruit.apple; pickup dùng item ID. Hotbar kiểm ItemId và tool ownership, số lượng lấy trực tiếp từ bag. Không còn writable inventoryCount/inventoryCapacity; SceneSnapshot.inventoryCount chỉ là resource count suy ra, không tính tools.

**Debug 25/25 PASS (22.40s), Release 25/25 PASS (10.97s)** trên bản cuối. Test mới gameplay.inventory kiểm full bag/matching stacks, atomic failure, IDs/amount invalid, apples không làm seed, tool mất hoặc binding/reward đổi trước impact. Live scene kiểm starter/apple/crop typed quantities; action replay, fixed-rate 30/60/144, graphics/smoke/Windows launch regression đều pass. Không có manual extended play/duyệt pixel UI số trong lượt này. 204 Original và imported/art SHA256 giữ nguyên.

**SQL máy người dùng đã kiểm thực tế:** MSSQL$SQLEXPRESS chạy; Windows Authentication connect .\SQLEXPRESS/master và SELECT SERVERPROPERTY thành công: Express Edition 64-bit, 17.0.1000.7. Driver ODBC 17/18 có trên máy. Chỉ probe .NET đọc thông tin, chưa C++ SQL integration, chưa tạo database/tables hoặc save/load; không đổi network/service. [Contract và SQL evidence](TYPED_INVENTORY.md).

P2-002 tổng thể **PARTIAL**; chưa watering/ba crop/money/buy-sell/bag-grid UI. Existing icons giữ nguyên (seed slot vẫn placeholder icon cũ), cake/watering/wood definitions không đồng nghĩa đã có use/drop. P2-001 chờ artwork của riêng task đó, không chặn logic nữa. Tiếp theo [P2-002B](TASKS/P2-002B.md) plot tái sử dụng + crop IDs/tưới/growth → giao dịch tiền/mua/bán → persistence pilot có roundtrip/rollback trên SQL đã cài khi cần. Không triển khai cả roadmap trong một lượt.

## Cập nhật P0-002 — 2026-11-05

**DONE kỹ thuật:** sửa đúng 11 fields flip_x trong 62 clips, không đổi frame/timing/asset IDs. Idle trái Linh/An (cả NPC) và action trái An không mirror nữa; side walk/run An dùng left không mirror, right mirror. Regression kiểm các clip đã review và walk → idle giữ hướng; chạy test mới với catalog trước sửa trả exit 1 đúng lỗi.

**Debug 24/24 PASS (23.17s), Release 24/24 PASS (11.36s)**, gồm graphics/runtime/geometry/fixed-rate/animation/smoke/Windows launch. Đã xem clip board sau sửa ở scale chuẩn; đây là contact sheet, không có manual gameplay soak mới. 204 Original và imported/art hashes khớp; không sửa PNG, Assets.csv hoặc engine/renderer.

[P0-002 và ảnh trước/sau](TASKS/P0-002.md). P0-001 vẫn PARTIAL do authored pivots/artwork/canopy mask; P2-001 vẫn BLOCKED artwork acceptance. An walk phải còn mirror phụ kiện; sửa flip không tạo art tám hướng hoặc layers. Các findings CSV P0-001 giữ nguyên làm evidence lịch sử, không phải danh sách lỗi còn mở.

Tiếp theo: chốt Linh model/reference stature/root → bàn giao và duyệt diagonals + body/hair/clothes/outfit layers → triển khai loader/controller P2-001 với artwork đủ điều kiện. Không tính prototype là MVP hoàn chỉnh. Các mốc dưới là lịch sử.

## Cập nhật P0-001 artwork/pivots — 2026-11-05

**Audit DONE; P0-001 tổng thể PARTIAL.** Đã xem/đo 31 character records + dog: 32 inputs, 68 declared frames; có bảng từng asset và ảnh sáng/tối. Linh là mẫu painted chính đề xuất, An mẫu phụ; chưa asset nào đạt tám hướng/outfit. Runtime vẫn composite bốn hướng, không có authored pivots hoặc layers. Library 0001–0012 sai style; 0055–0060 alpha 255 toàn ảnh, caro baked và nhiều pose cắt dính. Giữ needs_review.

Phát hiện 11 rows direction/flip sai (idle-left Linh/An, action-left An, walk/run side An); chưa sửa CSV trong audit, chuyển [P0-002](TASKS/P0-002.md). Auto pivot action đổi X khoảng 16 world units giữa hướng; không gọi đây là đo jitter động. Canopy vẫn ellipse sample nền, cần RGBA masks.

**Debug 24/24 PASS (21.30s)**; đã chạy graphics/smoke, chưa có manual animation soak hoặc Release mới trong lượt này. 204 Original và hash imported/art khớp, không sửa ảnh/asset CSV/runtime. [Báo cáo + evidence](P0-001_ARTWORK_PIVOT_AUDIT.md).

**P2-001 BLOCKED về artwork acceptance**, code vẫn TODO; Gameplay11/P1-001/002/003 đã xong kỹ thuật. Ba việc tiếp: sửa metadata P0-002 → chốt Linh model/root và bàn giao art tám hướng/layers → triển khai loader/controller outfit khi đủ art. MVP/farm/SQLite/lighting vẫn chưa hoàn tất. Các mốc bên dưới là lịch sử.

## Cập nhật P1-003 — 2026-11-05

**DONE kỹ thuật:** simulation 60 Hz tách khỏi render; input được lấy một lần/frame, giữ tap qua frame chưa có tick và chỉ consume một lần qua catch-up. Nội suy chân player/NPC/pet và camera dùng cùng vị trí cho draw/culling/Y-sort. Animation/impact chỉ tiến trong fixed update.

**Debug 24/24 PASS (22.51s), Release 24/24 PASS (11.19s)** qua Scripts/Build.ps1, gồm graphics, executable smoke, Windows launch, native minimize/restore và repaint. Trace gameplay 6 giây ở nhịp render giả lập 30/60/144 Hz đều có 360 ticks, cùng vị trí/state/animation và hai lần hái táo; render không đổi model hoặc phát impact. Không phải benchmark FPS; chưa có phiên chơi thủ công kéo dài trong lượt này. 204 Original khớp SHA256, ảnh và dữ liệu asset giữ nguyên.

Tiến độ hiện tại: Gameplay11 và P1-001/002/003 DONE trong phạm vi kỹ thuật đã kiểm. Game vẫn là prototype bốn hướng, action artwork một pose; inventory bộ đếm, một turnip plot và orchard. P0 art review PARTIAL; tám hướng/outfit, ba crop/tưới/bán, NPC/shop hoàn chỉnh, SQLite và day/night/lighting chưa xong. Không coi P1 hoàn tất là MVP hoàn tất.

Ba việc tiếp: P0-001 duyệt artwork/pivots → P2-001 chuẩn bị đủ art tám hướng/layers rồi triển khai controller → P2-002 typed inventory và vòng farm. Chi tiết giới hạn thời gian/pause/interpolation: [Fixed timestep](FIXED_TIMESTEP.md). Các cập nhật bên dưới là lịch sử.

## Cập nhật P1-002 — 2026-11-05

**DONE kỹ thuật:** animation markers và action transaction đã tích hợp vào luồng F thật. Start chỉ reserve; IMPACT mới kiểm tra lại target ID, tool/state, LOS/range/facing và tài nguyên rồi đổi inventory/stock/health và phát FX cùng actionId. Stable token chặn bấm dồn, copied action và timeline rewind cộng đồ hai lần. C hủy; hurt/teleport/reset/shutdown cancel; preview hoặc mất focus pause.

**Debug 22/22 PASS**, **Release 22/22 PASS** ở bản cuối (19.67s / 9.95s). Test mới gameplay.action_transaction; mở rộng animator/visuals/runtime để kiểm marker boundary/loop/dt/restore/copy, mục tiêu mất hoặc đổi state, hết seed/stamina/stock, full inventory/event queue; đi tuyến farm thật để gieo/chờ/thu và nhặt item. Graphics/smoke/console regression đã chạy. Không có phiên chơi thủ công mới trong lượt này.

Ảnh/các CSV đang dùng giữ nguyên; 204 Original khớp SHA256. CSV cũ tự có midpoint/end markers cho action; artwork hành động vẫn một pose/hướng, không nhận là art hoàn thiện. Inventory vẫn bộ đếm; tám hướng/outfit, fixed timestep, production NPC/shop/door, lighting/day-night/SQLite chưa xong.

Chi tiết: [Action & Animation Events](ACTION_ANIMATION_EVENTS.md). **Task code tiếp: P1-003 fixed-step simulation**. P0 art review vẫn PARTIAL. Các báo cáo P1-001/Gameplay11 dưới đây là mốc lịch sử.

## Cập nhật P1-001 — 2026-11-05

**DONE:** cross-validator asset–animation tích hợp AssetAudit/CTest; 62 clips được đối chiếu với 204 assets và 26 khai báo profile/state trong VisualCoverage.csv version 1. Bắt unknown/not-ready reference, kind, PNG/grid/frame window, missing directions và policy/version lỗi. Không dùng Any fallback để giả đủ hướng.

Debug **21/21 PASS** (24.13s), Release **21/21 PASS** (10.15s); bộ cross-validation có 97 checks/0 failures, CLI negative test trả exit 1 đúng diagnostic. 204 Original vẫn khớp SHA256; không sửa ảnh/Assets.csv/VisualClips.csv. Chi tiết: [báo cáo validation](ASSET_ANIMATION_VALIDATION.md).

Tiến độ hiện tại: engine/runtime cơ bản và Gameplay11 đã hoạt động; kiểm tra dữ liệu P1-001 xong. MVP chưa hoàn tất: animation bốn hướng, farming/inventory/NPC/animal còn prototype; tám hướng/outfit, impact events, vòng farm ba crop/tưới/bán, shop/door, SQLite, day/night/lighting chưa xong. Art review P0-001 còn PARTIAL. **Task code tiếp theo: P1-002 animation impact**, rồi fixed timestep.

Các kết quả Gameplay11 bên dưới là lịch sử ngày 04/11; không coi DONE kỹ thuật là game đã hoàn chỉnh.

## Cập nhật Gameplay11 — 2026-11-04

Đã sửa console Release, scale/ground placement, demo autoplay, pet follow, input ownership, portrait overlay và toolbar. Debug/Release đều build thành công, **19/19 tests PASS** (14.15s / 7.34s); bản Release mở trực tiếp và log Console attached: no, smoke PowerShell/CMD exit 0. NPC/pet/debug mặc định tắt, F9/F10/F11/F12 bật riêng; player/NPC/pet controller độc lập và walk theo dịch chuyển thật.

Đo 50 frames của 14 human/animal textures; 1 frame portrait Linh chạm mép trên, artwork NEEDS_ART_REVIEW. 204 Original vẫn khớp hash. Animation vẫn bốn hướng; chưa impact events/fixed-step/SQLite/day-night. UI là hotbar, chưa inventory hoàn chỉnh. Không đo FPS.

Chi tiết nguyên nhân, file và evidence: [GAMEPLAY11](GAMEPLAY11.md), [Character Scale & Style Standard](CHARACTER_SCALE_STYLE_STANDARD.md), [task](TASKS/GAMEPLAY11.md). Manual Release đã quan sát NPC idle/patrol toggle, pet follow, overlay và F7/F8; held-WASD/full loop nhiều FPS chưa kiểm thủ công, movement/state được kiểm bằng integration test.

## Baseline Phase F trước Gameplay11 (lịch sử)

Các bảng/kết quả bên dưới mô tả thời điểm audit ban đầu, không ghi đè cập nhật Gameplay11 trên đây.

2026-11-04. Cwd/Git root D:/my_code/MyFarmEngine; branch feat/gameplay10-phase-f-depth-directional-harvest; HEAD eac0715. Git sạch trước thay đổi. Không tìm thấy AGENTS.md trong repo ngoài vendor/output, D:/ hoặc D:/my_code/. Lỗi Git work-tree ở lần đọc sandbox đầu đã hết khi chạy với quyền truy cập đúng repo; core.bare=false, repo không hỏng.

Commit gần đây: eac0715 docs Phase F; 92b3a22 presets; 33c0419 orchard/directional actors; 851dc3d retrigger one-shot; cc38534 canopy mask.

DONE chỉ có nghĩa phạm vi được mô tả và kiểm chứng, không phải toàn master specification.

| Tính năng | Status | Bằng chứng / giới hạn |
| --- | --- | --- |
| Application/Window | DONE | Đường gọi main tới Game/scene có thật; lifecycle và smoke pass |
| Time | PARTIAL | Clamp/minimize/resize có; chưa fixed timestep/interpolation |
| Engine class | TODO | Engine.h/.cpp rỗng; Application là vòng đời đang dùng |
| OpenGL/Shader/Texture/Canvas cơ bản | DONE | Context 3.3, GLSL compile/link, PNG/RGBA, GL_LINEAR/blending; pixel test pass |
| Renderer2D đầy đủ | PARTIAL | Atlas/culling/Y-sort; mỗi sprite một draw, chưa batching/profiler |
| Camera/Input | DONE | Follow/zoom/clamp, world↔screen, WASD/key events tích hợp; camera test pass |
| ECS/Components/Systems | PARTIAL | ColliderComponent và Collision/Interaction có thật; chưa ECS registry, nhiều component khác rỗng |
| Animation/sprite sheet | PARTIAL | 62 clip rows, pause/loop/one-shot/retrigger/snapshot RAM, bốn hướng; chưa tám hướng/frame events/layers |
| Asset/resource management | PARTIAL | Manifest/decoder/cache/fallback tích hợp; chưa cross-validator/atlas builder/editor/async |
| Scene/World/Tilemap | PARTIAL | Grid/chunk range/zones/objects/colliders; một ảnh nền, chưa chunk streaming/unloading |
| Player | PARTIAL | Đi/chạy/collision/đổi Linh-An; run dùng walk nhanh hơn; chưa outfit layers |
| NPC | PARTIAL | Đứng cố định tại NpcPosition, đổi clip theo timer; chưa pathfinding/schedule/dialogue/relationship |
| Animal | PARTIAL | Chó follow/collision; một hướng any; sleep chỉ frame 0 và đổi tỷ lệ; chưa feed/pet/products |
| Farming | PARTIAL | Một turnip plot gieo/chờ 5 giây/thu; hai táo stock/regrow; chưa tưới/ba crop/vòng bán |
| Inventory/Tools | PARTIAL | Bốn hotbar icon và inventoryCount chung; chưa item IDs/stacks/money/equipment; can/cake chưa có use logic |
| Collision | DONE | Foot AABB, bounds/water/static/dynamic, substep slide, line of sight; tests pass |
| Interaction/effects | PARTIAL | F kiểm tra range/facing/tool/stamina/cooldown/capacity; model đổi ngay rồi phát event, chưa animation impact |
| Chop tree | PARTIAL | Generic object giảm health/tắt collider; hình baked vẫn còn, chưa fall/wood drops |
| Audio | TODO | Không có backend/call trong target chạy |
| SQLite save/load | TODO | Save.cpp rỗng, Save.h chỉ khai báo; AnimatorSnapshot chỉ lưu RAM |
| Lighting/shadows | PARTIAL | Rect contact proxy dưới người, ánh sáng baked; chưa sunlight/local light/soft shadow |
| Day/night/weather/seasons | TODO | DayNightSystem.h và WeatherSystem.h rỗng |
| Door/shop/crafting/cooking/quests/social | TODO | Chưa tích hợp; class/header/history image không phải tính năng |
| Multiplayer | TODO | Kế hoạch tương lai ngoài MVP |

## Build/test đã chạy

Toolchain: VS 2026 Community, MSVC 19.51.36248 x64, CMake 4.3.1-msvc1, Ninja. Vendor headers: GLFW 3.5.0, GLM 1.1.0, stb_image 2.30 và GLAD. Smoke log: OpenGL 3.3.0 NVIDIA 566.07, RTX 3050 Laptop GPU, VSync on.

1. Scripts/Build.ps1 -Phase phase-f -Configuration Debug: PrepareAssets kiểm tra 204 Original, configure/incremental build thành công; 16/16 CTest PASS, 8.13 giây. Build này chỉ copy assets vì object code đã có.
2. Sau nạp Launch-VsDevShell và CMake/Ninja PATH, build sạch riêng: cmake --preset phase-f-debug -B out/build/audit-20261104-debug; cmake --build out/build/audit-20261104-debug --parallel 4; ctest --test-dir out/build/audit-20261104-debug --output-on-failure. Compile/link 86 bước thành công, 16/16 PASS, 8.63 giây, gồm 4 graphics tests.
3. Scripts/CheckAssets.ps1: 189 imported files, 18 artwork và 204 immutable Original snapshots đúng SHA256.
4. out/build/phase-f-debug/AssetAudit.exe --report out/build/phase-f-debug/audit-20261104-assets.csv: 204 records, ready=27, needs_review=177, disabled=0, errors=0. Report có dimensions/alpha từng record.
5. out/build/phase-f-debug/MyFarmEngine.exe --smoke-test: exit 0 sau 5 frame, OpenGL/GPU log thật.
6. Đối chiếu 62 rows VisualClips với ready Assets.csv và total_frames: 0 mismatch; kiểm tra audit bổ sung, chưa có cross-validator tích hợp.

CTest gồm assets.validation/catalog, engine.camera/time/animator/lifecycle/canvas, gameplay.visuals/render_order/world/collision/foreground_trees/effects/interaction, game.smoke, assets.fallback. Graphics tests cần desktop và đã thực sự chạy. Probe pthread fail trên Windows là detection bình thường, Threads cuối cùng FOUND; không phải lỗi configure.

Không có lỗi build P0 cần sửa. Release/ASan chưa chạy phiên này. Không đo FPS, CPU/GPU frame time hoặc VRAM, không tuyên bố đạt 60 FPS. Smoke/pixel regression không thay chơi thử đầy đủ/duyệt toàn bộ animation.

Executable: out/build/phase-f-debug/MyFarmEngine.exe; bản compile sạch out/build/audit-20261104-debug/MyFarmEngine.exe. Chạy không tham số để chơi, --smoke-test tự thoát, --no-vsync dành cho đo sau này.

## Hình ảnh và thay đổi

27 ready gồm 8 history/reference, còn 19 là record nền/portrait/icon/sheet, không phải 19 bộ gameplay hoàn chỉnh. 177 Library vẫn needs_review/bị chặn upload. Catalog không báo missing/corrupt/grid/alpha error; hash không đánh giá chất lượng mỹ thuật.

Đã xem trực tiếp Linh walk-left, An action-directions, dog walk, turnip growth, world_phase_e và Original character_0001/sprite.png. Ảnh cũ là pixel art 164×219 từ nguồn. Atlas mới cần QA alignment/pivot/alpha edge; không tuyên bố đã duyệt hình cả 204 ảnh. Phân biệt nguyên nhân trong WORLD_ASSET_BIBLE.

Chỉ thêm sáu tài liệu yêu cầu, Docs/TASKS và link README. Không sửa source/CMake/CSV/asset, không xóa ảnh, không tạo engine mới, không commit/push. Output build/report/fallback nằm trong out/ bị Git ignore. Tài liệu cũ giữ làm lịch sử; baseline hiện hành là báo cáo này.

Ba task tiếp: P0-001 visual/frame/canopy QA → P1-001 cross-validator → P1-002 impact events. Xem ROADMAP và TASKS.
