# MyFarmEngine — Audit có kiểm chứng

## Tích hợp bộ ảnh Đơn Giản v4 — 2026-11-08

Đã giải nén ZIP Downloads MyFarmEngine_Assets_DonGian_v4_Da_Hoa_Cay_DoVat.zip (1238 entries) vào AssetLibrary_Review/DonGian_v4, kiểm CRC. Giữ nguyên nguồn và Original. Thông tin chờ tải bộ mới trong mục lịch sử v3 phía dưới đã được giải quyết.

Scripts/StageV4Assets.py đóng gói 450 vùng ảnh: 104 vùng terrain mỗi mùa × 4 và 34 vùng nhân vật/đồ vật. Atlas có viền extrude 2 pixel, provenance SHA256; không resize ảnh nguồn. Dữ liệu ảnh local chưa đưa lên Git. Chạy `python Scripts/StageV4Assets.py --configuration Release` (hoặc Debug), cần Pillow và thư mục đã giải nén.

WorldArt nạp atlas có kiểm tra metadata, đủ 47 mask bờ nước và frame nhân vật. Scene dùng tile 64, cache lựa chọn terrain bốn mùa, culling và Canvas2D batching hiện có; truy vấn luống liền kề dùng chỉ mục ô. Thiếu bộ local thì dùng đường render cũ. T đổi bảng màu mùa để review, chưa phải mùa gameplay.

Nhân vật dùng chung bộ prototype tám hướng, giữ tỷ lệ và root (96,180) trên canvas 192×192, chiều cao chuẩn 132 world units theo reference stature chung. Input, va chạm, marker hành động và tương tác bốn hướng vẫn giữ logic hiện hành. Mỗi entity giữ controller riêng. Chỉ có hai frame walk mỗi hướng; idle là placeholder, hành động dùng pose idle; chưa có outfit layers. Không coi animation tám hướng hoàn chỉnh là DONE.

Đồ vật/nhà được ghép lên collision và thứ tự chân hiện có, chưa phải bản đồ art hoàn thiện. Cây/bụi có viền màu, cỏ/nước có họa tiết lặp, artwork và footprint nhà còn NEEDS_ART_REVIEW. Chưa ghép cây modular vì thiếu attachment point; không dùng resize để tuyên bố sửa phong cách.

Kiểm chứng: Debug và Release 27/27 tests, gồm mask 47 loại, tám hướng và pivot có tác giả; test OpenGL kiểm nạp atlas local. Release executable chạy --smoke-test, exit 0. Chưa có buổi chơi tay dài hoặc số đo FPS cho bản v4.

Tiếp theo: (1) review trực quan spawn/nhà/cây và footprint; (2) bổ sung action frames/outfit sau khi chốt artwork; (3) hoàn thiện mùa gameplay độc lập với phím review T.

## Nhập terrain local và thử đất khô/ướt — 2026-11-08

Đã kiểm ZIP MyFarmEngine_Master_v3_Terrain_4Seasons.zip có sẵn trong Downloads và nhập riêng nhóm 01_Terrain_Seasonal_v3 vào AssetLibrary_Review/Terrain_v3_Local. Nhóm có 67 tệp gồm 56 PNG terrain, 4 atlas, 2 preview và 5 tệp dữ liệu/tài liệu. IMPORT_HASHES.json ghi hash ZIP và từng tệp. Ảnh đã được decode/verify lúc nhập.

Đây là bộ Master v3 procedural đã tải trước đó. Tab Viết cho tôi có liên kết bộ Assets Đơn Giản v3 mới hơn, nhưng trình duyệt mở chat đang ở trạng thái yêu cầu đăng nhập. Tải bộ mới còn chờ truy cập phiên chat được xác thực.

Debug 27/27 PASS (20.18s), Release 27/27 PASS (9.70s). Pixel OpenGL kiểm đất khô/ướt có hiển thị khác nhau.

Bản thử local sử dụng hai tile 128×128 đất luống khô và ướt của mùa xuân. Script StageTerrainReview.ps1 sao chép hai ảnh vào Assets/LocalReview cạnh executable. Có thể chọn spring/summer/autumn/winter qua tham số Season để review hình. Việc đổi ảnh review chưa tạo hệ mùa gameplay.

Đã xem preview bốn mùa và world prototype. Cỏ/nước có họa tiết lặp rõ, bờ nước chưa đủ autotile và luống trong world preview có hiện tượng nhìn xuyên nền. Bản thử giữ nền màu đất dưới PNG alpha để thể hiện ô khô/ướt rõ hơn. Các phần cỏ/nước/đường cần duyệt thêm trước khi ghép bản đồ.

