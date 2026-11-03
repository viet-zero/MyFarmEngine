# Kế hoạch Gameplay03 → Animation, thế giới và tương tác

Tài liệu này là bản triển khai tiếp theo sau Companion09. Mục tiêu là mở rộng game theo từng lát nhỏ có thể build và test, tránh thay toàn bộ engine trong một lần.

## Trạng thái source hiện tại

Các phần đã có: ECS và dense component registry, player, farm 24×18, crop/inventory/tool, save/load, shop, HUD, sprite player, visual profile cho NPC/cây/decoration và chó companion BFS. Các file Visual08/Companion09 còn dùng fallback màu khi PNG chưa sạch.

## Nguyên tắc bắt buộc

1. Không ghi đè ảnh gốc. Dùng ba thư mục:

```text
Assets/Original/       ảnh tải về hoặc ảnh gốc
Assets/Workspace/      ảnh đang cắt/chỉnh và file làm việc
Assets/Export/         PNG/sprite sheet sạch được game dùng
```

2. Mỗi loại thực thể có profile và clip riêng. Player, NPC, crop, dog, tree, rock và item không dùng chung một cấu hình animation chỉ vì cùng là sprite.
3. Gameplay không phụ thuộc texture GPU. Component giữ ID/state; WorldVisuals và SpriteSheetCache giữ phần vẽ.
4. Không tạo hoặc xóa component trong `Registry::Each`; thu thập lệnh rồi áp dụng sau vòng duyệt.
5. Tất cả thao tác phải kiểm tra khoảng cách, collider, trạng thái bận, năng lượng và tồn kho trước khi thay dữ liệu.
6. Mỗi lát thay đổi phải có test logic trước, sau đó mới test render.

## Lát 1 — pipeline asset sạch và animation nhiều hướng

Tạo metadata cho từng asset: `asset_id`, `kind`, `source`, `export_path`, `columns`, `rows`, `frame_width`, `frame_height`, `anchor_x`, `anchor_y`, `has_alpha`, `status`. Từ một ảnh tĩnh không được tự khẳng định đã có animation; cần tạo frame mới hoặc giữ clip tĩnh 1×1.

Player dùng `PlayerSprite.cfg`. NPC/crop/dog dùng `Visuals.csv`, `VisualClips.csv`, `VisualBindings.csv`. Thêm hướng vào profile nếu sheet thực sự có hàng hướng; nếu chỉ có ảnh nhìn xuống thì dùng clip 1 hướng và không lật giả để che thiếu dữ liệu.

Kiểm tra asset trước khi bật: file tồn tại, PNG decode được, alpha đúng, kích thước chia hết cho grid, không có margin ngoài dự kiến, UV không lật sai. Asset lỗi dùng fallback và ghi cảnh báo, không làm game crash.

## Lát 2 — chuyển động đẹp hơn

Thêm `VisualState::Walk`, `Run`, `Action`, `Hurt`, `Interact` theo từng profile. Animator giữ timeline riêng cho từng entity. PlayerAnimationSystem chọn clip theo velocity, hướng và hành động. CompanionSystem chọn walk/sleep theo mode.

Không cập nhật animation trong `Render()`; cập nhật trong `Update()` bằng `dt`. Khi pause, inventory hoặc shop mở, không advance timeline. Khi đổi state, restart đúng một lần, không restart mỗi frame.

## Lát 3 — mở rộng thế giới

Đổi farm từ mảng cố định 24×18 sang `WorldMap` có kích thước cấu hình, nhưng giữ API `TryCell`, `CanMoveTo` để các system cũ không phải đổi cùng lúc. Bước an toàn:

1. Tách `FarmCell` và `Farm::Width/Height` thành map data.
2. Thêm vùng chunk 32×32 hoặc 64×64.
3. Camera chỉ render chunk trong viewport cộng một viền.
4. Collision query chỉ xét ô quanh AABB của entity.
5. Save ghi kích thước map/chunk version.

