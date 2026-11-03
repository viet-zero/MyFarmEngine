# Phase F: thứ tự vẽ, hoạt ảnh theo hướng và thu hoạch cây trên ảnh nền

Tài liệu này giải thích cách chạy, cách sửa từng tham số và cách mang ba hệ thống sang game 2D khác. Mã hiện tại nằm trong `Gameplay/Graphics/RenderOrder.*`, `Gameplay/Graphics/VisualCatalog.*`, `Gameplay/World/ForegroundTrees.*`, `Gameplay/Systems/CollisionSystem.*`, `Gameplay/Systems/InteractionSystem.*` và `Gameplay/World/DemoScene2D.*`.

## Thử ngay trong game

Chạy `Scripts/Build.ps1 -Phase phase-f -Configuration Debug`, rồi mở `out/build/phase-f-debug/MyFarmEngine.exe` bằng File Explorer. Trong cảnh game F3, dùng WASD để đi, giữ Shift trái để chạy, Space để xem tư thế hành động, H để xem tư thế bị thương, Tab để đổi Linh/An. F7 đưa nhân vật đến phía nam cây táo, mặt quay về cây; bấm F để hái quả. Tiêu đề cửa sổ hiện `Apples: 1` sau lần hái thành công. F8 đặt nhân vật sát mép tán cây để kiểm tra lớp lá che phần trên cơ thể. R đặt lại cảnh và số quả thử nghiệm.

F chỉ tác động lên mục tiêu ở gần, phía trước nhân vật và có đường nhìn thông thoáng. Mỗi cây có số quả ban đầu trong CSV; sau khi hết, cây mọc lại sau `regrowSeconds`. Số `Apples` trên tiêu đề là bộ đếm của cảnh thử. `InteractionActor::inventoryCount` hiện là sức chứa chung cho hạt, quả và đồ nhặt; chưa có loại item riêng hoặc lưu số quả ra file save.

## 1. Y-sorting: chân quyết định lớp trước/sau

`RenderOrder` chỉ lưu các lệnh vẽ nhìn thấy trong khung hình. Mỗi frame, `DemoScene2D::Render` gọi:

```cpp
queue.Begin(cameraTopLeft, cameraViewSize);
queue.Submit(RenderKind::Actor, actorIndex, spriteTopLeft, spriteSize, feet.y);
queue.Submit(RenderKind::TreeCanopy, treeIndex, canopyTopLeft, canopySize, tree.footY);
queue.Sort();
for (const auto& entry : queue.Entries()) Draw(entry);
```

`Begin(viewOrigin, viewSize)` đặt vùng nhìn theo world units và xóa danh sách của frame trước nhưng giữ dung lượng vector. `Submit(kind,index,origin,size,footY)` trả `false` nếu hình nằm ngoài viewport, kích thước không hợp lệ hoặc có NaN/vô hạn. `kind` chọn hàm vẽ; `index` chỉ tới đối tượng trong mảng của loại đó; `origin,size` là hộp **hình**, dùng để cull; `footY` là Y của chân hoặc đáy thân, dùng để sắp xếp. Hai đối tượng cùng `footY` giữ thứ tự chúng được đưa vào queue nhờ `sequence`. `Sort()` chạy sau khi đã cull, rồi vẽ từ Y nhỏ đến Y lớn. Chi phí là `O(v log v)` với `v` là số đối tượng đang thấy, không phải toàn bộ bản đồ.

Với player/NPC/dog/crop/item, `footY` là vị trí chân hoặc điểm đặt trên đất. Với cây, `footY` là đáy thân đo trên ảnh nền, **không phải đáy tán**. Khi nhân vật ở phía trên thân, tán được vẽ sau và che nhân vật; khi nhân vật xuống phía dưới thân, nhân vật được vẽ sau. Collider chân nhỏ hơn sprite: kích thước hình 156×156 của người không được dùng làm vùng va chạm.