## Lưới thao tác 64×64 — 2026-11-05

Sáu luống đầu game nằm trên sáu ô liền kề của WorldMap. Đối tượng tương tác được lập chỉ mục theo ô khi dựng hoặc tải scene. Cùng một hàm xác định ô nhắm cho viền và phím F, chỉ xét hàng hoặc cột theo hướng nhìn, tầm với và đường nhìn. Khi chưa có mục tiêu, viền chỉ ô ngay phía trước. Ô có mục tiêu hiện viền vàng và gợi ý thao tác đối với luống và vật phẩm rơi.

G bật hoặc tắt đường lưới ở vùng nhìn thấy. Hình cây trong cụm luống thu nhỏ để vừa ô. Đường và nước dùng dữ liệu terrain hiện hành, vùng trồng hiện dùng các luống đã định nghĩa. Save cũ giữ cây theo ID, sáu luống mới vẫn tương thích bản lưu trước khi thêm cụm luống.

## Chuyển nông sản giữa kho và túi — 2026-11-05

Mở K trong vườn sở hữu. Phím 1–4 chọn củ cải, cà rốt, khoai tây hoặc táo. F rút một món vào túi và C cất một món vào kho. Bảng hiện số lượng túi/kho và hàng đang chọn. K đóng bảng. Rời vườn tự đóng bảng và kho chỉ chuyển hàng khi đúng chủ, có vật phẩm và nơi nhận còn chỗ. Bảng K giữ quyền xử lý phím chuyển đồ, nên F khi bảng mở tác động vào hàng đang chọn.

Bản lưu SQL dùng lại hai container hiện có, nên J lưu số lượng sau khi chuyển. Runtime test đi qua input buffer để rút rồi cất lại. Logic test kiểm túi đầy và ngoài vườn giữ nguyên kho.

## Ô trồng nhìn thấy và gieo từ điểm xuất phát — 2026-11-05

Sáu ô trồng 2×3 nằm phía dưới điểm xuất phát. Mỗi ô có viền và nhãn O TRONG, CAN TUOI, DANG LON hoặc DA CHIN. Viền sáng và gợi ý phím chỉ ô trong tầm tương tác theo hướng nhìn. Vật phẩm nhặt có nền và nhãn VAT PHAM. Đây là lớp ô tương tác đặt trên ảnh bản đồ, với các trạng thái thật của từng plot.

Từ lúc vào game, chọn 3 rồi nhấn F để gieo vào ô ngay phía trước. Chọn 1 rồi F để tưới, F để thu khi chín. Ô cũ vẫn dùng được. Bản lưu trước khi có sáu ô mới được tải với sáu ô khởi tạo trống. Runtime test kiểm gieo ngay tại spawn và tải bản lưu kiểu cũ.

## Bản thử ảnh cà rốt từ 04_Curated — 2026-11-05

Bản thử lấy ba frame cà rốt sprout, young, mature và hai icon carrot, seed_bag. Hai frame seed và harvestable của bộ nguồn bị cắt hình. Trong game, giai đoạn đầu dùng sprout và cây chín dùng mature kèm icon cà rốt. Ba frame 192 px có cùng đáy alpha y=180, được căn với điểm luống. Icon túi hạt dùng ở ô 3, icon cà rốt xuất hiện trong menu M.

`Scripts/StageCarrotReview.ps1 -Configuration Release` sao chép năm PNG vào Assets/LocalReview cạnh executable sau khi build Phase F. Bản Debug dùng `-Configuration Debug`. Mã chọn hình theo CropId và có fallback khi không có gói review. Tệp ảnh và cả thư viện review nằm tại máy người dùng để chờ xác nhận quyền phân phối.

Test runtime đọc pixel OpenGL và kiểm cây cà rốt non/chín cho hình khác nhau. Pass giao diện menu thử dùng tối đa hai draw call. Kiểm hash 204 Original đạt.

## P4-002A — Lưu và tải SQL Express local — 2026-11-05

**J** chụp snapshot và lưu, **L** tải snapshot trong gameplay khi menu mua bán đã đóng và thao tác hiện tại đã kết thúc. Luồng nền chạy ODBC qua Windows Authentication tới SQL Express local. Tải tạm dừng mô phỏng trong lúc chờ. Game kiểm phiên bản, định dạng, target và vị trí trước khi áp dụng dữ liệu. Khởi động lại game rồi nhấn L để tiếp tục bản lưu.

