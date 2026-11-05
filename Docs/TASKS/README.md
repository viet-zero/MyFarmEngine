# Backlog triển khai

Cập nhật 05/11/2026. Baseline và evidence ở [PROGRESS](../PROGRESS.md), dependencies ở [ROADMAP](../ROADMAP.md).

| Task | Status | Phạm vi |
| --- | --- | --- |
| [Gameplay11](GAMEPLAY11.md) | DONE kỹ thuật | Runtime/console/demo/scale/UI đã kiểm thử; artwork review còn |
| [P0-001](P0-001.md) | PARTIAL | Audit 32 inputs/68 frames xong; authored pivots/art/masks còn thiếu |
| [P1-001](P1-001.md) | DONE | Asset/clip cross-validation và schema coverage v1; Debug/Release 21/21 |
| [P1-002](P1-002.md) | DONE kỹ thuật | Animation markers + impact transaction; Debug/Release 22/22 |
| [P1-003](P1-003.md) | DONE kỹ thuật | Simulation 60 Hz/input/interpolation; Debug/Release 24/24 |
| [P0-002](P0-002.md) | DONE kỹ thuật | 11 direction/flip rows; Debug/Release 24/24 |
| [P2-001](P2-001.md) | BLOCKED art / TODO code | Thiếu tám hướng/layers/authored pivots |
| [P2-002](P2-002.md) | PARTIAL | Typed inventory integrated; watering/three crops/economy pending |
| [P2-002A](P2-002A.md) | DONE kỹ thuật | Stable ItemIds/stacks/impact/hotbar quantities |
| [P2-002B](P2-002B.md) | TODO — next logic | Reusable plot, crop IDs, watering/growth |
| [P3-001](P3-001.md) | TODO | Lighting/shadows |
| [P4-001](P4-001.md) | TODO | NPC/animal/shop/door |
| [P4-002](P4-002.md) | TODO | SQLite save/load |

Mỗi task cần build/test phù hợp, cập nhật tiến độ và hạn chế. Chỉ commit thay đổi đã kiểm thử, không tự push. P5/world expansion và multiplayer chỉ mở sau MVP ổn định.
