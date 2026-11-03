# Phase D — tự làm event, hiệu ứng và tương tác

Tài liệu này dùng cho MyFarmEngine và có thể áp dụng sang game 2D khác. Source chính: `Engine/Event/EventBus.*`, `Gameplay/Effects/EffectPool.*`, `Gameplay/Systems/InteractionSystem.*`, `Gameplay/World/DemoScene2D.cpp`. Trạng thái trong source là mô hình gameplay mẫu, chưa có ECS/save hoàn chỉnh.

## 1. Luồng xử lý

Khi bấm `F`, scene chọn target gần nhất phía trước bằng `Nearest`, rồi gọi `Use`. `Use` kiểm tra luật chơi và cập nhật stamina, inventory, HP/crop/pickup. Nếu thành công, nó phát `GameEvent`. Scene gọi `EventBus::Take`, đưa event vào `EffectPool::Consume`, rồi `EffectPool::Update(dt)`. `Render` chỉ đọc trạng thái và vẽ. Vì vậy một hiệu ứng bị bỏ do pool đầy không làm mất kết quả thao tác.

Trong demo: `2` chọn hoe để đánh object, `3` chọn seeds để trồng ở ô đất nâu, `F` thực hiện. Khi cây đủ 5 giây, `F` thu hoạch. Ô vàng gần điểm xuất phát là item nhặt được bằng `F`. Đèn vuông góc phải phía trên xanh là thành công, đỏ là bị từ chối; thanh xanh bên trái biểu thị stamina. Các hiệu ứng particle/floating marker đang là hình học fallback, chưa có font và sprite riêng.

## 2. EventBus: công dụng hàm và tham số

`GameEvent` gồm `kind` (`ToolUsed`, `ObjectHit`, `CropPlanted`, `CropHarvested`, `ItemPicked`), `targetId` là ID nghiệp vụ ổn định, `position` là tọa độ thế giới và `amount` là số lượng dương.

| Hàm/biến | Đầu vào | Công dụng và đầu ra |
|---|---|---|
| `MaxPending=256` | Hằng số | Số event tối đa đợi xử lý; giới hạn bộ nhớ trong một frame. Khi đầy, `Publish` trả false. |
| `Publish(event)` | Một `GameEvent` | Thêm event nếu còn chỗ và `position` hữu hạn, `amount>0`; trả bool. Luật chơi vẫn có thể thành công nếu hiệu ứng bị bỏ. |
| `Take()` | Không có | Trả toàn bộ event đang đợi và làm queue rỗng để xử lý trong frame. |
| `Pending()` | Không có | Số event chưa lấy; dùng cho test/telemetry. |
| `Clear()` | Không có | Bỏ event còn lại khi reset/thoát scene. |

Muốn thêm event mới: thêm tên vào `GameEventKind`, phát từ system nghiệp vụ sau khi điều kiện thành công, và thêm cách hiển thị ở `EffectPool::Consume` hoặc một consumer khác. Đừng phát event từ `Render`, vì render có thể chạy lại, đổi cỡ hay bị bỏ qua.

## 3. EffectPool: hàm, biến và giới hạn

`Effect` có `kind` (`Particle` hoặc `FloatingText`), `position`, `velocity` tính bằng world units/giây, `age`, `lifetime` tính bằng giây và `value`. Particle tự di chuyển `position += velocity*dt`; hết lifetime bị xóa. `FloatingText` hiện vẽ ô vàng; cần font để vẽ chữ thật sau này.

| Hàm/biến | Đầu vào | Công dụng và đầu ra |
|---|---|---|
| `Capacity=128` | Hằng số | Số effect đang sống tối đa. Tăng sẽ dùng thêm CPU/draw call; giảm làm effect thừa bị bỏ. |
| `Spawn(effect)` | Effect với vị trí/tốc độ hữu hạn, lifetime `>0` và `<=10` | Thêm nếu hợp lệ và còn chỗ, đặt age về 0; trả bool. |
| `Consume(events)` | Vector event vừa lấy | Tạo hai particle cho mỗi event, thêm floating marker cho nhặt/thu hoạch, cộng screen shake cho ObjectHit; tất cả vẫn chịu Capacity. |
| `Update(dt)` | Giây từ frame trước | Bỏ qua `dt` âm/NaN/vô hạn, clamp mỗi lần tối đa 0.1 giây, di chuyển, hết hạn và giảm shake. |
| `Active()` | Không có | Vector effect chỉ đọc để renderer vẽ; không giữ reference qua Update. |
| `Shake()` | Không có | Cường độ rung camera hiện tại; tối đa 4, giảm 12/giây. Render dùng bản sao camera nên không đổi vị trí gameplay. |
| `Clear()` | Không có | Xóa hiệu ứng và reset shake. |

Ví dụ muốn hiệu ứng lâu hơn: đổi `lifetime` trong `Consume` từ 0.45/0.55 thành 0.8 giây. Muốn ít particle hơn, bỏ một lần `Spawn`; muốn nhiều hơn, cân đối `Capacity` và đo draw calls. Không tăng pool vô hạn theo số event.