Snapshot v1 lưu tiền, vị trí, hạt đang chọn, vật phẩm trong ba lô và kho, vùng/chủ kho, cây, táo và trạng thái các đối tượng tương tác. Inventory được dựng lại theo loại và số lượng. NPC/pet, animation và các bộ đếm tạm khởi tạo lại từ scene. Phạm vi là một slot thủ công. Công cụ tự lưu, migration và phục hồi bản dự phòng từ UI còn ở backlog.

Database MyFarmEngineLocal đã được tạo trên .\SQLEXPRESS. Slot 1 dành cho game và slot 2 dành cho kiểm thử. Mỗi UPDATE lưu snapshot mới và bản trước trong cùng câu lệnh. Kiểm SQL thật chạy ghi/đọc bằng hai tiến trình Debug và đọc lại bằng Release. Một UPDATE vi phạm ràng buộc trên slot 2 bị từ chối, dữ liệu trước đó vẫn đọc được. Slot 1 còn trống sau test. Runtime test kiểm khôi phục snapshot vào scene OpenGL và từ chối snapshot thiếu target.

Debug và Release 27/27 PASS, gồm codec, scene restore và smoke. 204 Original khớp SHA256. Việc nhấn phím J/L trong phiên chơi thủ công còn cần kiểm tại máy người chơi.

Máy mới chạy Scripts/SetupLocalSave.ps1 một lần với SQL Express .\SQLEXPRESS và ODBC Driver 18. Xem [task lưu game](TASKS/P4-002.md).

## P2-002C — Giao dịch và giảm chi phí render — 2026-11-05

Mở menu bằng M. Dùng 1–4 chọn củ cải, cà rốt, khoai tây hoặc táo. F mua một hạt và gắn vào ô 3. Space gắn hạt đã có. C bán một sản phẩm từ túi, K bán từ kho khi ở trong vườn sở hữu. M đóng menu. Đây là menu prototype mở ở mọi vị trí, với 20 tiền ban đầu và giá mua/bán 3/6, 5/10, 7/14 cho ba loại cây. Táo bán 4. Mỗi tick xử lý tối đa một lệnh giao dịch đã buffer. Tiền và vật phẩm thay đổi cùng nhau sau kiểm tra giới hạn, quyền kho và request ID.

Canvas dùng instancing OpenGL 3.3 cho các quad liền kề cùng texture và mask, giữ thứ tự alpha/Y-sort. HUD gộp nét ngang theo hàng, nền ảnh dùng một quad và nền dự phòng chỉ duyệt tile trong tầm nhìn. EffectPool đọc trực tiếp event span rồi clear để giữ dung lượng buffer. Tìm tương tác lọc khoảng cách trước góc và chuẩn hóa hướng một lần. Plant/harvest tái sử dụng kiểm tra all-or-nothing của Inventory, thao tác trên đúng container thay vì copy cả hai.

Debug 26/26 PASS (20.39s), Release 26/26 PASS (11.31s). Kiểm thử OpenGL đo 1.050 quad liên tiếp thành 2 draw call. Menu mẫu trong runtime có 1 draw call ở pass màn hình. Đây là số lệnh gửi GPU, chưa có phép đo FPS/frame time. CTest kiểm cả màu alpha, mask, mua–trồng–tưới–thu–bán kho–mua tiếp, chống replay, full bag, thiếu tiền/đồ, overflow và biên quyền kho. 204 Original khớp hash.

Tiếp theo là snapshot lưu tiền, ba lô, kho và cây qua SQL Server. Dữ liệu gameplay hiện nằm trong RAM. Collision vẫn duyệt các tập collider hiện tại và sẽ cần profiling khi mật độ đối tượng tăng.

## P2-002B — Cây, kho vườn nhà và HUD gọn — 2026-11-05

**DONE kỹ thuật trong phạm vi slice.** Plot củ cải thật có gieo → khô → tưới → tăng trưởng → chín → thu → gieo lại trên cùng ID. CropId/seed/produce definitions cho turnip/carrot/potato đã kiểm đủ logic; scene hiện chỉ có starter/plot củ cải, chưa UI mua/chọn hai loại hạt kia. Cây chỉ lớn khi ẩm; thời gian dev 5/8/12s, mỗi lần tưới cấp 6s ẩm. Dùng fixed simulation; pause giữ age/moisture.

