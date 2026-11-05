# P0-001 — Artwork, pivots và readiness cho P2-001

## Sau P0-002 — 2026-11-05

[11 lỗi direction/flip đã được sửa](TASKS/P0-002.md), Debug/Release 24/24 PASS. Ảnh và findings P0-001 bên dưới giữ nguyên làm evidence trước sửa. Chưa có artwork/layers/pivot authored mới; không đổi Original. An walk phải vẫn mirror phụ kiện, cần art review. [Clip board mới](Reviews/P0-002/clips-dark.png).

2026-11-05. Repo D:/my_code/MyFarmEngine, branch feat/gameplay10-phase-f-depth-directional-harvest; audit trên baseline 0ca4128, working tree sạch khi bắt đầu. Không đổi runtime, ảnh, Assets.csv, VisualClips.csv hoặc trạng thái ready/needs_review.

## Kết luận

Audit lần này DONE; P0-001 tổng thể PARTIAL vì authored pivots, artwork và canopy masks chưa được tạo/duyệt. P2-001 BLOCKED ở acceptance artwork; phần controller/layer code vẫn TODO. Không asset nào hiện đủ để chứng nhận tám hướng + outfit thay thật.

Linh là ứng viên mẫu chính cho phong cách hand-painted cozy, An là mẫu phụ. Đây là đề xuất dựa trên hình đã xem, không phải art approval. Cả hai là sprite composite: da/tóc/nón/quần áo/túi đã dính trong PNG. Không thể bật outfit bằng cách lấy An thay Linh, cắt tùy ý hoặc mirror/rotate để giả đủ góc. Không tìm thấy source layer body/hair/clothes/pivot authored trong Assets đã kiểm.

## Phạm vi và evidence

Đã đọc toàn bộ 204 records/62 clips và coverage policies; kiểm ảnh 31 character records (13 ready + 18 Library needs_review) cùng dog ready: 32 inputs, 68 declared frames. Runtime ready gồm 14 textures/50 frames; Library 18 ảnh đang khai báo 18 frames. Không nhận 68 declared frames là 68 pose hợp lệ: Library 0055–0060 thực tế chứa nhiều hình bị cắt trong một record.

Script `python Scripts/AuditCharacterArt.py --output Docs/Reviews/P0-001` chỉ đọc input và tạo evidence dẫn xuất. Cần Pillow (đã chạy 12.3.0); không thêm dependency runtime. Alpha threshold >=32, scale 132 world units human/72 pet theo cùng max-height mỗi sheet như runtime. Contact sheets không phải screenshot OpenGL hoặc chứng nhận chuyển động liên tục; chúng không mô phỏng mipmaps/zoom camera. Board Library dùng nearest để nhìn cấu trúc pixel nguồn, không đề xuất đổi filtering game.

- [Asset decisions: từng ID](Reviews/P0-001/asset-decisions.csv) — 32 rows, review thủ công, không tự sửa manifest.
- [Frame metrics](Reviews/P0-001/frames.csv) — 68 rows, bounds, alpha, auto pivot, referenceHeight, world scale.
- [Input hashes](Reviews/P0-001/inputs.csv) — SHA256 32 PNG inputs của phép đo.
- [Runtime sáng](Reviews/P0-001/runtime-light.png) / [tối](Reviews/P0-001/runtime-dark.png) — tất cả 50 frames, raw source orientation.
- [Clip sáng](Reviews/P0-001/clips-light.png) / [tối](Reviews/P0-001/clips-dark.png) — 24 player idle/walk/action clips, frame đầu với flip_x thật trong CSV.
- [Library sáng](Reviews/P0-001/library-light.png) / [tối](Reviews/P0-001/library-dark.png).
- [Alpha edge samples](Reviews/P0-001/alpha-edges.png) — crop native pixel của Linh/An idle frame 0, phóng nearest 2×, composite sáng/tối.
- [Canopy proxy overlay](Reviews/P0-001/canopy-proxies.png) — ellipse đúng ForegroundTrees.csv đặt lên bản sao nền; không sửa nguồn.

## Asset dùng được ở mức nào

