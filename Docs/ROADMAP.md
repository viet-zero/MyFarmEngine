# MyFarmEngine — Roadmap sau audit

Cập nhật 2026-11-08: đường/rào và collider chân nhà/thân cây đã chỉnh trong cảnh Cozy; Debug/Release 27/27. Xem [COLLISION_VISUAL_ALIGNMENT.md](COLLISION_VISUAL_ALIGNMENT.md). Art đường/rào vẫn prototype; tiếp theo polish artwork, cổng/cửa nhà và animation.

## Gate mới nhất — 2026-11-08

P1-001/002/003 DONE kỹ thuật. Farming/replant/water/growth, typed inventory/kho, trade và manual SQL Server save/load có trong prototype. Có hai cách chơi: nhân vật và quản lý vườn bằng chuột dùng chung model. CozyV1 thêm cỏ/hoa/hồ/cây/nhà và scale CSV. P2-001 còn PARTIAL: 8 hướng local nhưng chỉ 2 walk frames/hướng, thiếu action/outfit/NPC identity. P0 artwork acceptance còn PARTIAL.

Ba task tiếp: duyệt biome/shore/footprint nhà town; chuẩn hóa action/outfit; UI chuột shop/kho và feedback/balance. Chi tiết acceptance và nơi chỉnh: PROJECT_HANDOVER.md. Chưa mở rộng world trước khi chốt visual/farming. Các gate bên dưới là lịch sử.

## P2-002B gate — 2026-11-05

Trồng/tưới/growth/trồng lại và kho sở hữu đã tích hợp, Debug/Release 26/26 PASS. [Contract](FARMING_HOME_STORAGE.md). Ba crop definitions được kiểm logic, scene chỉ có turnip; P2 umbrella vẫn PARTIAL. Theo user choice 1+3, hotbar 4 ô tự ẩn và K mở bảng kho khi cần. Next [P2-002C](TASKS/P2-002C.md): money/seed choice/buy-sell từ bag hoặc kho hợp lệ → SQL durable snapshot/save-load. Chưa ghi DB hoặc lưu qua restart; artwork làm sau.

## Logic-first inventory gate — 2026-11-05

Per user direction, gameplay logic no longer waits for new character images. P2-002A typed inventory is integrated; P2-002 remains PARTIAL. Next [P2-002B crop/watering](TASKS/P2-002B.md), then money/buy/sell and a save/load pilot using installed SQL Server where appropriate. P2-001 remains blocked only for its own art acceptance. .\SQLEXPRESS read-only connection tested successfully; no game database or C++ persistence added. [Inventory details](TYPED_INVENTORY.md).

## Gate P0-002 — 2026-11-05

[P0-002](TASKS/P0-002.md) DONE: 11 metadata flip fields đã sửa, regression Debug/Release 24/24; không thay ảnh/engine. P0-001 còn authored pivots/canopy/art, P2-001 còn BLOCKED artwork acceptance. Bước tiếp: chốt model/root Linh, tạo/duyệt tám hướng và outfit layers, rồi triển khai P2-001. Findings P0-001 bên dưới là lịch sử trước sửa.

## Gate artwork P0-001 — 2026-11-05

Audit xong, P0-001 PARTIAL vì art/root/masks chưa có. [Evidence và handoff](P0-001_ARTWORK_PIVOT_AUDIT.md). P2-001 BLOCKED artwork acceptance: chưa bộ tám hướng/outfit nào đủ; Linh là mẫu pilot đề xuất. Trước tiên [P0-002](TASKS/P0-002.md) sửa 11 rows direction/flip, rồi chốt authored roots và tạo/duyệt layers + diagonals, sau đó implementation P2-001. P1-001/002/003 DONE kỹ thuật; không mở rộng world khi chưa đạt MVP. Các gate dưới là lịch sử.

## Gate P1-003 — 2026-11-05

[P1-003](TASKS/P1-003.md) DONE kỹ thuật: fixed simulation 60 Hz, bounded catch-up, buffered input và actor/camera interpolation; Debug/Release 24/24. [Giới hạn và verification](FIXED_TIMESTEP.md). Tiếp theo P0-001 art review và P2-001 tám hướng/outfit (cần authored art), rồi P2-002 farm/typed inventory. Không mở rộng world trong lượt này. Các gate bên dưới là lịch sử.

## Gate P1-002 — 2026-11-05