**Yêu cầu mới về kho đã tích hợp:** nông sản/táo thu khi cả actor và target ở trong vùng sở hữu gửi thẳng vào HomeStorage 48 ô, không qua ba lô. Bag đầy vẫn thu được nếu kho còn chỗ; kho đầy giữ nguyên cây/stock, không fallback hoặc mất đồ. Ngoài vườn/không đúng owner thì vào bag; ground pickup vẫn bag. Region prototype x=[1400,3840), y=[640,2304), owner player local; chưa hệ đất/nhà hoặc multiplayer authority.

**UI theo lựa chọn 1+3:** 4 ô hotbar, tự ẩn sau 4s không thao tác (fade 0.6s cuối); di chuyển/đổi ô/F/C/K hiện lại. K mở/đóng bảng số lượng kho chỉ đọc trong vườn; ra ngoài tự đóng. Không vẽ lưới bag/kho thường trực, chưa rút/chuyển/sắp đồ. Labels ASCII prototype, không tạo font/image asset mới.

**Debug 26/26 PASS (20.84s), Release 26/26 PASS (10.50s)** bản cuối. New gameplay.farming kiểm 3 crop IDs, dry/wet/re-water/harvest/replant, partial wet tick, tool/cancel/revision, kho đầy/owner/biên/queue/replay, trace 30/60/144 Hz (840 ticks) và HUD timer. Runtime đi tuyến thật trồng/tưới/thu vào kho/trồng lại; OpenGL pixel tests kiểm hotbar ẩn/hiện, K panel và input chỉ toggle một lần qua catch-up; smoke/Windows launch regressions pass. Không manual extended-play hoặc art approval mới.

**204 Original + imported/art SHA256 không đổi.** Không đổi ảnh/VisualClips/Assets.csv. Kho/cây/bag vẫn RAM, Reset hoặc tắt game chưa bảo toàn; SQL preflight từ P2-002A không phải save/load. [Contract/cách chơi/giới hạn](FARMING_HOME_STORAGE.md), [task](TASKS/P2-002B.md).

P2-002 tổng thể PARTIAL: kinh tế/mua bán, lựa chọn 3 hạt ở UI, cây chặt/drop, lưu game còn thiếu. Tiếp [P2-002C](TASKS/P2-002C.md): tiền, mua hạt và bán từ đúng bag/kho; sau đó SQL snapshot/save/load có roundtrip/rollback để giữ cả kho và trạng thái cây. Artwork tám hướng/outfit làm sau, không chặn logic. Các mốc dưới là lịch sử.

## P2-002A — Inventory thật, logic-first — 2026-11-05

**DONE kỹ thuật trong phạm vi inventory core.** InteractionActor sở hữu một bag 12 ô với stable ItemIds, stack limits, add/remove all-or-nothing. Có watering can x1, axe x1, turnip seeds x3 ban đầu; tools max-stack 1. Gieo chỉ trừ seed.turnip tại impact; thu nhận crop.turnip, orchard nhận fruit.apple; pickup dùng item ID. Hotbar kiểm ItemId và tool ownership, số lượng lấy trực tiếp từ bag. Không còn writable inventoryCount/inventoryCapacity; SceneSnapshot.inventoryCount chỉ là resource count suy ra, không tính tools.

**Debug 25/25 PASS (22.40s), Release 25/25 PASS (10.97s)** trên bản cuối. Test mới gameplay.inventory kiểm full bag/matching stacks, atomic failure, IDs/amount invalid, apples không làm seed, tool mất hoặc binding/reward đổi trước impact. Live scene kiểm starter/apple/crop typed quantities; action replay, fixed-rate 30/60/144, graphics/smoke/Windows launch regression đều pass. Không có manual extended play/duyệt pixel UI số trong lượt này. 204 Original và imported/art SHA256 giữ nguyên.

**SQL máy người dùng đã kiểm thực tế:** MSSQL$SQLEXPRESS chạy; Windows Authentication connect .\SQLEXPRESS/master và SELECT SERVERPROPERTY thành công: Express Edition 64-bit, 17.0.1000.7. Driver ODBC 17/18 có trên máy. Chỉ probe .NET đọc thông tin, chưa C++ SQL integration, chưa tạo database/tables hoặc save/load; không đổi network/service. [Contract và SQL evidence](TYPED_INVENTORY.md).

P2-002 tổng thể **PARTIAL**; chưa watering/ba crop/money/buy-sell/bag-grid UI. Existing icons giữ nguyên (seed slot vẫn placeholder icon cũ), cake/watering/wood definitions không đồng nghĩa đã có use/drop. P2-001 chờ artwork của riêng task đó, không chặn logic nữa. Tiếp theo [P2-002B](TASKS/P2-002B.md) plot tái sử dụng + crop IDs/tưới/growth → giao dịch tiền/mua/bán → persistence pilot có roundtrip/rollback trên SQL đã cài khi cần. Không triển khai cả roadmap trong một lượt.