| Asset IDs | Dùng được hiện tại | Thiếu cho P2-001 |
| --- | --- | --- |
| player_linh_idle_dirs; player_linh_walk, _walk_up, _walk_left, _walk_right | Prototype bốn hướng / mẫu palette, silhouette, costume Linh | NE/SE/SW/NW thật cho idle + walk; body/hair/clothes riêng; authored root/pivots; sửa idle-left metadata |
| player_an_idle_dirs; player_an_walk, _walk_up, _walk_side | Prototype bốn hướng / mẫu An | Bốn góc chéo, layers/root; sửa idle-left và walk/run side flip |
| player_linh, player_an | Mẫu nhận diện và portrait | Một pose, không phải cycle hoặc layers; Linh chạm mép trên |
| player_linh_action_dirs, player_an_action_dirs | Tham chiếu pose action bốn hướng | Một pose/hướng, chưa windup/contact/recovery cycle; An action-left metadata sai; root bị ảnh hưởng tay vươn |
| dog_walk | Prototype pet một hướng any, bốn frame | Không phục vụ human/outfit; chưa các hướng hay sleep/eat riêng |
| library_snhr_v11_characters_character_0001_sprite … _0012_sprite | Chỉ reference, giữ needs_review | Pixel/chibi, khác anatomy/style với Linh/An, outfit baked, không có coverage tám hướng |
| library_snhr_v11_characters_character_0055_sprite … _0060_sprite | Loại khỏi gameplay hiện tại, giữ source/reference | Cả sáu alpha min=max=255: caro baked; nhiều pose bị cắt dính vào một frame; khác style, chưa grid/crop/alpha hợp lệ |

Linh/An có palette và nét painted tương đối gần nhau ở scale 132; portrait–idle–walk–action vẫn đổi tỷ lệ/pose/silhouette và phụ kiện. Cần artist duyệt model continuity, không dùng resize để nhận đã đồng nhất. Action An down thấp 519px trong sheet reference 610px: pose cúi thấp hơn là hợp lý, không normalize từng pose lên cùng chiều cao.

## Lỗi hướng xác nhận qua ảnh và code

[11 rows cần sửa](Reviews/P0-001/direction-findings.csv). DemoScene2D::SpriteFor mirror UV khi clip.flipX=true; validator hiện kiểm schema/coverage, không hiểu hình đang nhìn hướng nào.

- player_linh/npc_linh/player_an/npc_an idle left: source frame 1 đã nhìn trái, flip_x=1 làm quay phải. Đề xuất flip_x=0 (4 rows).
- player_an action left: source frame 1 đã vươn trái, flip_x=1 làm quay phải. Đề xuất 0 (1 row). Linh action left hiện đúng, flip_x=0.
- player_an walk/run và npc_an walk: sheet _walk_side nhìn trái; CSV hiện left flip=1 và right flip=0 làm đảo hai hướng. Đề xuất left=0/right=1 (6 rows).

Đây là lỗi metadata, không phải thiếu artwork hay lỗi renderer tự mirror. Trong lượt audit giữ CSV để findings có thể đối chiếu baseline. Task sửa riêng [P0-002](TASKS/P0-002.md) cần kiểm hướng thật và animation regression; không coi 24 tests pass là mỹ thuật/hướng đã đúng.

## Pivot và scale

Runtime MeasureSpriteFrames lấy bbox alpha>=32. PlaceSprite dùng x=(left+rightExclusive)/2, y=bottomExclusive; mirror cả pivot. anchor_x=0.5/anchor_y=1 trong Assets.csv là giá trị cell chung, không phải ground root được artist duyệt; sửa hai số đó hiện không thay authored placement vì draw dùng measured bounds. Không có per-frame/layer root/ground-contact data hiện hành.

Độ biến thiên auto pivot dưới đây là range giữa các frame trong cùng sheet nhân scale, không phải đo foot sliding lúc chơi; action_dirs/idle_dirs chứa các hướng khác nhau nên càng không gọi đó là jitter theo thời gian.

| Sheet | Range auto X / Y (world units) | Ý nghĩa |
| --- | --- | --- |
| player_linh_walk_right | 5.72 / 0.88 | Tâm silhouette thay theo bước, chưa root ổn định |
| player_an_walk_side | 5.40 / 0.45 | Mirror làm phụ kiện đổi bên; cần review asymmetry |
| player_linh_action_dirs | 16.31 / 0.66 | Tay vươn đổi tâm bbox giữa hướng |
| player_an_action_dirs | 16.45 / 1.51 | Tay/pose không đại diện ground root |
| dog_walk | 2.39 / 4.78 | Đáy chân/paw thay frame; không chứng nhận chân tiếp đất |

Không hardcode pivot mới bằng số đo bbox và gọi là authored. Artist cần đặt root theo body/ground plane, xác định contact chân cho từng pose; tất cả body/hair/clothes cùng canvas/root/timeline. Dùng reference stature của model xuyên sheets; không scale độc lập mỗi layer. Draw/cull/Y-sort vẫn dựa world feet; root thay đổi cần giữ collision/interaction không bị kéo theo animation.

