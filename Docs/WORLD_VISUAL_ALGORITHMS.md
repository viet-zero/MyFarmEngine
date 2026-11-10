# Phân bố có quy luật và hiệu ứng — 2026-11-10

Không có một thuật toán làm mọi artwork đẹp tự động. Bản này giữ layout gameplay có tác giả; chỉ sinh trang trí và chọn biến thể theo quy luật. Không random nhà/luống/NPC, không dùng resize để tuyên bố đồng nhất phong cách.

## Những gì đang chạy

1. **Bridson Poisson disk sampling** trong WorldDistribution.cpp: grid r/sqrt(2), active list, tối đa 30 candidate trong vành r..2r; seed 20261110. Cùng dữ liệu/build và seed giữ vị trí ổn định, khoảng cách tối thiểu được test; chưa là network lockstep đa nền tảng. Có guard đầu vào và cap 8192 điểm cho prototype.
2. **Rule-based placement/keepout**, CacheDecor: loại nước/đường, full visible bounds cộng 8 units không được đè collider. Cấm gần mục tiêu 80, gần cổng/cửa 110, và cấm một vòng ô quanh đường. Sau đó lọc mật độ theo zone (base farm .40, town .55, forest .85), nhân với trường mảng tần số thấp .45..1.0. Hash quyết định giữ/bỏ theo xác suất đã tính; vùng có cụm và khoảng thoáng, không rải đều toàn cảnh. Lọc không phá khoảng cách tối thiểu.
3. **Spacing theo ảnh thật**: max sprite height 32, width lấy aspect từ alpha bounds và cộng biên gió 2.5%; r=max(112,hypot(maxWidth,32)+16). Vì vậy thay ảnh rộng hơn sẽ tăng khoảng cách, không chỉ đổi một con số cho đẹp. Props vật lý vẫn có footprint riêng từ CSV.
4. **Cache/chunk**: sinh/lọc một lần khi Initialize, lưu 24 chunk của world 96×64. Render chỉ duyệt chunk nhìn thấy, không chạy sampler mỗi frame. Reset gameplay không xáo lại trang trí.
5. **Stable spatial hash**: seed + tọa độ quyết định kích thước, pha, vật liệu đường và biến thể gỗ; không RNG theo frame. Biến thiên địa lý hiện là trường sin tần số thấp, không phải implementation Perlin/Simplex.
6. **Quy luật sống động**: gió là sóng theo simulation time và vị trí, thêm lệch pha nhỏ từng cụm. Stretch chỉ quanh root cố định; không đổi vị trí/collider. Hồ giữ gợn sáng đã có. Cổng có tiến độ mở/đóng .25s; cửa nhà chuyển giữa hai ảnh cùng canvas bằng crossfade. Pause dừng simulation/animation.
7. **Topology đường**: nối theo láng giềng 4 hướng, vai đường giữ liên tục; texture dirt phủ nhẹ thành một pass cùng atlas. Blob 47-mask đã có cho v4; không nói rằng WFC hoặc toàn bộ 47-mask mới đã được triển khai trong Cozy spring.
8. **Y-sort/culling**: nhân vật/cây/nhà lấy chân; rào từng panel có depth riêng; atlas chỉ batch các draw liền nhau tương thích để không phá alpha order.

Không dùng WFC để xáo bản đồ hiện tại: nó cần bộ ràng buộc tile/asset được duyệt và có thể làm hỏng tuyến đi/plot/nhà đã thiết kế. Có thể áp dụng sau cho khu world mới, không phải thay thuật toán bằng tên gọi.

## Nguồn đã đối chiếu