Ảnh `world_phase_e` vẫn được vẽ một lần làm nền tĩnh. Hai cây táo được lấy mẫu lại từ **chính texture nền** bằng `Canvas2D::SpriteEllipse(texture,x,y,width,height,uv)`. Shader chỉ cho các pixel trong mặt nạ ellipse đi qua, nên tán có thể nằm trong queue mà không cắt file PNG. `uv` là `{x/worldWidth,y/worldHeight,width/worldWidth,height/worldHeight}`. `SpriteEllipse` bật uniform `ellipseMask`, vẽ rồi tắt ngay để sprite sau không bị cắt. Cách này xấp xỉ viền tán; nếu muốn đường viền từng chiếc lá thật chính xác, bước sau là vẽ một mặt nạ alpha riêng hoặc xuất foreground layer từ phần mềm vẽ.

### Chỉnh vùng tán

`Gameplay/Data/ForegroundTrees.csv` có các cột:

| Cột | Công dụng |
|---|---|
| `gameplay_id` | Ghép với collider thân cây cùng ID trong `Objects.csv`; không dùng ID hình ảnh. |
| `canopy_x,canopy_y,width,height` | Hình chữ nhật trên **tọa độ world** chứa tán, để cull và lấy mẫu texture. Thu hẹp nếu lá che cả thùng, hàng rào hoặc vật ở cạnh. |
| `foot_y` | Mốc Y nơi đổi thứ tự che khuất; thường là chân thân cây trên ảnh. |
| `interact_x,interact_y` | Điểm F nhắm đến, đặt ở phía có lối đi, ngoài collider thân. |
| `fruits` | Số quả ban đầu, từ 1 đến 100. |

Ảnh nền 1536×1024 được kéo ra world 6144×4096. Khi đo trên PNG gốc, nhân tọa độ pixel với 4 để đưa vào CSV. Ví dụ tán bắt đầu ở pixel `(382.5,225)` thành `(1530,900)` world units. Sau mỗi lần chỉnh CSV, build lại để copy dữ liệu cạnh EXE; chạy F8 và xem lá có che đúng người mà không che đồ khác.

## 2. Hoạt ảnh nhiều tư thế và hướng

`Gameplay/Data/VisualClips.csv` khai báo clip theo `profile + state + direction`. `profile` là `player_linh`, `player_an`, `npc_linh`, `npc_an`, `dog` hoặc `crop_turnip`. State có `idle`, `walk`, `run`, `action`, `hurt`, `sleep` tùy profile. Hướng có `down,left,right,up,any`; `any` là fallback của một state không cần nhiều hướng.

| Cột | Công dụng |
|---|---|
| `asset_id` | ID texture ready trong `Gameplay/Data/Assets.csv`; không phải tên file trực tiếp. |
| `first_frame` | Frame đầu tiên trong atlas, bắt đầu từ 0. |
| `frame_count` | Số frame chạy liên tiếp của clip, phải nằm trong `total_frames`. |
| `fps` | Số frame/giây; tăng để chuyển động nhanh hơn mà không đổi tốc độ đi. |
| `loop` | `1` lặp idle/walk/run; `0` dừng ở frame cuối của action/hurt. |
| `total_frames` | Tổng frame trong atlas, kiểm tra giới hạn. |
| `flip_x` | `1` lật ngang UV khi bản vẽ gốc quay bên phải nhưng clip dùng cho bên trái. |

Atlas mới của người là lưới 2×2: frame 0 góc trên trái, 1 trên phải, 2 dưới trái, 3 dưới phải. Các file ở `Assets/Art/Characters/`; pipeline giữ bản gốc trong `Assets/Original/Art/Characters/` và chỉ nạp bản đã duyệt từ `Assets/Export/Art/Characters/`. Muốn tự thay sprite: xuất PNG RGBA có khung không chồng nhau; cập nhật cả `Assets.csv`, `VisualClips.csv`, manifest SHA256 và ba thư mục đúng pipeline; chạy `PrepareAssets.ps1`, `AssetAudit` và test visuals. Xem `Assets/Art/PHASE_F_PROMPTS.md` để biết nguồn và cách tạo bộ này.