Không tạo thế giới vô hạn trước khi có giới hạn bộ nhớ và quy tắc save. Bản đầu nên có 3 khu: làng, ruộng, hồ/rừng; mỗi khu có tile/collider/đối tượng riêng.

## Lát 4 — cảnh quan và đối tượng riêng

Thêm definition data cho `Tree`, `Rock`, `Bush`, `FenceGate`, `WaterRipple`, `Chest`, `Sign`, `Decoration`. Visual profile chỉ mô tả hình; definition gameplay mô tả collider, có thể chặt/nhặt/mở hay không.

Ví dụ luồng cây: người chơi ở trong bán kính → tool đúng → kiểm tra stamina và cooldown → phát event `TreeHit` → giảm HP → tạo hiệu ứng bụi/âm thanh → khi HP=0 tạo log/wood và thay state thành stump. Không để Renderer tự trừ HP hoặc thêm vật phẩm.

## Lát 5 — hiệu ứng và thao tác

Tạo ECS component nhẹ: `Lifetime`, `Particle`, `FloatingText`, `ScreenShake`, `InteractionPrompt`. Event bus gửi `ToolUsed`, `CropPlanted`, `CropHarvested`, `ObjectHit`, `ItemPicked`.

Hiệu ứng có thể bị bỏ qua nếu không đủ tài nguyên. Mỗi effect phải có giới hạn số lượng, thời gian sống và fallback. Không cấp phát vector không giới hạn mỗi frame; dùng pool hoặc reserve khi bắt đầu.

## Lát 6 — va chạm và logic an toàn

Dùng collider riêng với sprite: AABB hoặc circle cho player/animal; tile collider cho water/fence/rock/building. Di chuyển theo trục X rồi Y, thử vị trí mới bằng `CanMoveTo`, trượt dọc vật cản thay vì teleport. Interaction ray/ô trước mặt chỉ thành công khi target hợp lệ và nằm trong range.

Mọi thao tác phá hủy entity phải chạy ngoài `Each`. Entity handle sau load phải lấy lại; không giữ con trỏ/component reference qua `Swap`, `Destroy`, `Emplace` hoặc `Read`.

## Cấu trúc file đề xuất cho bản tiếp theo

```text
Engine/Animation/Animator.*
Engine/Effects/EffectPool.*
Engine/Event/EventBus.*
Gameplay/Assets/AssetManifest.*
Gameplay/Components/ColliderComponent.h
Gameplay/Components/InteractableComponent.h
Gameplay/Components/EffectComponent.h
Gameplay/World/WorldMap.*
Gameplay/World/Chunk.*
Gameplay/Systems/CollisionSystem.*
Gameplay/Systems/InteractionSystem.*
Gameplay/Systems/EffectSystem.*
Gameplay/Systems/AnimationSystem.*
Gameplay/Graphics/WorldVisuals.*
Gameplay/Data/Assets.csv
Gameplay/Data/Objects.csv
Gameplay/Data/WorldZones.csv
Tests/Gameplay10/LogicTests.cpp
```

## Thứ tự kiểm thử

Build với cảnh báo là lỗi. Test asset lỗi/thiếu, grid sai, clip sai, animation pause/restart, va chạm mép map, va chạm chéo góc, thao tác ngoài range, túi đầy, entity đã hủy, save/load version cũ, spawn object ở ô water/fence và số lượng effect tối đa.

Sau đó test render bằng EGL/OpenGL: đọc pixel frame đầu/cuối, alpha, UV theo từng hàng, object phía sau/ở trước, camera ở mép map và resize/HiDPI. Windows/MSVC/RTX 3050 vẫn phải build và chạy trực tiếp trên máy bạn.

## Hiệu năng cần giữ

- Không load/decode PNG trong vòng lặp frame.
- Cache texture theo đường dẫn chuẩn hóa + grid.
- Update animation/AI chỉ trên entity có component cần thiết.
- Render culling theo camera/chunk.
- Reserve draw batch và effect pool.
- Đo update CPU, số draw call, số texture bind và thời gian GPU riêng; không gọi một benchmark query là FPS toàn game.
