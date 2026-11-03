# Gameplay10 — Phase A: kiểm kê và pipeline asset

Ngày thực hiện: **03/11/2026**. Branch: `feat/gameplay10-phase-a-assets`. Baseline: `1b7432c`. Chỉ triển khai Phase A; chưa bắt đầu Phase B–E.

## 1. Đọc trước khi sửa và đối chiếu kế hoạch

Không có AGENTS.md trong repo, thư mục cha `D:/my_code`, gốc ổ D hoặc ZIP. Đã đọc README, CMake/preset, hai shader và toàn bộ mã do dự án sở hữu trong Engine, Game, Gameplay, Scripts, Tests; các file rỗng được ghi nhận. ThirdParty được giữ nguyên, không tuyên bố đã audit từng dòng của toàn bộ thư viện vendor.

`SourceInventory.csv` liệt kê **109 file baseline** từ Git: 49 file có nội dung và 60 file rỗng. Số file có nội dung không phải số tính năng hoàn thành.

ZIP có hai tài liệu, lưu nguyên nội dung trong `Plan/`. SHA256 ZIP nguồn:

```text
4BF6CFD3EA1FDDDC567C050FDD5567FB913DD7E540685DB67D8F074F2977C3C0
```

Kế hoạch mô tả bản có ECS dense registry, farm/crop/inventory/shop/save, Animator/VisualCatalog và companion. Repo thực tế là bản 0.2.0 có engine/OpenGL 3.3, camera và cảnh vườn mẫu; chưa có các hệ thống đó. Không nhập code/archive cũ, không chuyển OpenGL sang 4.6, không dựng thêm ECS hoặc gameplay để khớp một giả định sai trong tài liệu.

Profile/clip/binding baseline:

| Có sẵn | Nội dung thực tế |
|---|---|
| Assets/Library/import_manifest.csv | 189 entry được nhập từ archive, ghi nguồn và SHA256 |
| Assets/Art/manifest.csv | SHA256 của 4 PNG mới tạo |
| Assets/Library/SNHR_v11/entity_manifest.csv | 60 nhân vật/động vật, còn needs_visual_review |
| Assets/Art/Items/farm_items_atlas.json | Mô tả bốn vùng atlas; chưa có parser JSON runtime |
| PlayerSprite.cfg, Visuals.csv, VisualClips.csv, VisualBindings.csv | Chưa tồn tại |
| Gameplay/Data/*.json | Sáu file khung rỗng |

## 2. Kết quả kiểm kê ảnh

`AssetInventory.csv` là kết quả decode bằng C++/stb_image: **190 PNG**, không có lỗi decode trong kho hiện tại.

- 190 bản gốc được chụp lại trong Original, kèm manifest SHA256.
- 177 ảnh thư viện nằm trong Workspace để tiếp tục duyệt; giữ `needs_review`, không được upload GPU.
- 13 ảnh được kiểm chứng để export: 2 nhân vật tĩnh, 1 item atlas, 1 map, 1 nền và 8 ảnh lịch sử chỉ dùng xem tham khảo.
- Một atlas 2×2 chứa bốn vật phẩm không được coi là bốn frame animation. Nhân vật một ảnh vẫn là clip tĩnh 1×1.

```text
Assets/
  Original/       190 PNG snapshot + manifest.csv, không ghi đè
  Workspace/      177 bản làm việc cần duyệt, không được đóng gói runtime
  Export/         13 PNG đã qua kiểm tra kỹ thuật
  Shaders/        shader ngoài
Gameplay/Data/Assets.csv   metadata đầu vào cho parser
```

Các thư mục Art/Backgrounds/Library cũ vẫn được giữ để bảo toàn lịch sử, nguồn và checksum. Runtime mới không đọc ảnh từ các thư mục đó. Gói build mới chỉ chép Export, Shaders và Assets.csv; không đóng gói Original/Workspace. Bản build cũ có thể còn file dư từ cấu trúc cũ; dùng thư mục build Phase A mới khi đóng gói.

Kiểm tra PNG/alpha/grid không đồng nghĩa toàn bộ sprite đã đẹp hoặc có giấy phép phân phối thương mại. Không tự xóa nền, cắt sprite hay thay đổi pixel trong Phase A. `.gitattributes` giữ nguyên byte PNG/JPG và hai file metadata archive có checksum.

## 3. Manifest và kiểm tra hợp lệ

`Gameplay/Data/Assets.csv` dùng UTF-8, có thể có BOM. Header gồm:

```text
asset_id,kind,source,export_path,columns,rows,frame_width,frame_height,anchor_x,anchor_y,has_alpha,status,first_frame,frame_count,margin_x,margin_y,spacing_x,spacing_y
```

- ID phải duy nhất và chỉ chứa chữ thường/số/gạch dưới. Hai dòng trùng ID đều bị loại, tránh chọn tùy tiện dòng đầu/cuối.
- `kind`: character, animal, crop, environment, item, item_atlas, background hoặc reference.
- `source` phải ở Original; `export_path` phải ở Export. Chặn absolute path, drive, `..`, đường dẫn UTF-8 hỏng và đường dẫn canonical thoát khỏi thư mục cho phép qua liên kết.
- PNG được decode thật trên CPU; giới hạn file nén 32 MiB, ảnh giải nén RGBA 64 MiB, cạnh tối đa 8192.
- Grid phải khớp chính xác kích thước ảnh, có xét margin/spacing. Không tự chia làm tròn, không tự cắt viền.
- `first_frame + frame_count` phải nằm trong tổng số ô. `FrameUv` tính UV từ frame, margin và spacing; không cho đọc frame ngoài bounds.
- Anchor là tọa độ chuẩn hóa [0,1], từ chối NaN/Inf. Cảnh mẫu lấy anchor từ metadata.
- `has_alpha` mô tả alpha channel thật của PNG, không phải RGBA được decoder tự thêm. Sprite/công cụ có alpha phải có vùng nhìn thấy và vùng trong suốt; RGBA opaque vẫn hợp lệ cho background/reference.
- Chỉ `ready` mới được bật. `needs_review`/`disabled` dùng fallback; lỗi một ID không làm các ID hợp lệ khác bị vô hiệu. CSV sai header hoặc thiếu file làm catalog rỗng và game dùng fallback.
- CSV hỗ trợ field có dấu phẩy hoặc dấu nháy được quote/escape. Mỗi record trên một dòng; không hỗ trợ field nhiều dòng.

## 4. API và file chính

| File/API | Vai trò, đầu vào → đầu ra |
|---|---|
| Engine/Resources/ImageData.* / DecodePng | Path Unicode → RGBA CPU, kích thước, alpha và mã lỗi; không cần GL |
| Gameplay/Assets/AssetManifest.* / Load | CSV + Assets root → record theo ID và diagnostics |
| AssetManifest::Validate | Record → hợp lệ/không hợp lệ, path và ảnh đã decode |
| AssetManifest::FrameUv | Record + chỉ số tương đối → vùng UV hoặc nullopt |
| Gameplay/Assets/AssetAudit.cpp | Công cụ CLI kiểm kê/validate toàn catalog, xuất CSV và exit code |
| Gameplay/Graphics/AssetTextures.* / Load | ID → texture đã kiểm chứng hoặc nullptr; cache canonical path + grid/frame/margin/spacing |
| Texture2D::Upload | RGBA đã decode → texture GPU; không decode lại trong đường cache |
| AssetRoot/FindAsset | Chọn Assets cạnh EXE làm nguồn authoritative; ảnh trong Export |
| Scripts/PrepareAssets.ps1 | Tạo snapshot/Workspace/export ứng viên; không ghi đè Original hay bản làm việc/export đang có |

Gameplay không nhận GPU pointer trong component ECS — repo chưa có ECS. Texture/cache thuộc lớp visual; chỉ cảnh vẽ giữ con trỏ mượn, được bỏ trước khi cache hủy. Không thay đổi component/registry hoặc save trong Phase A.

Thiếu PNG, PNG hỏng, grid/alpha/status sai hoặc manifest thiếu sẽ log warning một lần theo ID và vẽ màu/hình khối thay thế. F1 hiện hiện placeholder cho năm ảnh thư viện chưa được duyệt; F2 vẫn xem ảnh lịch sử hợp lệ. Không lén đọc lại ảnh cũ ngoài Export để che lỗi của gói phát hành.

## 5. Cách dùng pipeline

PowerShell tại repo:

```powershell
.\Scripts\PrepareAssets.ps1
.\Scripts\Build.ps1 -Phase phase-a -Configuration Debug
.\Scripts\Build.ps1 -Phase phase-a -Configuration Release
.\Scripts\CheckAssets.ps1
.\out\build\phase-a-release\AssetAudit.exe --report Docs/PhaseA/AssetInventory.csv
```

`Build.ps1` mặc định là Phase A, gọi PrepareAssets trước configure. Lần đầu Prepare tạo catalog từ kho hiện tại; các lần sau giữ catalog đã sửa. Ảnh Workspace/Export đã có không bị ghi đè. Chỉ đổi `status` sang ready sau khi kiểm tra thủ công và xác nhận grid/anchor/alpha đúng. File tồn tại hoặc checksum đúng không đủ để tự phê duyệt sprite.

Thêm ảnh mới: giữ file nguyên gốc trong Original và ghi checksum vào Original/manifest.csv; làm việc trên Workspace; tạo PNG sạch trong Export; thêm ID và metadata thực vào Assets.csv; chạy audit, unit test rồi test render. Thay đổi pixel trong export không làm đổi nguồn gốc. Chưa có công cụ cắt sprite/animation tự động trong Phase A.

Mở game không cần CMD bằng nhấp đúp:

```text
D:\my_code\MyFarmEngine\out\build\phase-a-release\MyFarmEngine.exe
```

## 6. Kiểm chứng đã chạy

Baseline trước thay đổi: Debug, warnings-as-errors, **5/5 test đạt**. Bản Phase A cuối: MSVC 19.51 trên Windows, OpenGL/NVIDIA RTX 3050 Laptop GPU.

| Kiểm tra | Kết quả |
|---|---|
| Build /W4 /WX, Debug | Thành công, 8/8 test đạt |
| Build /W4 /WX, Release | Thành công, 8/8 test đạt |
| assets.validation | 37 kiểm tra: file thiếu/hỏng, PNG truncated, grid, frame bounds, alpha, anchor NaN/Inf, traversal, UTF-8 hỏng/Unicode hợp lệ, CSV quote/BOM/duplicate |
| assets.catalog | 190 PNG, ready=13, needs_review=177, errors=0 |
| engine.canvas | Pixel PNG, alpha, UV, camera, cache reuse, không upload needs_review, hủy texture |
| assets.fallback | Game đóng gói riêng vẫn chạy khi thiếu PNG, PNG hỏng, thiếu manifest và grid sai |
| engine.camera / time / lifecycle / game.smoke | Hồi quy phần nền và game thật đạt |
| MSVC AddressSanitizer | assets.validation + assets.catalog: 2/2 đạt, không báo lỗi bộ nhớ |
| CheckAssets.ps1 | 189 entry archive + 4 ảnh mới + 190 snapshot Original khớp SHA256 |
| UBSan | Chưa chạy: không có Clang/WSL cài sẵn; không tự cài công cụ/hệ điều hành |

Lệnh sanitizer trong Developer PowerShell có CMake/Ninja:

```powershell
cmake --preset phase-a-asan
cmake --build --preset phase-a-asan
ctest --preset phase-a-asan
```

ASan chỉ kiểm tra parser/decoder/catalog CPU, chưa phải toàn bộ ứng dụng GPU. Không tuyên bố đã qua UBSan hoặc đạt toàn bộ yêu cầu sanitizer của kế hoạch. Chưa kiểm tra nhiều GPU/DPI hoặc save cũ; repo chưa có hệ save chạy được nên ca tương thích save chưa áp dụng. Không thêm animation hoặc Phase B khi còn các giới hạn này. Thời gian CTest không phải FPS/benchmark.

## 7. Bàn giao

Thay đổi chỉ nằm trên branch Phase A, `main` được giữ nguyên. Các báo cáo source, ảnh và bản kế hoạch nằm cạnh tài liệu này; `TIEN_DO.txt` ghi trạng thái mới nhất. Dừng tại đây sau commit Phase A. Phase B cần quyết định clip thật cho từng đối tượng và bổ sung môi trường kiểm tra UBSan nếu yêu cầu Definition of Done nghiêm ngặt của ZIP được giữ nguyên.