- Robert Bridson, [Fast Poisson Disk Sampling in Arbitrary Dimensions](https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf), SIGGRAPH 2007: khoảng cách r, acceleration grid và active-list sampler.
- Amit Patel, [Making maps with noise functions](https://www.redblobgames.com/maps/terrain-from-noise/): tần số/biome, biến thiên theo mảng và phân bố cây; là cơ sở lựa chọn hướng, không sao chép toàn bộ map generator.
- [Blob tileset/Wang bitmask reference](https://www.boristhebrave.com/permanent/24/06/cr31/stagecast/wang/blob.html), bản mirror Guy Walker: quy tắc edge/corner; v4 mask hiện có giữ nguyên.

## Tên ảnh, role và hiệu ứng

Convention: category_subject_material_direction_or_state_vNN.png. Tên giúp author tìm đúng tài nguyên; tên file đơn lẻ không tự tạo AI/animation. Assets/Workspace/CozyV2/asset_roles.json là binding có kiểm tra, dùng bởi StageCozyAssets.py.

| File nguồn | ID runtime | Role/effect đang nối |
|---|---|---|
| env_road_dirt_spring_material_v02.png | road | Vật liệu dirt/biến thể wear ổn định |
| prop_fence_oak_horizontal_v02.png | fence | Panel ngang giữ aspect/root |
| Cùng ảnh fence, region UV | fence_post/fence_rail | Trụ/thanh rào dọc cùng vật liệu gỗ; không sửa source PNG |
| building_farmhouse_door_open_v02.png | house_open | Crossfade với props.house, cùng canvas 1448×1086 |
| CozyV1/meadow-v1.png | meadow | Decor theo Poisson/keepout và gió giữ chân |

Có thể đổi tên file bằng sửa trường file trong manifest, giữ logical id đã nối code. Role/effect là metadata mô tả binding hiện tại; đổi chuỗi effect không tự viết một shader/controller mới. Region rect dùng pixel trong ảnh source; thay artwork phải duyệt lại rect, root và alpha. Pipeline từ chối path ngoài thư mục, ID trùng, thiếu role bắt buộc, region ngoài frame hoặc canvas nhà hai state không khớp.

## Cổng/cửa và các giới hạn

Portals.csv định nghĩa id, collider_id, opening rect, owner, door flag và pick_rise. Chỉ hai portal đang tích hợp: gate.town.01 và door.farm.home. Dùng F khi trong tầm/hướng hợp lệ hoặc B + click trong vườn đúng chủ. Có cooldown, busy/UI ownership; không phụ thuộc công cụ đang chọn.

Collision chỉ mở khi pose mở hoàn tất. Khi cổng mở, phần trụ/bản lề 14 units hai mép vẫn chặn. Cửa chỉ mở hành lang rộng 80, phần tường kề giữ nguyên. Đóng bị từ chối nếu player/pet/NPC đang trong vùng đóng, kể cả avatar đang ẩn ở chế độ quản lý. Hình cổng thu theo bản lề chiếu 2D, cửa crossfade; chưa có animation skeletal/3D hinge.

Save v2 lưu trạng thái đích mở/đóng, load snap pose và kiểm vị trí actor trước thay model. Save v1 vẫn đọc được, portal mặc định đóng. Cổng đã bị phá trong save cũ vẫn vắng mặt, không hiện tương tác vô hình. Không có interior mới: cửa cho qua footprint mặt tiền, không chuyển scene vào nhà. Không tự ghi slot SQL của người chơi khi test.

Tự chỉnh: WorldDistribution.cpp (sampler), DemoScene2D::CacheDecor/Render/DrawFencePanel (mật độ/keepout/gió/material), WorldPortals.cpp (tiến độ và an toàn đóng), Portals.csv (geometry/picking/owner), asset_roles.json (ảnh/region). Cần rebuild/restart để cache cập nhật; Original chỉ đọc.

QA nhanh: F9 bật debug; V đến trước cổng, N đến trước cửa (chỉ có tác dụng khi debug/demo bật). F mở/đóng, B đổi mode rồi click. R reset test; J/L là save/load, không nhấn J nếu không định ghi slot cá nhân.

Còn cần duyệt: alpha/fringe một số artwork, đường nối ở mọi zoom, các palette mùa ngoài xuân, nội thất/âm thanh và gesture tương tác. Thuật toán kiểm bounds/spacing không thay art acceptance bằng mắt.