## Cập nhật P0-002 — 2026-11-05

**DONE kỹ thuật:** sửa đúng 11 fields flip_x trong 62 clips, không đổi frame/timing/asset IDs. Idle trái Linh/An (cả NPC) và action trái An không mirror nữa; side walk/run An dùng left không mirror, right mirror. Regression kiểm các clip đã review và walk → idle giữ hướng; chạy test mới với catalog trước sửa trả exit 1 đúng lỗi.

**Debug 24/24 PASS (23.17s), Release 24/24 PASS (11.36s)**, gồm graphics/runtime/geometry/fixed-rate/animation/smoke/Windows launch. Đã xem clip board sau sửa ở scale chuẩn; đây là contact sheet, không có manual gameplay soak mới. 204 Original và imported/art hashes khớp; không sửa PNG, Assets.csv hoặc engine/renderer.

[P0-002 và ảnh trước/sau](TASKS/P0-002.md). P0-001 vẫn PARTIAL do authored pivots/artwork/canopy mask; P2-001 vẫn BLOCKED artwork acceptance. An walk phải còn mirror phụ kiện; sửa flip không tạo art tám hướng hoặc layers. Các findings CSV P0-001 giữ nguyên làm evidence lịch sử, không phải danh sách lỗi còn mở.

Tiếp theo: chốt Linh model/reference stature/root → bàn giao và duyệt diagonals + body/hair/clothes/outfit layers → triển khai loader/controller P2-001 với artwork đủ điều kiện. Không tính prototype là MVP hoàn chỉnh. Các mốc dưới là lịch sử.

## Cập nhật P0-001 artwork/pivots — 2026-11-05

**Audit DONE; P0-001 tổng thể PARTIAL.** Đã xem/đo 31 character records + dog: 32 inputs, 68 declared frames; có bảng từng asset và ảnh sáng/tối. Linh là mẫu painted chính đề xuất, An mẫu phụ; chưa asset nào đạt tám hướng/outfit. Runtime vẫn composite bốn hướng, không có authored pivots hoặc layers. Library 0001–0012 sai style; 0055–0060 alpha 255 toàn ảnh, caro baked và nhiều pose cắt dính. Giữ needs_review.

Phát hiện 11 rows direction/flip sai (idle-left Linh/An, action-left An, walk/run side An); chưa sửa CSV trong audit, chuyển [P0-002](TASKS/P0-002.md). Auto pivot action đổi X khoảng 16 world units giữa hướng; không gọi đây là đo jitter động. Canopy vẫn ellipse sample nền, cần RGBA masks.

**Debug 24/24 PASS (21.30s)**; đã chạy graphics/smoke, chưa có manual animation soak hoặc Release mới trong lượt này. 204 Original và hash imported/art khớp, không sửa ảnh/asset CSV/runtime. [Báo cáo + evidence](P0-001_ARTWORK_PIVOT_AUDIT.md).

**P2-001 BLOCKED về artwork acceptance**, code vẫn TODO; Gameplay11/P1-001/002/003 đã xong kỹ thuật. Ba việc tiếp: sửa metadata P0-002 → chốt Linh model/root và bàn giao art tám hướng/layers → triển khai loader/controller outfit khi đủ art. MVP/farm/SQLite/lighting vẫn chưa hoàn tất. Các mốc bên dưới là lịch sử.

## Cập nhật P1-003 — 2026-11-05

**DONE kỹ thuật:** simulation 60 Hz tách khỏi render; input được lấy một lần/frame, giữ tap qua frame chưa có tick và chỉ consume một lần qua catch-up. Nội suy chân player/NPC/pet và camera dùng cùng vị trí cho draw/culling/Y-sort. Animation/impact chỉ tiến trong fixed update.

**Debug 24/24 PASS (22.51s), Release 24/24 PASS (11.19s)** qua Scripts/Build.ps1, gồm graphics, executable smoke, Windows launch, native minimize/restore và repaint. Trace gameplay 6 giây ở nhịp render giả lập 30/60/144 Hz đều có 360 ticks, cùng vị trí/state/animation và hai lần hái táo; render không đổi model hoặc phát impact. Không phải benchmark FPS; chưa có phiên chơi thủ công kéo dài trong lượt này. 204 Original khớp SHA256, ảnh và dữ liệu asset giữ nguyên.

