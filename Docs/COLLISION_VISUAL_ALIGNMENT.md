# Đường, rào và footprint hình ảnh — 2026-11-08

Dự án D:/my_code/MyFarmEngine. Không đổi Original hoặc source PNG. Đây là chỉnh geometry/model/presentation, không tuyên bố hoàn thiện artwork.

## Nguyên nhân và sửa

- Town collider cũ phủ cả vùng ảnh baked (clockhouse 1280×690, market 760×720). Nhà mới vẽ theo chiều cao 420 nên có tường vô hình. Collider mới chỉ phần chân tường 420×100, giữ center chân cũ cho town.
- Đài phun nước đã không render nhưng vẫn chặn 500×400: bỏ collider phantom, chưa bổ sung fountain gameplay.
- Cây apple có collider 160×160/120×130 và oak 72×88, đang chặn dưới cả canopy. Đổi thân 72×24, giữ nguyên chân giữa render và điểm tương tác cây. Đi phía sau tán được, thân vẫn chặn.
- Gate thu collider xuống 80×20, giữ foot; ảnh giữ aspect nhưng giới hạn bề rộng theo footprint, không kéo méo.
- Rào dọc từng dùng ảnh rào ngang lặp có khe, cả rào dài có một depth key. Nay panel 64 world units, mỗi panel có Y-sort/culling riêng, rails đi theo đúng trục và post chung hai đầu. Vẽ wood geometry có bevel màu dùng cùng Canvas; cần duyệt hand-painted art riêng sau.
- Đường xuân không lặp tile sỏi dày: dùng dải đất 48 trong tile 64, shoulders 2 lớp, nối theo láng giềng horizontal/vertical. Grass base một pass, đường một batch rectangle; không xen texture/rectangle từng tile. Ba palette review khác vẫn v4.
- Rào chuyển ra ngoài tim đường; phía bắc tách hai đoạn, khoảng mở x=3056..3152 (96 units) tại y=1248..1264. Đường dọc x=2560..2624 thông, rào tây x=2640..2656.
- Nhà farm đặt footprint x=2638,y=2160,w=420,h=100, chân (2848,2260), trong khoảng đất giữa đường; không che luống cũ (3000,1700) hay cắt đường tới luống.

## File tự chỉnh

| File | Vai trò |
|---|---|
| Gameplay/Data/Objects.csv | Footprint cây/cổng/đá; root render = (x+w/2,y+h), đổi kích thước nhớ giữ root khi cần |
| Gameplay/Data/StaticColliders.csv | Chân tường/rào và các lối mở. Không lấy kích thước nguyên ảnh làm collider |
| Gameplay/Data/VisualScale.csv | Chiều cao hình, độc lập physics. fence điều chỉnh chiều cao nhìn thấy; chia panel vẫn 64 theo lưới |
| DemoScene2D::DrawFencePanel/DrawStaticObject/DrawWorldObject | Render theo root; rào mỗi panel có depth riêng |
| DemoScene2D::Render | Dải đường theo topology, overlay collider F9, culling |
| CollisionSystem::MoveAndSlide | Substep tối đa 8 units; khoảng dừng sát collider có thể còn khe nhỏ trong một substep |

F9 vẽ viền đỏ đúng static/object collider, điểm chân actor và debug gallery. G hiện tile. B chuyển garden control, không thay collider/quyền sở hữu; chuột chỉ crop trong vườn. Menu vẫn chặn world click.

## Kiểm chứng

Tests bảo vệ canopy/roof-only walkable, thân/tường bị chặn, lối vào/rào dọc có đường đi, không tunneling, sliding, scene chuyển Idle khi bị chặn, Y-sort panel quanh actor. Regression farming/chuột/UI/save và hash Original vẫn chạy cùng suite.

Save giữ IDs, crop/inventory/stock được map sang vị trí canonical hiện tại. Save có vị trí người chơi nằm đúng trong footprint mới sẽ bị từ chối an toàn bởi kiểm CanOccupy; không tự ghi đè DB hoặc tự teleport khi load. Không ghi save SQL trong buổi kiểm tra hình.

Còn giới hạn: đường/rào là geometry prototype, chưa là artwork cuối; mép hồ vẫn tile collision với bờ mềm. Nếu đổi scale nhà/cây lớn phải duyệt collider bằng F9 lại, không tự suy collider từ alpha. Kiểm fallback baked legacy không thay cho review cảnh v4/Cozy đang dùng.