Các hàm chính:

- `VisualCatalog::Load(csv)` kiểm tra schema, frame và clip trùng; `Find(profile,state,facing)` tìm clip, tự dùng hướng `any` nếu có; `AssetIds()` cho scene nạp trước texture một lần để tránh giật khi lần đầu đổi hướng.
- `FacingFromVelocity(x,y,previous)` chọn hướng theo trục vận tốc trội. Khi đứng yên hoặc đầu vào lỗi, giữ hướng cũ.
- `ResolvePlayerState(moving,running,actionTriggered,hurtTriggered,current,currentFinished)` ưu tiên hurt rồi action; action/hurt chưa xong được giữ, sau đó về walk/run/idle theo chuyển động thực.
- `AnimationController::Select(catalog,profile,state,facing,restart)` chọn clip. Mặc định `restart=false`: gọi mỗi frame cùng clip sẽ không quay về frame 0. Đặt `restart=true` **chỉ** khi có một lệnh action mới. Đổi state hoặc hướng chọn clip khác và bắt đầu timeline mới.
- `AnimationController::Update(dt,paused)` tiến thời gian; `paused=true` giữ frame. dt âm, NaN, vô hạn không làm tiến animation. `Frame()` trả chỉ số frame trong atlas, `Finished()` báo clip không lặp đã xong.
- `Save()` và `Rebuild(catalog,snapshot)` chụp/khôi phục timeline animation trong bộ nhớ. Đây chưa phải hệ thống lưu toàn bộ game xuống đĩa.
- `GrowthFrame(age,stageDurations)` chọn giai đoạn cây trồng theo tuổi. Mảng ví dụ `{1,1,1,2}` là thời lượng của bốn giai đoạn theo đơn vị tuổi game; tuổi không hợp lệ về frame 0.

`DemoScene2D::DrawActor` lấy `animation.Clip()->assetId`, texture đã nạp và `AssetManifest::FrameUv`, rồi áp `flipX` vào UV. Nếu asset thiếu, chân dung một tư thế hoặc hình khối thay thế được dùng. `m_PlayerAnimation`, `m_NpcAnimation`, `m_DogAnimation`, `m_CropAnimation` là các controller tách riêng; không chia sẻ đồng hồ animation. NPC đổi state theo chu kỳ thử, chó chọn idle/walk/run/sleep theo khoảng cách, cây trồng chọn frame theo tuổi. Những quy tắc này có thể thay bằng AI và gameplay thật ở game khác.

## 3. Collider thân cây và sự kiện hái quả

`Objects.csv` chứa hộp chắn quanh **thân** cây, ví dụ `tree.apple.01`; `ForegroundTrees.csv` chỉ chứa hình tán và điểm tương tác. `WorldMap::CanOccupy(center,collider)` kiểm tra giới hạn thế giới, ô nước, object và static collider. `ColliderComponent::halfSize` là nửa chiều rộng/cao của vùng **chân**. Player dùng `(18,12)`, chó `(22,13)`. Nếu sprite lớn hơn hộp chân, đó là chủ ý để đi sau tán và đi sát cây.

`CollisionSystem::MoveAndSlide(world,position,direction,speed,deltaTime,collider,otherActors)` trả vị trí mới. Hướng được chuẩn hóa để đi chéo không nhanh hơn; `speed` là world units/giây (đi 180, chạy 300), `deltaTime` được giới hạn tối đa 0.1 giây, bước con tối đa khoảng 8 units; thử X rồi Y để trượt dọc chướng ngại. `otherActors` chứa NPC/chó hoặc người chơi khác. Đầu vào NaN/âm hoặc vị trí đầu không hợp lệ giữ nguyên vị trí. Đổi kích thước chân, tốc độ hay collider thì chạy lại `gameplay.collision`.

