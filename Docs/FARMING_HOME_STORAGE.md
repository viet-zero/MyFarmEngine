# P2-002B — Trồng lại, tưới, tăng trưởng và kho vườn nhà

2026-11-05. Logic-first, không thay ảnh Original/Export hay manifest artwork. P2-002 tổng thể vẫn PARTIAL; đây là phần farming/kho prototype đã tích hợp, không phải toàn bộ farm/economy/save.

## Cách chơi hiện tại

- Đứng gần và nhìn về plot `crop.plot.01` tại (3000,1700). Plot hiện có đất đã chuẩn bị sẵn, chưa có thao tác cuốc đất.
- Chọn **3**, nhấn **F** để gieo hạt củ cải. Start chưa trừ hạt; IMPACT mới trừ một hạt và tạo cây khô.
- Chọn **1**, nhấn **F** để tưới bằng bình đang sở hữu. Chưa có dung tích bình hoặc lấy nước; tưới hiện dùng stamina/cooldown như các thao tác khác.
- Cây chỉ lớn trong phần thời gian còn ẩm. Khô thì dừng, không chết/héo. Cây chín không mất sản phẩm khi hết ẩm.
- Cây chín: nhấn **F** để thu. Plot không bị deactivate; age/moisture/crop reset, đất vẫn sẵn sàng cho lượt gieo tiếp. Chọn 3/F để trồng lại.
- Táo và nông sản thu trong vườn sở hữu tự vào kho. Vật phẩm rơi nhặt dưới đất vẫn vào ba lô.
- **K** mở/đóng bảng số lượng kho khi đứng trong vườn; đi ra ngoài thì đóng. Panel chỉ đọc số lượng, chưa có rút/chuyển/sắp xếp đồ, không tự pause gameplay.
- Hotbar vẫn **4 ô**, không đưa 12 ô ba lô/48 ô kho ra ngoài thường trực. Di chuyển, đổi ô, F/C/K sẽ hiện lại; sau 4 giây không thao tác sẽ ẩn, mờ dần trong 0.6 giây cuối. Pause dừng timer UI cùng simulation.

UI có chữ ASCII tối giản `KHO`, `CU CAI`, `CA ROT`, `KHOAI TAY`, `TAO`, `SO O`; không thêm font/ảnh mới. Thông báo ngắn `VAO KHO`, `KHO DAY`, `DA TUOI` dùng cùng style prototype. Chưa phải UI mỹ thuật hoàn chỉnh hoặc tiếng Việt đủ dấu.

## Model cây và luật prototype

Crop.h/.cpp trước đây là stub chưa tích hợp; hiện chứa enum/definitions thực sự và được link trong MyFarmGameplayLogic. InteractionTarget giữ trạng thái cây duy nhất: tilled/planted/ready, CropId, double age/moisture và revision. CropState chỉ suy ra Untilled/Empty/Planted/Growing/Ready, không có state machine thứ hai.

| CropId | Cây | Hạt → sản phẩm (ItemId) | Wet growth để chín | Ẩm mỗi lần tưới |
| --- | --- | --- | --- | --- |
| 1 | Turnip/củ cải | 1001 → 1002 | 5 giây | 6 giây |
| 2 | Carrot/cà rốt | 1006 → 1007 | 8 giây | 6 giây |
| 3 | Potato/khoai tây | 1008 → 1009 | 12 giây | 6 giây |

Đây là thời gian dev để kiểm logic, chưa cân bằng theo ngày/mùa. Không đọc Crops.json/Items.json (còn stub); definitions hiện compiled. Thời gian chỉ tiến qua fixed simulation 60 Hz; Tick phòng vệ clamp tối đa 0.1s/call. Trong một tick chỉ cộng min(dt, moisture), không làm cây lớn cả tick nếu chỉ còn ít nước. Tưới đưa moisture về mức đầy, không cộng dồn vượt mức. Tưới ngay khi đã đầy trả AlreadyWatered, không trừ tài nguyên. Crop chín ưu tiên harvest ngay cả khi đang chọn bình tưới.

Ba definitions đã kiểm đủ chu kỳ bằng tests. **Scene hiện chỉ có plot củ cải và starter seed củ cải**; chưa UI chọn/mua hạt cà rốt/khoai, chưa artwork hai cây đó. Không nhận có ba crop playable hoàn chỉnh từ registry/test. Model API chấp nhận binding hạt khác đúng CropId/produce, không suy loại cây từ atlas frame.

## Kho vườn sở hữu