Tiến độ hiện tại: Gameplay11 và P1-001/002/003 DONE trong phạm vi kỹ thuật đã kiểm. Game vẫn là prototype bốn hướng, action artwork một pose; inventory bộ đếm, một turnip plot và orchard. P0 art review PARTIAL; tám hướng/outfit, ba crop/tưới/bán, NPC/shop hoàn chỉnh, SQLite và day/night/lighting chưa xong. Không coi P1 hoàn tất là MVP hoàn tất.

Ba việc tiếp: P0-001 duyệt artwork/pivots → P2-001 chuẩn bị đủ art tám hướng/layers rồi triển khai controller → P2-002 typed inventory và vòng farm. Chi tiết giới hạn thời gian/pause/interpolation: [Fixed timestep](FIXED_TIMESTEP.md). Các cập nhật bên dưới là lịch sử.

## Cập nhật P1-002 — 2026-11-05

**DONE kỹ thuật:** animation markers và action transaction đã tích hợp vào luồng F thật. Start chỉ reserve; IMPACT mới kiểm tra lại target ID, tool/state, LOS/range/facing và tài nguyên rồi đổi inventory/stock/health và phát FX cùng actionId. Stable token chặn bấm dồn, copied action và timeline rewind cộng đồ hai lần. C hủy; hurt/teleport/reset/shutdown cancel; preview hoặc mất focus pause.

**Debug 22/22 PASS**, **Release 22/22 PASS** ở bản cuối (19.67s / 9.95s). Test mới gameplay.action_transaction; mở rộng animator/visuals/runtime để kiểm marker boundary/loop/dt/restore/copy, mục tiêu mất hoặc đổi state, hết seed/stamina/stock, full inventory/event queue; đi tuyến farm thật để gieo/chờ/thu và nhặt item. Graphics/smoke/console regression đã chạy. Không có phiên chơi thủ công mới trong lượt này.

Ảnh/các CSV đang dùng giữ nguyên; 204 Original khớp SHA256. CSV cũ tự có midpoint/end markers cho action; artwork hành động vẫn một pose/hướng, không nhận là art hoàn thiện. Inventory vẫn bộ đếm; tám hướng/outfit, fixed timestep, production NPC/shop/door, lighting/day-night/SQLite chưa xong.

Chi tiết: [Action & Animation Events](ACTION_ANIMATION_EVENTS.md). **Task code tiếp: P1-003 fixed-step simulation**. P0 art review vẫn PARTIAL. Các báo cáo P1-001/Gameplay11 dưới đây là mốc lịch sử.

## Cập nhật P1-001 — 2026-11-05

**DONE:** cross-validator asset–animation tích hợp AssetAudit/CTest; 62 clips được đối chiếu với 204 assets và 26 khai báo profile/state trong VisualCoverage.csv version 1. Bắt unknown/not-ready reference, kind, PNG/grid/frame window, missing directions và policy/version lỗi. Không dùng Any fallback để giả đủ hướng.

Debug **21/21 PASS** (24.13s), Release **21/21 PASS** (10.15s); bộ cross-validation có 97 checks/0 failures, CLI negative test trả exit 1 đúng diagnostic. 204 Original vẫn khớp SHA256; không sửa ảnh/Assets.csv/VisualClips.csv. Chi tiết: [báo cáo validation](ASSET_ANIMATION_VALIDATION.md).

Tiến độ hiện tại: engine/runtime cơ bản và Gameplay11 đã hoạt động; kiểm tra dữ liệu P1-001 xong. MVP chưa hoàn tất: animation bốn hướng, farming/inventory/NPC/animal còn prototype; tám hướng/outfit, impact events, vòng farm ba crop/tưới/bán, shop/door, SQLite, day/night/lighting chưa xong. Art review P0-001 còn PARTIAL. **Task code tiếp theo: P1-002 animation impact**, rồi fixed timestep.

Các kết quả Gameplay11 bên dưới là lịch sử ngày 04/11; không coi DONE kỹ thuật là game đã hoàn chỉnh.

## Cập nhật Gameplay11 — 2026-11-04

Đã sửa console Release, scale/ground placement, demo autoplay, pet follow, input ownership, portrait overlay và toolbar. Debug/Release đều build thành công, **19/19 tests PASS** (14.15s / 7.34s); bản Release mở trực tiếp và log Console attached: no, smoke PowerShell/CMD exit 0. NPC/pet/debug mặc định tắt, F9/F10/F11/F12 bật riêng; player/NPC/pet controller độc lập và walk theo dịch chuyển thật.

