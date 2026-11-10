# GAMEPLAY13 — Cổng/cửa và bố trí hình có quy luật

2026-11-10. DONE kỹ thuật trong prototype; artwork/interior còn PARTIAL.

- CozyV2: 3 source PNG riêng, prompt/hash/role manifest. Dirt material, fence panel/post/rail, farmhouse open-state cùng canvas. Stage không sửa Original.
- WorldDistribution: Bridson Poisson active list/grid, guard/cap, deterministic seed; min spacing theo bounds + wind margin. CacheDecor giữ clear đường/collider/luống/portals, mật độ theo zone × trường mảng lớn, 24 chunk.
- WorldPortal: animation .25s, collision opens after pose complete, keep hinge/posts, close occupancy rejection. F hợp lệ theo range/facing/LOS; mouse theo đúng chủ/vùng vườn; không xuyên UI, không phụ thuộc tool.
- Save v2 portal intent, v1 compatibility; load pose snap và kiểm player placement trước mutate scene. Legacy removed gate không tương tác vô hình.
- Auto tests: Poisson seed/spacing/domain, portal transition/post/corridor, close occupant, F/mouse/permissions, scene save restore, codec v1/v2. SQL test slot 2 roundtrip v2 thành công, không ghi slot 1.
- Native: F mở gate, B+click đóng; N QA house, F mở, B+click đóng, thấy hai ảnh/state đúng. Reset về spawn/debug off.

Giới hạn: chưa interior, chưa gesture/sound opening, chưa Poisson cho NPC/physical layout; chưa WFC/infinite streaming. Density là quy tắc sinh trang trí có tác giả, không random layout gameplay. Chi tiết source và chỉnh tay: WORLD_VISUAL_ALGORITHMS.md.