19 edge flags gồm Linh portrait 1 + Library 18; 49 runtime frames còn lại không chạm cell edge ở threshold32. Edge flag không tự chứng minh clipping; Linh portrait top=0, cần source có headroom để xác định có mất mũ/tóc không. 0055–0060 opaque nên bounds toàn cell là đương nhiên. 0001–0012 có alpha binary, ảnh cắt sát; cần extrusion/headroom nếu dùng pipeline khác.

Các sample sáng/tối không cho bằng chứng halo nền caro ở runtime Linh/An; soft alpha không tự là lỗi. Kiểm atlas bleed bằng mipmaps/chuyển động chưa được duyệt toàn bộ trong audit tĩnh; không kết luận mọi viền đã sạch.

## Artwork, filtering, renderer và canopy

- Artwork: Library pixel/chibi và caro baked nằm ngay trong source. Thiếu góc/layers/clean source không sửa bằng filtering. Không key nền hoặc tạo art thay thế trong lượt này.
- Filtering: Texture2D dùng MAG GL_LINEAR, MIN GL_LINEAR_MIPMAP_LINEAR và generate mipmaps. Không tìm thấy đường runtime dùng nearest cho character. Atlas margin/spacing=0, chưa extrusion; bleed là rủi ro cần tái hiện, chưa xác nhận lỗi ở mọi sheet.
- Scale: world_phase_e RGB 1536×1024 phủ 6144×4096 (4 world units/source pixel); painted sheet 627px/cell được thu mạnh. Chênh lệch mật độ chi tiết là thật, không thể lấy thêm detail nền bằng upscale.
- Ordering: feet-based sort/culling có tests; silhouette root sai vẫn khiến người trượt thị giác dù model feet không đổi. Không thay renderer trong audit.
- Canopy: hai ellipse sample lại RGB background, gồm nền giữa lá và không bao phủ toàn tán. Overlay cho thấy khoảng nền/đường rào lẫn trong vùng proxy. Cần export canopy RGBA/mask riêng đã duyệt; không thể coi ellipse là leaf alpha chính xác. Chưa có phiên F7/F8 kiểm thủ công mới; foreground/render-order/canvas và game smoke đã chạy.

## Handoff P2-001 (đề xuất authoring, chưa là runtime schema)

1. Chọn Linh làm model pilot; chốt model sheet/tỷ lệ, palette, side phụ kiện, camera angle và reference stature trước khi vẽ thêm. An chưa cần mở rộng đồng thời.
2. Giao idle/walk thật N/NE/E/SE/S/SW/W/NW; hiện thiếu bốn góc chéo và side mirror còn sai. Baseline walk hiện 4 frames/hướng; đề xuất pilot giữ 4 để giảm scope, artist phải duyệt gait/timeline. Không tính rotate/mirror là art chéo.
3. Giao body, hair, base clothes và ít nhất một outfit đổi được; phần bị che phải có hình hợp lệ, order/hide theo hướng/pose. Một composite PNG không chứng minh có body dưới áo. Nón/túi là layer chỉ khi artist chuẩn bị, không tự cắt từ composite.
4. Bàn giao RGBA không checkerboard, source layer editable nếu có; per-frame direction/state/index, duration, root/ground contact, shared canvas/reference stature, visibility/order, padding/extrusion policy. Metadata này cần task loader/schema riêng, không giả đã được Assets.csv hỗ trợ.
5. Acceptance: đủ coverage thật + layer timeline/root alignment trên sáng/tối và nền world ở gameplay scale; đổi outfit idle/walk không lộ phần thiếu hoặc nhảy chân; không double-scale, không mirror phụ kiện sai; validator và collision/animation regression pass. Outfit ID serialize thuộc implementation P2; không chứng nhận SQLite đã có.

## Verification và giới hạn

Scripts/Build.ps1 -Phase phase-f -Configuration Debug: **24/24 PASS, 21.30s**, gồm assets, visuals, geometry, render-order, foreground, canvas, graphics smoke và Windows launch. Không sửa C++; không chạy lại Release trong lượt audit này (P1-003 baseline Release 24/24 là kết quả trước đó).

CheckAssets: 189 imported + 18 artwork files và **204 immutable Original** khớp SHA256. Audit đọc/đo 32 inputs, sinh 68 metrics và đã xem boards sáng/tối, clip orientation, alpha samples, canopy overlay cùng source idle và Library0055. Chưa duyệt động mọi animation, mọi zoom/mip level hay toàn bộ 177 Library assets. Không coi source integrity/schema pass là art-approved.