Đo 50 frames của 14 human/animal textures; 1 frame portrait Linh chạm mép trên, artwork NEEDS_ART_REVIEW. 204 Original vẫn khớp hash. Animation vẫn bốn hướng; chưa impact events/fixed-step/SQLite/day-night. UI là hotbar, chưa inventory hoàn chỉnh. Không đo FPS.

Chi tiết nguyên nhân, file và evidence: [GAMEPLAY11](GAMEPLAY11.md), [Character Scale & Style Standard](CHARACTER_SCALE_STYLE_STANDARD.md), [task](TASKS/GAMEPLAY11.md). Manual Release đã quan sát NPC idle/patrol toggle, pet follow, overlay và F7/F8; held-WASD/full loop nhiều FPS chưa kiểm thủ công, movement/state được kiểm bằng integration test.

## Baseline Phase F trước Gameplay11 (lịch sử)

Các bảng/kết quả bên dưới mô tả thời điểm audit ban đầu, không ghi đè cập nhật Gameplay11 trên đây.

2026-11-04. Cwd/Git root D:/my_code/MyFarmEngine; branch feat/gameplay10-phase-f-depth-directional-harvest; HEAD eac0715. Git sạch trước thay đổi. Không tìm thấy AGENTS.md trong repo ngoài vendor/output, D:/ hoặc D:/my_code/. Lỗi Git work-tree ở lần đọc sandbox đầu đã hết khi chạy với quyền truy cập đúng repo; core.bare=false, repo không hỏng.

Commit gần đây: eac0715 docs Phase F; 92b3a22 presets; 33c0419 orchard/directional actors; 851dc3d retrigger one-shot; cc38534 canopy mask.

DONE chỉ có nghĩa phạm vi được mô tả và kiểm chứng, không phải toàn master specification.

| Tính năng | Status | Bằng chứng / giới hạn |
| --- | --- | --- |
| Application/Window | DONE | Đường gọi main tới Game/scene có thật; lifecycle và smoke pass |
| Time | PARTIAL | Clamp/minimize/resize có; chưa fixed timestep/interpolation |
| Engine class | TODO | Engine.h/.cpp rỗng; Application là vòng đời đang dùng |
| OpenGL/Shader/Texture/Canvas cơ bản | DONE | Context 3.3, GLSL compile/link, PNG/RGBA, GL_LINEAR/blending; pixel test pass |
| Renderer2D đầy đủ | PARTIAL | Atlas/culling/Y-sort; mỗi sprite một draw, chưa batching/profiler |
| Camera/Input | DONE | Follow/zoom/clamp, world↔screen, WASD/key events tích hợp; camera test pass |
| ECS/Components/Systems | PARTIAL | ColliderComponent và Collision/Interaction có thật; chưa ECS registry, nhiều component khác rỗng |
| Animation/sprite sheet | PARTIAL | 62 clip rows, pause/loop/one-shot/retrigger/snapshot RAM, bốn hướng; chưa tám hướng/frame events/layers |
| Asset/resource management | PARTIAL | Manifest/decoder/cache/fallback tích hợp; chưa cross-validator/atlas builder/editor/async |
| Scene/World/Tilemap | PARTIAL | Grid/chunk range/zones/objects/colliders; một ảnh nền, chưa chunk streaming/unloading |
| Player | PARTIAL | Đi/chạy/collision/đổi Linh-An; run dùng walk nhanh hơn; chưa outfit layers |
| NPC | PARTIAL | Đứng cố định tại NpcPosition, đổi clip theo timer; chưa pathfinding/schedule/dialogue/relationship |
| Animal | PARTIAL | Chó follow/collision; một hướng any; sleep chỉ frame 0 và đổi tỷ lệ; chưa feed/pet/products |
| Farming | PARTIAL | Một turnip plot gieo/chờ 5 giây/thu; hai táo stock/regrow; chưa tưới/ba crop/vòng bán |
| Inventory/Tools | PARTIAL | Bốn hotbar icon và inventoryCount chung; chưa item IDs/stacks/money/equipment; can/cake chưa có use logic |
| Collision | DONE | Foot AABB, bounds/water/static/dynamic, substep slide, line of sight; tests pass |
| Interaction/effects | PARTIAL | F kiểm tra range/facing/tool/stamina/cooldown/capacity; model đổi ngay rồi phát event, chưa animation impact |
| Chop tree | PARTIAL | Generic object giảm health/tắt collider; hình baked vẫn còn, chưa fall/wood drops |
| Audio | TODO | Không có backend/call trong target chạy |
| SQLite save/load | TODO | Save.cpp rỗng, Save.h chỉ khai báo; AnimatorSnapshot chỉ lưu RAM |
| Lighting/shadows | PARTIAL | Rect contact proxy dưới người, ánh sáng baked; chưa sunlight/local light/soft shadow |
| Day/night/weather/seasons | TODO | DayNightSystem.h và WeatherSystem.h rỗng |
| Door/shop/crafting/cooking/quests/social | TODO | Chưa tích hợp; class/header/history image không phải tính năng |
| Multiplayer | TODO | Kế hoạch tương lai ngoài MVP |

