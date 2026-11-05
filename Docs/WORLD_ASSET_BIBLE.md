# MyFarmEngine — World asset bible

## Sau P0-002 — 2026-11-05

[11 lỗi direction/flip đã được sửa](TASKS/P0-002.md), Debug/Release 24/24 PASS. Ảnh và findings P0-001 bên dưới giữ nguyên làm evidence trước sửa. Chưa có artwork/layers/pivot authored mới; không đổi Original. An walk phải vẫn mirror phụ kiện, cần art review. [Clip board mới](Reviews/P0-002/clips-dark.png).

## Audit P0-001 — 2026-11-05

[Artwork/pivot audit](P0-001_ARTWORK_PIVOT_AUDIT.md) ghi quyết định từng character và evidence 68 frames. Không có bộ tám hướng/outfit ready; Linh/An chỉ prototype composite và style reference. 11 direction rows cần sửa metadata; Library0055–0060 opaque checkerboard, giữ needs_review. Ground pivot hiện là bbox bottom-center, chưa authored. Canopy proxy chưa alpha mask. Original/Export/runtime CSV giữ nguyên. Ghi chú kỹ thuật cũ bên dưới chỉ là lịch sử; filtering hiện có mipmaps từ Gameplay11.

## Cập nhật Gameplay11

Original/Export/CSV không thay đổi. [Character Standard](CHARACTER_SCALE_STYLE_STANDARD.md) định nghĩa scale/pivot, ghi NEEDS_ART_REVIEW cho artwork chưa đồng nhất. AssetTextures cache bounds từng frame; human/pet/icon dùng uniform aspect placement. Texture MIN hiện GL_LINEAR_MIPMAP_LINEAR, MAG GL_LINEAR; không biến pixel art thành painted art hoặc bổ sung chi tiết nền. Các chẩn đoán Phase F bên dưới giữ làm lịch sử trước Gameplay11.

Baseline 2026-11-04. Technical ready không đồng nghĩa art/animation đã hoàn chỉnh. History/concept/placeholder không tính là gameplay assets.

## Nguồn và pipeline thực tế

Gameplay/Data/Assets.csv có 204 records, 27 ready, 177 needs_review. Assets/Original/manifest.csv có 204 snapshots đã kiểm SHA256. Original bất biến, Workspace review, Export runtime; Art/Backgrounds/Library và manifests vẫn giữ nguyên. PrepareAssets copy-once/hash guard → AssetManifest/DecodePng → AssetTextures cache/GPU. CheckAssets kiểm hash. Chưa có AtlasBuilder hoặc frame review tool tích hợp.

| Bộ runtime | Kỹ thuật | Giới hạn |
| --- | --- | --- |
| world_phase_e | RGB 1536×1024 trải world 6144×4096 | Một nền baked, không phải tile/object artwork độc lập |
| world_garden/background_morning | Nền ready | Không chứng minh tương tác/dynamic lighting |
| Linh/An portraits | RGBA 1254×1254 | Một pose, không phải outfit layers |
| Human sheets | RGBA 1254×1254, 2×2, frame 627×627 | Bốn hướng theo CSV, một số mirror; chưa tám hướng/alignment QA đầy đủ |
| dog_walk | RGBA, 2×2 | Hướng any; không có eat/sleep/pet sheet riêng |
| turnip_growth | RGBA, 2×2 | Bốn giai đoạn; chưa water/wither/season hoặc ba crop |
| items_farm | RGBA, 2×2, bốn icon | Icon không đồng nghĩa tool gameplay |
| 8 history previews | ready/reference | Chỉ F2, không phải feature implementation |
| 177 Library records | Source decode được | needs_review, không tự động chuyển thành gameplay |

## Phân biệt nguyên nhân hình ảnh

- Artwork: Original/Library/SNHR_v11/Characters/character_0001/sprite.png là pixel art 164×219 ngay trong nguồn, sai định hướng hand-painted. GL_LINEAR không thêm chi tiết. Giữ source và needs_review.
- Filtering: Texture2D đặt MIN/MAG GL_LINEAR, wrap CLAMP_TO_EDGE. GL_NEAREST chỉ cho texture trắng 1×1 vẽ hình khối. Không có bằng chứng cấu hình nearest gây pixelation của ảnh cũ. Chưa mipmap; sprite downscale mạnh cần review chuyển động trước chọn giải pháp.
- Scale: nền lớn gấp 4 theo world units và zoom 0.5–2.5 có thể lộ texel khi phóng gần. Sprite người frame 627 px vẽ quad 156 units; anchor lấy từ portrait, chưa pivot/bounds riêng từng frame. Chênh lệch hình trong atlas có thể làm chân lệch dù grid hợp lệ.
- Alpha: ready sprites đã kiểm channel/range 0–255. Validator toàn ảnh chưa chứng minh từng frame có viền sạch. Các sheet đã xem cần review viền và chân trên nền sáng/tối; không tự key màu/xóa nền/ghi đè Original.
- Atlas: FrameUv hỗ trợ grid/margin/spacing; các sheet hiện margin/spacing=0. Chưa extrusion/gutter/per-frame bounds. Kiểm tra bleeding khi scale, không kết luận chỉ từ schema pass.
- Render ordering: footY sort/cull/tie sequence có regression. Hai canopy ellipse lấy mẫu lại nền có thể che chi tiết nền đi kèm; chưa mask alpha chính xác. Object bị tắt collider không xóa được hình baked.
- Shader: GLSL 330 texture*tint + ellipse alpha, chưa light/normal/shadow pass. Canvas pixel tests kiểm orientation/color/cover/mask state, không chứng minh mỹ thuật toàn cảnh.

## Contract mục tiêu, chưa là schema runtime

Original hand-painted cozy countryside, 3/4 top-down, không pixel art/ảnh chụp/3D render trộn lẫn. Mỗi asset có stable ID, provenance/source bất biến, export đã duyệt, world size, foot pivot, collision footprint, render layer/footY, shadow proxy và seasonal/time rules khi cần. Metadata frame thêm bounds/pivot/direction/layer compatibility/events theo ANIMATION_SPEC.

MVP: một farm, player idle/walk tám hướng, một outfit thay được, ba crop, cây chặt, một animal/NPC/door/shop, inventory, SQLite và day/night/shadow. Nhà/cây/đèn/nội thất cần sprite/state độc lập; không dùng một nền khổng lồ thay toàn world. Mở rộng chỉ sau slice ổn định.

Asset acceptance: source hash không đổi; validation pass; duyệt hình tĩnh/animation ở scale gameplay trên nền sáng/tối; không foot sliding/cắt frame/bleeding; đúng hướng/phong cách. Test fixtures phải ghi nhãn, không tính là art hoàn thiện.
