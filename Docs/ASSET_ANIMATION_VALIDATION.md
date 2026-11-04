# Asset–Animation Cross-validation — P1-001

Ngày 2026-11-05. Hoàn thành task kiểm tra dữ liệu; artwork approval, animation events và gameplay mới không thuộc thay đổi này.

## Implementation thực tế

Mở rộng VisualCatalog và AssetAudit hiện có, không tạo manager song song. VisualClip giữ profile/kind/state/sourceRow để diagnostics chỉ đúng ID và dòng CSV. ValidateAgainst là bước CPU-only, không tạo window/context, không sửa catalog hoặc các con trỏ clip đang dùng bởi AnimationController. Asset được decode/validate tối đa một lần mỗi ID trong một lượt cross-check.

AssetAudit mặc định chạy cả catalog ảnh và cross-check VisualClips/VisualCoverage. Có lỗi trả exit 1; tham số sai hoặc lỗi ghi report trả 2. CSV --report vẫn là báo cáo asset theo format cũ; diagnostics animation ghi stderr và summary riêng, không giả định các dòng asset valid nghĩa clip cũng hợp lệ.

## Contract version 1

Gameplay/Data/VisualCoverage.csv có 26 rows profile/state, khai báo chính xác 62 clips hiện tại:

schema_version,profile,kind,state,directions,pivot_policy,layer_policy

- schema_version=1. Version khác bị từ chối, không đoán cách đọc schema tương lai.
- kind player/npc tương ứng asset character; dog tương ứng animal; crop tương ứng crop. Kind phải nhất quán giữa contract, clip và asset.
- directions là danh sách down|left|right|up hoặc any cho state thật sự chỉ có một hướng/fallback. Mỗi hướng khai báo cần clip ĐÚNG hướng, không cho Any fallback giả là artwork bốn hướng. Không mix any với các hướng cụ thể.
- pivot_policy=alpha_bounds_v1, layer_policy=single_sprite_v1 ghi rõ cách render hiện có từ Gameplay11. Đây chưa phải authored per-frame pivots hoặc layered outfit; policy mới phải được triển khai trước khi nhận.
- Mỗi profile/state trong clips cần contract; direction ngoài contract bị báo lỗi. Duplicate rows/directions, thiếu state/hướng, header sai hoặc file thiếu đều fail.
- Bộ bốn hướng hiện tại vẫn hợp lệ. Không coi coverage bốn hướng là đã đạt mục tiêu tám hướng; tám hướng sẽ cần schema/runtime và artwork phù hợp trong task riêng.

## Frame semantics

Clip first_frame là chỉ số tuyệt đối trong atlas. total_frames phải bằng columns×rows của asset, không phải kích thước cửa sổ frame đang cho phép. Clip phải nằm hoàn toàn trong [asset.first_frame, asset.first_frame+asset.frame_count). Cửa sổ first_frame khác 0 có fixture positive và negative riêng để tránh validator từ chối atlas hợp lệ hoặc đọc frame ngoài vùng được cấp.

Kiểm tra unknown/not-ready asset, kind, actual PNG decode/alpha/grid/path/anchor qua AssetManifest hiện có, tổng frame và range. Một lỗi ví dụ: visual_row_2:hero|walk|down:asset=missing:unknown_asset. Lỗi thiếu hướng có coverage row, profile/state và tên hướng.

## Chạy lại

Từ Developer PowerShell hoặc qua Scripts/Build.ps1:

    out/build/phase-f-debug/AssetAudit.exe
    out/build/phase-f-debug/AssetAudit.exe --report out/build/phase-f-debug/assets-20261105.csv

Với dữ liệu riêng, truyền đồng bộ --root, --manifest, --visuals và --coverage. Nếu chỉ đổi manifest mà giữ defaults, các clip mặc định vẫn được kiểm tra với manifest mới; công cụ không tự bỏ qua animation để cho pass.

VisualCoverage là contract authoring/CI, hiện chưa được game đọc trong render loop và không cần deployment cạnh game EXE. Runtime vẫn dùng AssetTextures/fallback như trước. Không tự chuyển needs_review thành ready. Không ghi đè ảnh Original/Export hoặc sửa VisualClips.csv.

## Evidence 2026-11-05

- Scripts/Build.ps1 -Phase phase-f -Configuration Debug: build thành công, 21/21 CTest PASS (24.13 giây).
- Scripts/Build.ps1 -Phase phase-f -Configuration Release: build thành công, 21/21 PASS (10.15 giây).
- assets.visual_cross_validation: 97 checks, 0 failures; catalog thật và fixtures cho unknown/not-ready/disabled, kind, total/grid/window, missing direction/state, Any fallback, duplicate/version/policy errors, partial parse errors, UTF-8 BOM và controller stability.
- assets.visual_audit_cli: executable thật trả 1 và coverage_manifest_missing khi thiếu contract; không chỉ test library.
- assets.catalog: 204 assets, ready=27, needs_review=177, errors=0; 62 clips cross-check errors=0.
- Graphics regression/smoke/console tests đã chạy trong cả hai suite. Không mở phiên chơi thủ công mới vì task không đổi rendering/gameplay; visual observations Gameplay11 vẫn thuộc báo cáo ngày trước.
- CheckAssets: 189 imported files, 18 artwork và 204 immutable Original snapshots khớp SHA256. Không đổi ảnh/các CSV asset và clip cũ. Không chạy ASan hoặc benchmark trong lượt này.

## Tiến độ sau task

Engine/render/camera/input/collision cơ bản đã tích hợp. Gameplay11 runtime/console/demo fixes và P1-001 data validation đã xong trong phạm vi kỹ thuật. Game vẫn là prototype: animation bốn hướng, một plot turnip/hái táo, inventory bộ đếm, NPC/pet demo. Chưa hoàn tất vertical slice: ba crop/tưới/bán, inventory theo item, outfit/tám hướng, production NPC/animal/shop/door, SQLite, clock/lighting/shadows.

Tiếp theo P1-002: F tạo action, gameplay đổi đúng một lần tại animation impact, có cancel/retrigger/target invalid tests. P0 art review/pivots/foreground masks vẫn PARTIAL và có thể làm riêng; không bị validator đánh dấu art hoàn chỉnh.