[P1-002](TASKS/P1-002.md) DONE kỹ thuật: F → animation IMPACT → model/FX exactly-once; cancel/pause/retrigger/target invalid được kiểm thử, Debug/Release 22/22. [Contract và giới hạn](ACTION_ANIMATION_EVENTS.md). Tiếp theo P1-003 fixed simulation; P0 artwork vẫn PARTIAL, chưa triển khai tám hướng hoặc world expansion.

## Gate P1-001 — mốc trước P1-002

[P1-001](TASKS/P1-001.md) DONE: cross-validator, coverage schema v1, 97 checks và Debug/Release 21/21 tests pass. P0-001 vẫn PARTIAL cho artwork/pivots/masks; phần kỹ thuật dữ liệu được triển khai độc lập, không cần thay ảnh. Thứ tự code tiếp: P1-002 animation impact → P1-003 fixed simulation → P2 gameplay; chưa mở rộng world.

## Gate Gameplay11 — mốc lịch sử 04/11

Sửa runtime/console, opt-in behavior, placement/UI: DONE theo [task Gameplay11](TASKS/GAMEPLAY11.md), Debug/Release 19/19. P0-001 còn PARTIAL do artwork/canopy mask/pivots chưa duyệt đầy đủ. Giữ thứ tự tiếp P0 art review → P1 cross-validator → impact events, chưa mở rộng tám hướng hoặc world.

2026-11-04. Baseline Phase F compile sạch và 16/16 tests pass; chưa đạt vertical slice. Giữ cấu trúc Engine/Game/Gameplay, không dựng dự án mới. Nội dung lịch sử trong tài liệu cũ không thay baseline PROGRESS.md.

| Priority | Task | Dependencies | Acceptance gate |
| --- | --- | --- | --- |
| P0 | Baseline audit/build/runtime/catalog | DONE trong phiên này | Fresh Debug compile, 16 tests, smoke và hash pass |
| P0 | [P0-001](TASKS/P0-001.md) frame/alpha/canopy QA | Baseline | Tái hiện lỗi, tách artwork/filter/scale/render; source nguyên vẹn |
| P1 | [P1-001](TASKS/P1-001.md) asset/clip cross-validator — DONE | Gameplay11 technical baseline | Debug/Release 21/21; references/frame/grid/coverage được kiểm tự động |
| P1 | [P1-002](TASKS/P1-002.md) animation impact — DONE | P1-001 | Debug/Release 22/22; model/FX tại impact, cancel/pause/retrigger guards |
| P1 | [P1-003](TASKS/P1-003.md) fixed simulation — DONE | P1-002 | Render rates khác nhau cho kết quả gameplay tương đương |
| P2 | [P2-001](TASKS/P2-001.md) tám hướng/outfit | P1-001, P1-002 | Art đủ coverage, layers aligned, outfit đổi thật |
| P2 | [P2-002](TASKS/P2-002.md) farm/typed inventory | P1-002/P1-003 + P2-002A; P2-001 only for final art | Ba crop, tưới/thu/bán, cây chặt cập nhật hình và collider |
| P3 | [P3-001](TASKS/P3-001.md) clock/light/shadow | P2-002 | Ngày đêm mượt, shadow proxy/local light có toggle |
| P4 | [P4-001](TASKS/P4-001.md) NPC/animal/shop/door | P2-002 | Một cư dân/thú/cửa/shop hoạt động, giao dịch atomic |
| P4 | [P4-002](TASKS/P4-002.md) SQLite | P3-001, P4-001 | Restart/roundtrip/migration/backup/write-error tests |
| P5 | Expansion/polish/performance | MVP gate | Task cụ thể chỉ mở sau slice ổn định |

MVP gate: một farm chơi được, player idle/walk tám hướng, outfit thay được, ba crop, cây chặt, animal/NPC/door/shop, inventory, SQLite restart và day/night/shadows. Chơi full loop và kiểm package trên máy khác trước mở rộng. Không tính history/fallback/class rỗng là feature.

Cập nhật 05/11: P1-001/P1-002 đã DONE kỹ thuật; ba task code tiếp là P1-003 → P2-001 → P2-002. P0-001 art review tiếp tục riêng. Mỗi task có acceptance/tests/out of scope. Không triển khai cả roadmap trong lượt audit. Không tự push; chỉ commit thay đổi đã kiểm thử. Multiplayer ngoài MVP.
