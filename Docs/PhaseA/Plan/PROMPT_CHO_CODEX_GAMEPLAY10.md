# Prompt cho Codex — triển khai Gameplay10 theo từng bước

Bạn là kỹ sư C++20/OpenGL 4.6 đang tiếp tục dự án MyFarmEngine 2D nông trại. Hãy làm việc trực tiếp trên repository hiện tại. Mục tiêu là nâng cấp từ Gameplay03 + Sprite07 + Visual08 + Companion09 với animation và đồ họa riêng cho từng thực thể, asset sạch, thế giới rộng hơn, nhiều phong cảnh/đối tượng, hiệu ứng, thao tác và va chạm ổn định.

## Quy tắc an toàn

- Đọc `AGENTS.md`, `README.md`, `CMakeLists.txt` và source hiện hành trước khi sửa.
- Không xóa, reset hoặc ghi đè không hồi phục các asset gốc, save, ThirdParty hay thay đổi của người dùng.
- Tạo branch `feat/gameplay10-world-animation`.
- Mỗi lát phải build/test trước khi sang lát sau. Nếu test hỏng, dừng tại lát đó và báo file/lỗi.
- Không tạo ảnh giả rồi gọi là asset thật. Ảnh chưa xác định phải đưa vào `Assets/Workspace` và ghi metadata `needs_review`.
- Không đưa texture/GPU pointer vào component ECS.
- Không structural-mutate registry trong `Each`; dùng pending commands.
- Không nói “đã tối ưu nhất” nếu chưa có benchmark trước/sau.

## Hiện trạng cần giữ

Đã có ECS Registry dense storage, Player/Farm/Crop/Inventory/Tool/Shop/Save, HUD, Animator/SpriteSheet, VisualCatalog/WorldVisuals/SpriteSheetCache và CompanionSystem chó đi theo/tìm đường. Các hệ thống cũ phải tiếp tục chạy khi asset ngoài bị thiếu; fallback màu là hành vi hợp lệ.

## Lộ trình bắt buộc

### Phase A — kiểm kê và asset pipeline

1. Liệt kê source file và các profile/clip/binding CSV hiện có.
2. Tạo `Assets/Original`, `Assets/Workspace`, `Assets/Export` nếu thiếu.
3. Thêm `Gameplay/Data/Assets.csv` và parser có validate: đường dẫn, alpha, grid, anchor, kích thước, status.
4. Không bật asset nào nếu decode/grid/alpha không hợp lệ; dùng fallback và warning.
5. Viết test cho file thiếu, PNG hỏng, grid không chia hết, frame vượt bounds và Unicode path.

### Phase B — animation riêng từng thực thể

1. Mở rộng Animator/VisualCatalog để profile có clip `idle`, `walk`, `run`, `action`, `hurt`, `sleep` tùy loại.
2. Player chọn clip theo hướng và velocity; NPC/crop/dog có state riêng; cây chọn frame tăng trưởng theo tuổi game.
3. Không restart clip mỗi frame; pause phải giữ frame; dt NaN/âm phải an toàn.
4. Test frame bounds, loop/non-loop, đổi state, đổi hướng, pause và save/load rebuild.

### Phase C — thế giới và phong cảnh

1. Tách `WorldMap/Chunk` khỏi Farm nhưng giữ API cũ trong giai đoạn chuyển tiếp.
2. Mở rộng bản đồ tối thiểu thành làng, ruộng, hồ/rừng; mỗi zone có tile/collider và spawn riêng.
3. Camera clamp ở mép thế giới; culling theo viewport; không render toàn bản đồ vô điều kiện nếu map lớn.
4. Thêm definition data cho cây, đá, bụi, cổng, rương, biển báo và hiệu ứng nước. Visual ID khác gameplay ID.

### Phase D — hiệu ứng và interaction

1. Thêm EventBus cho `ToolUsed`, `ObjectHit`, `CropPlanted`, `CropHarvested`, `ItemPicked`.
2. Thêm EffectPool/Lifetime/Particle/FloatingText/ScreenShake với giới hạn và fallback.
3. Thêm InteractionSystem: range, facing/ray, cooldown, stamina, trạng thái bận, inventory capacity.
4. Mọi thay đổi item/HP/world phải nằm ở system nghiệp vụ, không nằm trong renderer.

### Phase E — collision

1. Thêm ColliderComponent riêng với sprite.
2. Player/animal dùng AABB/circle; tile water/fence/building dùng static collider.
3. Di chuyển tách X/Y, kiểm tra AABB trước khi commit; trượt dọc vật cản.
4. Không cho click/interact qua UI; không thao tác ngoài range hoặc vào entity đã destroy.
5. Test mép map, góc vật cản, ao, cổng, NPC, dog, object bị phá và load save.

## Cách báo cáo sau mỗi phase

Ghi rõ:

1. File tạo/sửa và lý do.
2. API/hàm chính, dữ liệu vào/ra.
3. Test đã chạy và lệnh cụ thể.
4. Kết quả pass/fail, warning, sanitizer.
5. Giới hạn chưa kiểm chứng trên Windows/MSVC/GPU.
6. Commit message theo Conventional Commits, ví dụ `feat(animation): add per-profile clips and safe asset validation`.

## Definition of Done

Chỉ kết thúc một phase khi source build sạch với `-Wall -Wextra -Wpedantic -Werror` (hoặc `/W4 /WX`), test logic pass, không có ASan/UBSan lỗi, asset thiếu vẫn fallback, save cũ không crash và có hướng dẫn cài từng file. Sau đó tạo commit riêng, không gộp nhiều phase vào một commit.

Bắt đầu bằng Phase A. Không tự nhảy sang Phase C/E khi Phase A chưa có test pass.