HomeStorage là container riêng dùng cùng Inventory implementation; không có bản sao bộ đếm đồ hoặc inventory manager thứ hai. Ba lô Starter có 12 ô, kho có 48 ô; maxStack của nông sản vẫn 20. Inventory hỗ trợ tối đa 48 ô, default vẫn 12. Panel đếm ô thực dùng và số lượng từ chính kho.

Prototype scene dùng `HomeStorage::PlayerGarden()`:

- ownerId=1 (player local), enabled=true;
- origin=(1400,640), size=(2440,1664), tức x∈[1400,3840), y∈[640,2304);
- region này bao phủ plot và hai cây táo đang chơi; không tự đồng nhất toàn Zone::Farm là đất sở hữu. Đây là vùng tọa độ prototype, chưa là editor thửa đất/hệ nhà cửa hoặc hàng rào nhìn thấy được.

Auto-store yêu cầu cả chân người chơi và điểm thu hoạch trong region, owner khác 0 và khớp ownerId. Ngoài vườn, storage tắt hoặc owner không khớp thì harvest vào ba lô theo capacity bag. Đây là rule routing single-player, chưa phải quyền thu hoạch/chống trộm hoặc authority multiplayer.

Trong vườn: bag đầy vẫn thu được nếu kho còn chỗ. Kho đầy trả StorageFull, **giữ cây/stock và tài nguyên**, không tự chuyển sang bag hoặc đánh rơi để tránh mất đồ. Không tạo ItemPicked trung gian cho cùng sản phẩm. Ground pickup không bị hút vào kho dù đang đứng trong nhà/vườn. Cây ngoài phạm vi không gửi vào kho từ xa.

Kho hiện nằm trong RAM và reset theo Reset world; **chưa lưu qua tắt game**. Không claim SQL save/load từ việc SQL Express đã kết nối chỉ đọc ở lượt trước. SQL persistence phải lưu đúng cả bag, kho, owner/region, CropId, revision, age/moisture và trạng thái plot trong cùng snapshot/version.

## Action/atomicity

Start ghi operation Plant/Water/Harvest cùng CropId/revision, binding và destination/owner. IMPACT kiểm lại range/facing/LOS, can/seed, target lifecycle và capacity của đúng container. Đổi owner/qua biên làm route đổi trước impact thì trả TargetChanged; không đổi đích lặng lẽ. Natural growth không tăng revision, còn plant/water/harvest tăng revision; token của chu kỳ cũ không thể thu cây mới trên cùng plot ID.

Bag và kho được chuẩn bị bằng value copy sau khi kiểm event queue. Chỉ thành công mới thay containers/stamina/target và phát FX. Kho đầy giữa Start–Impact, cancel, mất bình, đổi crop/revision hoặc queue đầy không deposit/thu/tưới một phần. Exact-once actionId cũ vẫn chặn copied action, timeline rewind và marker lặp. CropWatered là event mới; dùng Action animation/IMPACT hiện có, chưa có artwork tưới riêng.

## Verification

New gameplay.farming covers:

- Ba seed/crop IDs: gieo, khô, tưới, hết ẩm, tưới lại, maturity, harvest và replant.
- Soil/wrong seed/tool, thiếu bình trước impact, hủy tưới, AlreadyWatered, partial wet tick, invalid dt.
- Home storage: bag đầy vẫn deposit, kho đầy không fallback, capacity đổi trước impact, owner/enable/actor/target boundaries, qua biên trước impact, ground pickup vào bag, cancel/replay/revision/event-queue failure.
- Trace 14 giây ở render cadence giả lập 30/60/144 Hz: 840 ticks, carrot được tưới/thu vào kho và trồng lại khô, kết quả tương đương.
- Hotbar fade/hide/reveal/invalid dt và cadence.

Runtime graphics test đi tuyến scene thật, trừ seed/thu đúng IMPACT, dry/pause không lớn, water→grow→home harvest→replant cùng plot. OpenGL pixel checks xác nhận hotbar ẩn/hiện và panel K được vẽ; K buffered chỉ toggle một lần qua catch-up, rời vườn đóng và ngoài vườn không mở được. Orchard fixed-rate tests kiểm bag không tăng và storage tăng đúng, renderer không đổi container.

Full Debug/Release và Original integrity kết quả cuối được ghi trong PROGRESS.md. Không có manual extended play, benchmark FPS hoặc art approval mới; không suy rằng pixel visibility test đã duyệt toàn bộ readability/anatomy.
