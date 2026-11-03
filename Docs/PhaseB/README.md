# Phase B — animation riêng từng thực thể

Ngày hoàn thành: **03/11/2026**. Branch: `feat/gameplay10-phase-b-animation`.
Baseline là commit Phase A `0a8a1c9`; không sửa hoặc merge `main`.

## Phạm vi thực tế

Kế hoạch trong ZIP giả định dự án đã có ECS, Animator, VisualCatalog, crop/dog system và save game hoàn chỉnh. Source hiện tại chưa có các hệ đó; các component và `Gameplay/Save` vẫn là scaffold. Phase B vì vậy bổ sung một lõi animation CPU độc lập và tích hợp nó vào cảnh 2D hiện có, không nhập code cũ hoặc giả vờ có ECS/save đầy đủ.

Asset player đã duyệt hiện chỉ là ảnh 1×1. Các state đã hoạt động và frame UV được lấy qua metadata đã validate, nhưng hình player chưa có chuyển động nhiều frame cho đến khi có sprite sheet thật được duyệt. Crop và dog dùng fallback hình học vì asset tương ứng vẫn `needs_review` theo Phase A.

## Thành phần đã thêm

- `Engine/Animation/Animator.*`: timeline loop/non-loop, giới hạn frame, pause, snapshot và rebuild.
- `Gameplay/Graphics/VisualCatalog.*`: profile `idle`, `walk`, `run`, `action`, `hurt`, `sleep`; lookup theo state/hướng với fallback `any`.
- `Gameplay/Data/VisualClips.csv`: clip riêng cho hai player, NPC, dog và crop. File được chép cạnh EXE vào `Assets/Data`.
- `AnimationController`: mỗi thực thể giữ timeline riêng. Gọi lại cùng clip không restart; đổi state/clip restart đúng một lần.
- `FacingFromVelocity`: player chọn hướng theo trục vận tốc trội và giữ hướng cũ khi đứng yên.
- `GrowthFrame`: chọn stage cây theo tuổi game, clamp stage cuối và lọc tuổi/duration không hợp lệ.
- Scene: player chọn idle/walk/run; `Space` action; `H` hurt. NPC, dog và crop có state/timeline riêng. Khi mở gallery, scene không update nên frame animation được giữ nguyên.

Mọi `dt` âm, NaN hoặc vô hạn bị Animator bỏ qua. `DemoScene2D` cũng chuẩn hóa `dt` trước khi dùng cho movement/camera/tuổi game.

## Save/load rebuild

`VisualAnimationSnapshot` giữ profile, state, hướng, elapsed và pause. `AnimationController::Rebuild` tra lại clip trong catalog thay vì giữ pointer tài nguyên qua load, sau đó clamp/modulo elapsed theo loop mode. Test xác nhận state, hướng, frame và pause được khôi phục; elapsed NaN được đưa về 0.

Đây là rebuild trạng thái animation trong bộ nhớ. Repo chưa có serializer/save game chạy được, nên Phase B không tuyên bố đã lưu file save ra đĩa hoặc tương thích save phiên bản cũ.

## Build và chạy

```powershell
cd D:\my_code\MyFarmEngine
.\Scripts\Build.ps1 -Phase phase-b -Configuration Debug
.\Scripts\Build.ps1 -Phase phase-b -Configuration Release
.\out\build\phase-b-release\MyFarmEngine.exe
```

Có thể mở trực tiếp EXE bằng File Explorer; giữ thư mục `Assets` nằm cạnh EXE.

## Kiểm thử đã chạy

| Cấu hình | Kết quả | Nội dung |
|---|---:|---|
| Debug `/W4 /WX` | 10/10 | Toàn bộ asset, camera, time, animation, visual, lifecycle, smoke, canvas, fallback |
| Release `/W4 /WX` | 10/10 | Toàn bộ test như Debug |
| MSVC AddressSanitizer CPU | 4/4 | asset validation/catalog, Animator, VisualCatalog/controller |

Test animation bao phủ frame bounds, fps sai/NaN, loop, non-loop, không restart cùng clip, đổi state, đổi hướng, pause, `dt` âm/NaN/vô hạn, snapshot/rebuild và crop growth theo tuổi. Test catalog từ chối clip vượt bounds.

UBSan chưa chạy vì máy hiện không có Clang/WSL phù hợp. ASan chưa bao phủ toàn bộ đường OpenGL/GPU. Chưa benchmark FPS/draw call và chưa kiểm thử sprite sheet nhiều frame thật vì asset hiện có chỉ một frame. Dừng ở Phase B; chưa bắt đầu Phase C.

## Lịch sử commit Phase B

- `ed469eb` — `feat(animation): add safe timeline animator`
- `61d63c2` — `feat(animation): add per-profile visual catalog`
- `fc90032` — `feat(gameplay): drive entity visuals from animation state`
- `80f66e7` — `feat(render): select validated animation frame UVs`
- `9c9a00d` — `build(animation): add Phase B release and ASan presets`