## Build/test đã chạy

Toolchain: VS 2026 Community, MSVC 19.51.36248 x64, CMake 4.3.1-msvc1, Ninja. Vendor headers: GLFW 3.5.0, GLM 1.1.0, stb_image 2.30 và GLAD. Smoke log: OpenGL 3.3.0 NVIDIA 566.07, RTX 3050 Laptop GPU, VSync on.

1. Scripts/Build.ps1 -Phase phase-f -Configuration Debug: PrepareAssets kiểm tra 204 Original, configure/incremental build thành công; 16/16 CTest PASS, 8.13 giây. Build này chỉ copy assets vì object code đã có.
2. Sau nạp Launch-VsDevShell và CMake/Ninja PATH, build sạch riêng: cmake --preset phase-f-debug -B out/build/audit-20261104-debug; cmake --build out/build/audit-20261104-debug --parallel 4; ctest --test-dir out/build/audit-20261104-debug --output-on-failure. Compile/link 86 bước thành công, 16/16 PASS, 8.63 giây, gồm 4 graphics tests.
3. Scripts/CheckAssets.ps1: 189 imported files, 18 artwork và 204 immutable Original snapshots đúng SHA256.
4. out/build/phase-f-debug/AssetAudit.exe --report out/build/phase-f-debug/audit-20261104-assets.csv: 204 records, ready=27, needs_review=177, disabled=0, errors=0. Report có dimensions/alpha từng record.
5. out/build/phase-f-debug/MyFarmEngine.exe --smoke-test: exit 0 sau 5 frame, OpenGL/GPU log thật.
6. Đối chiếu 62 rows VisualClips với ready Assets.csv và total_frames: 0 mismatch; kiểm tra audit bổ sung, chưa có cross-validator tích hợp.

CTest gồm assets.validation/catalog, engine.camera/time/animator/lifecycle/canvas, gameplay.visuals/render_order/world/collision/foreground_trees/effects/interaction, game.smoke, assets.fallback. Graphics tests cần desktop và đã thực sự chạy. Probe pthread fail trên Windows là detection bình thường, Threads cuối cùng FOUND; không phải lỗi configure.

Không có lỗi build P0 cần sửa. Release/ASan chưa chạy phiên này. Không đo FPS, CPU/GPU frame time hoặc VRAM, không tuyên bố đạt 60 FPS. Smoke/pixel regression không thay chơi thử đầy đủ/duyệt toàn bộ animation.

Executable: out/build/phase-f-debug/MyFarmEngine.exe; bản compile sạch out/build/audit-20261104-debug/MyFarmEngine.exe. Chạy không tham số để chơi, --smoke-test tự thoát, --no-vsync dành cho đo sau này.

## Hình ảnh và thay đổi

27 ready gồm 8 history/reference, còn 19 là record nền/portrait/icon/sheet, không phải 19 bộ gameplay hoàn chỉnh. 177 Library vẫn needs_review/bị chặn upload. Catalog không báo missing/corrupt/grid/alpha error; hash không đánh giá chất lượng mỹ thuật.

Đã xem trực tiếp Linh walk-left, An action-directions, dog walk, turnip growth, world_phase_e và Original character_0001/sprite.png. Ảnh cũ là pixel art 164×219 từ nguồn. Atlas mới cần QA alignment/pivot/alpha edge; không tuyên bố đã duyệt hình cả 204 ảnh. Phân biệt nguyên nhân trong WORLD_ASSET_BIBLE.

Chỉ thêm sáu tài liệu yêu cầu, Docs/TASKS và link README. Không sửa source/CMake/CSV/asset, không xóa ảnh, không tạo engine mới, không commit/push. Output build/report/fallback nằm trong out/ bị Git ignore. Tài liệu cũ giữ làm lịch sử; baseline hiện hành là báo cáo này.

Ba task tiếp: P0-001 visual/frame/canopy QA → P1-001 cross-validator → P1-002 impact events. Xem ROADMAP và TASKS.
