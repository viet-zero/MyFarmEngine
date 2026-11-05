# Character Scale & Style Standard — Gameplay11

## Sau P0-002 — 2026-11-05

[11 lỗi direction/flip đã được sửa](TASKS/P0-002.md), Debug/Release 24/24 PASS. Ảnh và findings P0-001 bên dưới giữ nguyên làm evidence trước sửa. Chưa có artwork/layers/pivot authored mới; không đổi Original. An walk phải vẫn mirror phụ kiện, cần art review. [Clip board mới](Reviews/P0-002/clips-dark.png).

## Review P0-001 — 2026-11-05

Đã có [audit 32 inputs/68 frames](P0-001_ARTWORK_PIVOT_AUDIT.md) và [metrics](Reviews/P0-001/frames.csv). Standard 132/72 và alpha-bounds pivot vẫn là giải pháp prototype; không đủ cho layers. Cần authored root/shared reference stature; không suy pivot từ tâm tay vươn. Linh/An chỉ là ứng viên painted, chưa art-approved; không bộ nào đủ tám hướng/outfit. Metadata flip sai ở 11 rows được ghi task P0-002, chưa sửa tại mốc audit.

2026-11-04. Runtime standard, không phải chứng nhận artwork đã hoàn chỉnh.

## World scale và ground anchor

WorldMap vẫn tile=64 và world=6144×4096; không đổi tọa độ/collider/data cũ. HumanHeight=132 world units (khoảng 2.06 tile), PetHeight=72 (1.125 tile), khai báo tại Gameplay/Graphics/SpriteGeometry.h. Player và NPC dùng chung quy tắc, camera không scale UI.

AssetTextures đo bounds alpha>=32 của mỗi frame ngay lúc load, cache theo asset ID. Dùng chiều cao content lớn nhất TRONG CÙNG SHEET làm referenceHeight. scale=targetHeight/referenceHeight; cùng một scale X/Y, giữ aspect ratio nguồn và không kéo dẹt pose. Các frame cùng sheet không tự normalize riêng từng frame gây breathing. Hình cúi/nghiêng có thể thấp hơn targetHeight. Không resize/rewrite bất kỳ file PNG.

Ground pivot tạm thời: bottom-center của bounds alpha từng frame; mirror X cũng mirror pivot. Full quad bao gồm padding trong source nhưng content bottom đúng feet.y. Rendering/culling cùng SpritePlacement; Y-sort tiếp tục dùng world feet.y, collision footprint không phụ thuộc độ lớn hình. Bỏ sinusoidal bob giả khiến chân rời đất. Đổi frame không đổi simulation position. Dog không còn sleep bằng ép chiều cao quad.

Bounds alpha không thay thế pivot do artist đặt: tay vươn dài, bóng đã baked, lông/hat có thể làm pivot lệch ngang. Bước art tiếp phải cung cấp ground pivot/contact mask và reference stature theo model, không chỉ bounding box. Không dùng normalization để giả vờ sửa tỷ lệ đầu/thân/chân, perspective hoặc style.

## Style và review

Gameplay mặc định chỉ có player Linh và pet, đều thuộc bộ hand-painted đang ready. NPC An chỉ hiện trong demo opt-in. Không tự nạp Library needs_review vào gameplay. Gallery/history là mode debug riêng, không overlay vào game. Một portrait không phải bộ animation; Tab đổi nhân vật chỉ trong developer mode.

| Artwork | Review status | Lý do / hành động |
| --- | --- | --- |
| 14 ready character/animal textures, 50 frames | NEEDS_ART_REVIEW | Bounds kỹ thuật đã đo; cần artist duyệt anatomy, perspective, silhouette/pivot và continuity giữa sheets. Không tự đổi ready CSV thành art-approved |
| player_linh portrait frame 0 | NEEDS_ART_REVIEW | Alpha>=32 bounds (359,0,856,1250), chạm mép trên source 1254×1254; cần source/export có headroom nếu nội dung bị cắt |
| Linh/An action, hurt và sleep | NEEDS_ART_REVIEW | Action hiện một pose/hướng, hurt/sleep tái dùng portrait; không phải full action cycle |
| dog_walk | NEEDS_ART_REVIEW | Một hướng; thiếu sleep/eat/pet artwork. Runtime chỉ dùng idle/walk/run, không ép ảnh giả ngủ |
| Library character_0001 | NEEDS_ART_REVIEW | Pixel art 164×219 từ nguồn, khác phong cách; tiếp tục needs_review, không đưa vào scene |
| world_phase_e | NEEDS_ART_REVIEW | Nền RGB 1536×1024 bị trải ×4 world scale, props và độ chi tiết không luôn tương xứng với người; cần nguồn chi tiết/layers riêng, không thể phục hồi bằng filtering |

Báo cáo đo 50 frames: out/build/phase-f-debug/gameplay11-character-review.csv (generated, Git ignored). Một frame chạm cell edge tại threshold, không có nghĩa các frame khác tự động đạt mỹ thuật.

Texture MAG vẫn GL_LINEAR; MIN đổi GL_LINEAR_MIPMAP_LINEAR và generate mipmaps để giảm aliasing khi thu nhỏ. Không tăng chi tiết artwork hoặc giải quyết style; atlas chưa có extrusion/gutter nên cần duyệt bleed ở scale nhỏ và bổ sung padding trong pipeline nếu tái hiện. Source hash giữ nguyên.

## UI và acceptance

Hotbar chung: slot 64, gap 8, padding 12 screen units; scale đều cho viewport nhỏ, nằm trong viewport. Icon fit bounds content vào box với cùng tỷ lệ X/Y. Màu nền/viền/highlight thống nhất, stamina riêng, debug portraits chỉ F9. Index marks hiện là chấm, chưa có text/icon font hoặc inventory đầy đủ.

Acceptance đã kiểm bằng gameplay.sprite_geometry và gameplay.runtime: preserve aspect, mirrored foot center/bottom, frame isolation, stable scale trong sheet, responsive toolbar, overlay on/off, independent actor state. Visual Release đã quan sát player/pet, hai người, NPC walk/idle, F7/F8. Không tuyên bố duyệt toàn bộ animation hoặc artwork tám hướng.
