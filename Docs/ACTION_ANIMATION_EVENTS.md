# Animation impact và action transaction — P1-002

2026-11-05. Mở rộng Animator, VisualCatalog và InteractionSystem hiện có; không thêm animation/resource/save manager song song. Luồng F thật trong DemoScene2D không còn gọi Use để mutate ngay tại input.

## Hành vi gameplay

F kiểm tra mục tiêu gần/phía trước/line of sight, tool, cooldown, stamina và inventory, rồi tạo reservation. Start chỉ giữ action ID, target ID và selected tool; không trừ hạt/stamina, không cộng đồ, không giảm stock/health hoặc phát FX của thao tác.

Timeline chạy từ update kế tiếp. IMPACT tra lại target theo ID, kiểm owner token, target kind, thao tác gieo hay thu đã chọn, active/range/facing/LOS và tài nguyên hiện tại; chỉ khi hợp lệ mới áp dụng model và phát GameEvent. COMPLETE dẫn sang recovery 0.15 giây rồi giải phóng busy. Cooldown bắt đầu tại impact thành công, cùng lúc trừ stamina. Regen/crop growth bình thường vẫn chạy khi không pause.

| Phase | Quy tắc |
| --- | --- |
| Start | Reserve ID/target/tool, input frame chưa tiến timeline |
| Active | Tiến animation, chưa tác động model trước marker |
| Impact | Tiêu thụ token đúng một lần, kể cả khi revalidation thất bại |
| Complete | Animation kết thúc; không nhận Complete lặp để reset recovery |
| Recovery | Vẫn busy, khóa movement/facing, không replay impact |
| Finished | Thả owner; lệnh F mới có ID mới nếu cooldown/điều kiện cho phép |
| Cancelled | Hủy reservation; trước impact không mất tài nguyên, sau impact không rollback kết quả đã commit |

Phím **C** hủy interaction. Hurt debug, Tab đổi nhân vật, debug teleport, Reset và Shutdown hủy pending action. Item trên hotbar có thể đổi nhưng action đang chờ vẫn giữ tool đã chọn khi Start. Space/H vẫn là debug actions; marker từ idle/walk hoặc entity khác không được áp vào giao dịch người chơi.

Mất focus và preview/gallery pause simulation/animation/cooldown/growth, không tự hoàn tất giao dịch. Preview tiêu thụ gameplay taps để F/C/Space/Tab không bị replay khi đóng UI. Quay lại resume action tại timeline cũ; có thể C để hủy.

## Exactly-once và target lifetime

InteractionAction chỉ giữ stable targetId, không giữ pointer vào vector targets. Start cấp ID tăng đơn điệu theo InteractionActor; Scene.Reset giữ nextActionId nên callback cũ không trùng action mới. ID hiện scoped theo actor trong phiên offline, chưa phải global multiplayer ID.

Impact đối chiếu activeActionId và lastImpactActionId trên actor, cùng impactConsumed/phase trên action. Copy/stale action hoặc tua lại AnimatorSnapshot không thể cộng item lần hai. Thiếu/inactive target, ID trùng, kind/plant-state đổi, hết seed/stamina/stock, full inventory, sai tầm/hướng/LOS đều fail trước mutation. Không tái thử cùng ID sau impact đã thất bại.

GameEvent có actionId (legacy direct Use giữ 0); ToolUsed và event model cùng ID, là hook chung cho FX/audio về sau. EventBus không đủ chỗ cho toàn bộ events của một impact thì trả EventQueueFull trước khi trừ/cộng tài nguyên. FX hiện được EffectPool tiêu thụ cùng frame impact; chưa có audio backend. Consumer phải phân biệt kind và actionId, không bỏ các event khác kind chỉ vì trùng action ID.

Use vẫn là primitive immediate có kiểm tra, phục vụ test/caller cấp thấp cũ; runtime F chỉ dùng Start/Impact. Busy/active owner chặn Use bypass một action đang chờ.

## Animation marker API

AnimationClip thêm markers {name,time}; time tính bằng giây từ đầu clip, không phải absolute atlas frame. Markers phải có tên, time hữu hạn trong [0,duration], sắp theo time; cặp name/time trùng bị từ chối. duration=frameCount/FPS.

Animator::Update phát MarkerHits trong khoảng (oldTime,newTime], cộng marker 0 ở update dương đầu tiên. Pause, dt=0/âm/NaN/inf không phát marker. One-shot clamp ở cuối và không phát lại. Nhiều marker vượt qua trong một dt vẫn được báo.