Luồng F: `InteractionSystem::Nearest(actor,targets)` tìm mục tiêu đang hoạt động trong `Range=100` và góc nhìn phía trước; `WorldMap::HasLineOfSight(from,to)` chặn xuyên qua nhà, rào và nước; `InteractionSystem::Use(actor,target,selectedItem,events)` kiểm tra cooldown, stamina, sức chứa và quy tắc mục tiêu. Với `FruitTree`, hàm trừ `stock`, cộng một vào sức chứa chung, phát `FruitHarvested`. `InteractionSystem::Tick(actor,targets,dt)` hồi stamina/cooldown và làm quả mọc lại khi `stock==0` sau `regrowSeconds=30`. Event đưa qua `EventBus` sang `EffectPool`; vẽ không thay đổi luật va chạm. Hái hết quả không tắt collider thân cây.

`ForegroundTrees::Load(csv,world)` chỉ nhận ID có collider chắn trong `WorldMap`, từ chối tọa độ/giá trị lỗi và giữ dữ liệu cũ nếu CSV mới sai. `Contains(gameplayId)` giúp `DemoScene2D::Reset` không biến thân cây ăn quả thành object phá được. `Entries()` trả danh sách bất biến để thêm tán vào render queue.

## Mang sang game khác hoặc ECS

1. Quy ước mọi vị trí chân theo cùng một hệ world units, Y tăng xuống dưới. Tách `Transform(position)`, `Sprite(bounds,clip)`, `Collider(halfSize)`, `InteractionTarget` thành dữ liệu riêng; một entity có thể có cả bốn.
2. Update logic và collision trước. Tạo một render entry cho từng entity **đang nhìn thấy**, lấy `footY` từ chân. Background luôn vẽ trước queue; UI luôn vẽ sau queue.
3. Với ảnh nền vẽ sẵn, đánh dấu vùng tán + foot Y + collider thân trên editor/CSV. Không dùng cả hình tán làm collider. Kiểm tra từ cả hai phía và tại mép tán.
4. Gắn `AnimationController` riêng từng entity. Chỉ gọi restart khi sự kiện mới xảy ra. Nạp sẵn atlas của những clip thường dùng; cache texture theo asset ID.
5. Tương tác cập nhật model, phát event, rồi renderer đọc model. Với game online dự kiến, server phải kiểm tra khoảng cách, hướng, stock và cooldown; client chỉ dự đoán hiệu ứng. Kế hoạch mạng nằm trong `Docs/KeHoachOnline.md`.

## Kiểm tra và giới hạn

Chạy `Scripts/Build.ps1 -Phase phase-f -Configuration Debug`, tương tự Release; `-Asan` kiểm tra các target CPU. Test `gameplay.render_order` kiểm tra cull, thứ tự và Y bằng nhau; `gameplay.visuals` kiểm tra frame/hướng/pause/rebuild/retrigger; `gameplay.collision` kiểm tra thân cây và điểm F7/F8; `gameplay.foreground_trees` kiểm tra cấu hình; `gameplay.interaction` kiểm tra stock và event; `engine.canvas` kiểm tra mặt nạ ellipse không rò sang sprite khác. `game.smoke` kiểm tra game khởi động và render vài frame. Test logic không thay được việc nhìn trực tiếp F7/F8 trên máy có OpenGL.

Ảnh nền hiện có ba cây táo trang trí, Phase F gắn overlay/collider/thu hoạch cho hai cây trong CSV. Mép ellipse chỉ xấp xỉ viền lá. Bước đồ họa kế tiếp là alpha mask chính xác hoặc foreground layer được xuất riêng; bước gameplay kế tiếp là item `Apple` riêng, save/load phiên bản hóa và trạng thái cây trong multiplayer.