## 4. InteractionSystem: hàm và thông số

`InteractionActor` giữ `position`, vector `facing`, `stamina`, `cooldown` còn lại (giây), `inventoryCount`, `inventoryCapacity`, cờ `busy`, `uiCapturesInput`. `InteractionTarget` giữ `id`, loại `Object/Crop/Pickup`, `position`, `health`, `active`, `planted`, `ready`, `age`. Game thật nên đặt chúng trong component/system riêng và lưu bằng ID, không bằng con trỏ GPU.

| Hàm/biến | Đầu vào | Công dụng và đầu ra |
|---|---|---|
| `Range=100` | Hằng số world units | Khoảng cách tối đa từ tâm actor tới tâm target. Tăng cho phép dùng từ xa hơn; nên so với kích cỡ tile 64. |
| `CooldownSeconds=0.35` | Hằng số giây | Thời gian đợi sau một thao tác thành công. Giảm làm thao tác nhanh hơn. |
| `StaminaCost=2` | Hằng số điểm | Stamina trừ sau thao tác hợp lệ. `Tick` hồi 1 điểm/giây đến 20. |
| `Nearest(actor,targets)` | Actor và vector target có thể sửa | Tìm target `active` gần nhất, trong Range, nằm phía trước (dot hướng >=0.5); trả pointer hoặc null. Pointer chỉ dùng ngay, không giữ khi vector thay đổi. |
| `Use(actor,target,selectedItem,events)` | Actor, target hoặc null, item 0–3, event bus | Trả `InteractionResult`: Success hoặc lý do từ chối. Kiểm tra busy/UI, active, tọa độ, range, hướng, cooldown, stamina, đúng tool, seed/độ chín/capacity trước khi sửa state. |
| `Tick(actor,targets,dt)` | Actor, targets và giây/frame | Bỏ dt âm/NaN/vô hạn, clamp 0.1; giảm cooldown, hồi stamina và tăng age crop đã trồng; crop ready sau 5 giây. |

`InteractionResult` có `NoTarget`, `Inactive`, `TooFar`, `WrongFacing`, `Busy`, `Cooldown`, `NoStamina`, `InventoryFull`, `NoSeeds`, `WrongTool`, `NotReady`. Sau khi bị từ chối, không trừ stamina, không đổi HP/túi và không phát event.

Tool mapping trong demo: index `1` (phím `2`) là hoe để đánh object; index `2` (phím `3`) là seeds để trồng crop. Crop chín có thể thu hoạch bằng `F`; pickup không cần tool. Một object có health mặc định 2, hit lần thứ hai chuyển `active=false`; `WorldMap::SetObjectActive(gameplayId,false)` bỏ hình và collider. Ô crop đang được dùng một lần; sau thu hoạch `active=false`. Reset `R` tạo lại các target và stamina.

Để đổi tốc độ cây, sửa ngưỡng `target.age>=5.0f` trong `InteractionSystem::Tick`, đồng thời chỉnh các mốc `GrowthFrame(it->age,{1,1,1,2})` ở `DemoScene2D::DrawCrop`. Tổng bốn mốc là 5 giây. Để đổi giới hạn túi, đặt `InteractionActor::inventoryCapacity`; nếu tăng giới hạn mà không thêm UI đếm đồ, người chơi chưa nhìn thấy số lượng thực tế.

## 5. Tự áp dụng sang game khác

1. Định nghĩa actor, target và event bằng dữ liệu gameplay thuần CPU; quyết định rõ ai sở hữu HP, inventory và thời gian.
2. Viết hàm kiểm tra trước khi mutate: UI/busy → target → range/hướng → cooldown/stamina → tool → capacity. Test từng trường hợp từ chối không thay state.
3. Khi thành công, mutate đúng một lần và phát event. Consumer có thể phát âm thanh, VFX, quest hoặc gửi mạng; event không nên là nguồn sự thật cho HP/inventory nếu hệ thống chưa có giao dịch/rollback.
4. Cho effect pool capacity và lifetime. `Update` chỉ dùng dt hữu hạn, `Render` chỉ đọc.
5. Khi object bị hủy, cập nhật cả trạng thái render và collider theo gameplay ID. Nếu game có ECS, áp dụng lệnh destroy sau vòng duyệt entity.
6. Chạy test logic, smoke và kiểm tra bằng tay F ở gần/xa, hướng trước/sau, túi đầy, cooldown và reset.

Giới hạn hiện tại: UI chỉ là tín hiệu màu, không có thông báo chữ; inventory là số đếm, chưa có item stack/save; tương tác chỉ áp dụng cho vài target mẫu, chưa có raycast vật cản hay hệ stamina riêng; collision đầy đủ thuộc Phase E. `EventBus` chỉ là queue trong một frame, chưa phải mạng hoặc log bền vững.
