# MyFarmEngine — Animation specification

## Farming actions — 2026-11-05

Plant/Water/Harvest now share the existing four-direction Action clip and IMPACT transaction; no watering-specific or new crop artwork was created. Operation/crop generation/binding/warehouse owner and capacity are revalidated. Pause freezes moisture/growth, render does not advance crop or deposit. P2-002B is logic completion only; eight directions/outfit remain art-gated. [Details](FARMING_HOME_STORAGE.md).

## Cập nhật P1-003 — 2026-11-05

Animation controller và impact transaction chỉ update trong simulation 60 Hz. Render(alpha) nội suy vị trí chân/camera, không update timer, animation frame, world hoặc events. Reset/teleport/viewport/pause snap presentation để tránh trượt qua map; không dùng chung timer giữa entity. [Fixed timestep](FIXED_TIMESTEP.md). Bốn hướng và artwork một pose cho action vẫn giữ nguyên.

## Cập nhật P1-002 — 2026-11-05

Animator đã có marker/time và MarkerHits; Action non-loop có IMPACT/COMPLETE, CSV cũ dùng midpoint/end fallback. F live dùng Start → Active → Impact → Complete → Recovery; model/FX đổi đúng một lần tại impact, revalidate target ID/resources. C hủy; preview/mất focus pause. [Contract, CSV và restore policy](ACTION_ANIMATION_EVENTS.md). Tám hướng/per-frame durations/outfit/audio chưa triển khai; các ghi chú “chưa events” bên dưới là lịch sử trước P1-002.

## Cập nhật P1-001 — 2026-11-05

VisualCatalog có cross-validation CPU qua AssetAudit, đối chiếu 62 clips với AssetManifest và contract VisualCoverage version 1. Kind/state/direction/frame range và pivot/layer policy được kiểm tự động. [Contract và semantics](ASSET_ANIMATION_VALIDATION.md): total_frames=columns×rows, range phải nằm trong asset window; Any không thay cho directional artwork. Runtime vẫn bốn hướng/single sprite, chưa animation events hoặc outfit layers.

## Cập nhật Gameplay11

NPC không còn fmod timer showcase; patrol chỉ khi demoMode+npcAI, walk khi thực sự đổi vị trí. Pet follow opt-in, bỏ sleep bằng đổi tỷ lệ. Player/NPC/pet controller độc lập; input mất focus bị clear, facing khóa trong one-shot. [Ground placement](CHARACTER_SCALE_STYLE_STANDARD.md) dùng alpha bounds/cache, không đổi model position theo frame. Bốn hướng/FPS/CSV giữ tương thích, chưa frame events. Phần hiện trạng Phase F dưới đây là lịch sử trước thay đổi này.

Audit 2026-11-04 dựa trên Animator.*, VisualCatalog.*, VisualClips.csv và DemoScene2D.cpp.

## Hiện trạng

Animator có firstFrame/frameCount/FPS/loop, timeline deltaTime, pause, Finished, giữ clip khi Select lặp, restart có chủ đích và snapshot RAM. VisualCatalog có 62 rows: profile/state/direction/asset_id/first_frame/frame_count/fps/loop/total_frames/flip_x. Hướng Any/Down/Left/Right/Up; FacingFromVelocity chọn trục trội, giữ hướng lúc đứng, chưa hướng chéo.

Linh/An: idle mỗi hướng một frame; walk bốn frame/hướng; run tái dùng walk ở 10 FPS thay vì 6. An walk-left mirror side atlas. Action một frame/hướng ở 5 FPS; hurt/sleep dùng portrait một frame. NPC đổi clip theo timer nhưng đứng cố định. Dog dùng atlas any; sleep frame 0 bị đổi chiều cao, chưa sleep animation. Turnip dùng GrowthFrame(age), không loop animation để tăng trưởng.

F gọi InteractionSystem::Use thay model/phát gameplay event NGAY, rồi mới Select action. EffectPool tiêu thụ event cùng update. EventBus này chưa phải animation frame events. Chưa duration từng frame, impact marker, cancel/recovery transaction, outfit layers hoặc transition blending. Vòng lặp hiện chưa fixed-step.

## Contract sẽ mở rộng, chưa triển khai

Mở rộng Animator/VisualCatalog hiện có, không tạo manager song song. Clip có stable ID; duration đều/không đều; loop/one-shot; direction; bounds/pivot từng frame; visual offset; marker timestamp/frame; transition/cancel policy. Đọc được CSV cũ hoặc có migration rõ ràng.

Coverage N/NE/E/SE/S/SW/W/NW bằng artwork đúng góc, không xoay sprite 45 độ. Idle/walk đủ tám hướng trước nhận MVP; run dùng walk chỉ là prototype. Sit/sleep ít hướng phải khai báo rõ.

Layers: Body, Face/Eyes, HairBack, HairFront, Top, Bottom, Shoes, Headwear, Accessory, Equipment, HeldItem. Đồng bộ timeline/frame/pivot, thứ tự theo hướng/pose, hide rules; outfit ID serializable. Tab đổi Linh/An hiện chưa phải thay outfit.

Action: Start → Active → Impact → Complete → Recovery. Start tạo action ID/target stable ID; Impact xác thực lại điều kiện và áp model đúng một lần, đồng bộ FX/audio. Chốt policy hủy, target mất, pause, retrigger, đổi hướng, dt lớn và restore. Không giữ con trỏ target có thể mất hiệu lực, không cộng item vì clip phát lại.

## Acceptance/tests

Giữ engine.animator/gameplay.visuals pass. Regression cho nhiều marker vượt trong dt, đầu/cuối/loop, pause/restart/restore, cancel/exactly-once và pending-action save policy. Test integration F → Impact → inventory/stock, không chỉ parser. Duyệt animation thật tám hướng, chân theo pivot, layer không lệch/cắt; action một frame chưa là thao tác chặt/tưới hoàn chỉnh.
