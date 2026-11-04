# MyFarmEngine — Roadmap sau audit

2026-11-04. Baseline Phase F compile sạch và 16/16 tests pass; chưa đạt vertical slice. Giữ cấu trúc Engine/Game/Gameplay, không dựng dự án mới. Nội dung lịch sử trong tài liệu cũ không thay baseline PROGRESS.md.

| Priority | Task | Dependencies | Acceptance gate |
| --- | --- | --- | --- |
| P0 | Baseline audit/build/runtime/catalog | DONE trong phiên này | Fresh Debug compile, 16 tests, smoke và hash pass |
| P0 | [P0-001](TASKS/P0-001.md) frame/alpha/canopy QA | Baseline | Tái hiện lỗi, tách artwork/filter/scale/render; source nguyên vẹn |
| P1 | [P1-001](TASKS/P1-001.md) asset/clip cross-validator | P0-001 | Reject references/frame/grid/coverage sai tự động |
| P1 | [P1-002](TASKS/P1-002.md) animation impact | P1-001 | Model thay đúng một lần tại impact, cancel/retrigger rõ |
| P1 | [P1-003](TASKS/P1-003.md) fixed simulation | P1-002 | Render rates khác nhau cho kết quả gameplay tương đương |
| P2 | [P2-001](TASKS/P2-001.md) tám hướng/outfit | P1-001, P1-002 | Art đủ coverage, layers aligned, outfit đổi thật |
| P2 | [P2-002](TASKS/P2-002.md) farm/typed inventory | P2-001, P1-003 | Ba crop, tưới/thu/bán, cây chặt cập nhật hình và collider |
| P3 | [P3-001](TASKS/P3-001.md) clock/light/shadow | P2-002 | Ngày đêm mượt, shadow proxy/local light có toggle |
| P4 | [P4-001](TASKS/P4-001.md) NPC/animal/shop/door | P2-002 | Một cư dân/thú/cửa/shop hoạt động, giao dịch atomic |
| P4 | [P4-002](TASKS/P4-002.md) SQLite | P3-001, P4-001 | Restart/roundtrip/migration/backup/write-error tests |
| P5 | Expansion/polish/performance | MVP gate | Task cụ thể chỉ mở sau slice ổn định |

MVP gate: một farm chơi được, player idle/walk tám hướng, outfit thay được, ba crop, cây chặt, animal/NPC/door/shop, inventory, SQLite restart và day/night/shadows. Chơi full loop và kiểm package trên máy khác trước mở rộng. Không tính history/fallback/class rỗng là feature.

Ba task tiếp: P0-001 → P1-001 → P1-002. Mỗi task có acceptance/tests/out of scope. Không triển khai cả roadmap trong lượt audit. Không tự push; chỉ commit thay đổi đã kiểm thử. Multiplayer ngoài MVP.