Với loop, mỗi marker trả occurrences của các vòng bị vượt; số entry bị chặn bởi số marker, không tạo queue theo hàng triệu vòng. Đếm quá uint64 thì saturate. Danh sách theo thứ tự marker trong clip, không phải stream chronologically interleaved của mọi vòng; consumer cosmetic phải xử lý count có budget. Action gameplay bắt buộc non-loop và một IMPACT nên không phụ thuộc replay loop.

Payload marker sở hữu tên riêng, copy animator không tham chiếu tên trong actor khác. Span MarkerHits chỉ dùng tới Update/Play/Restore/Reset tiếp theo. Snapshot thêm started flag; Restore không phát lại marker đã nằm trong phần timeline trước. Một snapshot ở đúng loop boundary không lặp marker 0 đã phát.

## CSV tương thích

VisualClips.csv 10/11 cột hiện có giữ nguyên. Với Action legacy non-loop, loader tạo IMPACT tại 50% duration và COMPLETE tại duration. Ví dụ action một pose, FPS=5 có duration 0.2 giây, impact 0.1 giây. Đây là timeline tương thích kỹ thuật, không biến pose tĩnh thành animation nghệ thuật hoàn chỉnh.

Có thể thêm cột cuối events vào header 11 cột:

    profile,kind,state,direction,asset_id,first_frame,frame_count,fps,loop,total_frames,flip_x,events
    player_linh,player,action,down,player_linh_action_dirs,0,1,5,0,4,0,IMPACT@0.1|COMPLETE@0.2

Khi dùng header mở rộng, mọi row cần đủ 12 fields; state không có event để trống cột cuối. Action phải khai báo đúng một IMPACT có 0<time<duration và một COMPLETE tại cuối clip; không tự fallback nếu cột events đã khai báo nhưng trống/sai. Action loop, duplicate impacts, thiếu Complete, time/format không hợp lệ bị VisualCatalog/AssetAudit từ chối. Các generic marker khác được phép nếu có time hợp lệ.

Không đổi Assets.csv, VisualClips.csv hay PNG đang sử dụng. Cross-validator P1-001 tiếp tục kiểm 62 clips và coverage bốn hướng.

## Restore/save policy

AnimatorSnapshot chỉ lưu presentation timeline, không serialize transaction hoặc world. InteractionAction là transient: phải Cancel trước khi thay/restore world/inventory; Reset/Shutdown hiện làm điều đó. Token active/lastImpact trên actor bảo vệ cả timeline rewind trong phiên đang chạy. Chưa có API save/load game/SQLite; không nhận đã triển khai resume pending action từ đĩa.

## Kiểm thử và giới hạn

Build/test Debug và Release bằng Scripts/Build.ps1 -Phase phase-f -Configuration Debug/Release. Suite có 22 tests, gồm gameplay.action_transaction mới và các test engine.animator/gameplay.visuals/gameplay.runtime mở rộng.

- Marker: biên 0/cuối, nhiều marker/dt lớn, loop counts, pause, invalid dt, saturation, restore/restart, copy payload và metadata lỗi.
- Transaction: Start không mutate, vector reallocation, stable IDs, stale/copy/rewind không double apply, Complete/recovery, cancel trước/sau, mất target, sai kind/plant-operation, LOS/range/facing, hết seed/stamina/stock, full inventory/event queue.
- Scene/renderer thật: F ở vườn táo; pause/spam/movement lock; C/hurt/teleport/reset; đi qua lối vào farm để gieo/chờ/thu turnip, rồi nhặt item. Model/FX chỉ đổi tại marker, không lặp sau completion.
- Keyboard/UI thật qua callback: pending action freeze trong preview; F trong gallery không tạo interaction mới khi về gameplay. Smoke/console/pixel/collision/Y-sort tests tiếp tục được chạy.

Các input trace được inject vào cùng UpdateSimulation của game, không phải bản mô phỏng logic khác. Không có phiên chơi thủ công mới trong lượt này; regression graphics thực sự tạo OpenGL context. Chưa kiểm toàn loop thủ công ở nhiều frame rates hoặc chạy ASan/benchmark. Timestep hiện vẫn variable với clamp 0.1; fixed-step thuộc P1-003.

Art action hiện vẫn một pose/hướng, inventory vẫn bộ đếm, cây chặt vẫn cần art/state đầy đủ. Tám hướng/outfit, audio, day/night, SQLite và production NPC chưa hoàn thành.


## Kết quả bản cuối

Debug build thành công, 22/22 PASS (19.67s); Release build thành công, 22/22 PASS (9.95s). Log tại out/build/phase-f-debug hoặc phase-f-release/Testing/Temporary/LastTest.log. CheckAssets xác minh 189 imported files, 18 artwork và 204 Original snapshots khớp SHA256. AssetAudit/cross-validator hiện có giữ PASS. Không chạy ASan hoặc benchmark.
