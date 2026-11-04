# MyFarmEngine — Audit có kiểm chứng

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
