# Báo cáo Phase D

Branch `feat/gameplay10-phase-d-interaction`, nền Phase C `1d195f9`. Source có EventBus, EffectPool và InteractionSystem CPU riêng. Scene dùng `F` cho object/crop/item, kiểm tra điều kiện trước khi đổi HP, inventory, stamina và world collider. VFX phát qua event; khi thiếu tài nguyên, effect bị bỏ theo giới hạn thay vì làm gián đoạn gameplay.

Đọc [thư mục hướng dẫn](../../HuongDan/README.md) để xem công dụng từng hàm, biến, cách chỉnh thông số và cách tự làm lại cho game khác. Hướng dẫn WorldMap đã chuyển vào đó; đường dẫn cũ vẫn có trang chuyển hướng.

Kiểm thử: Debug và Release `/W4 /WX` đạt 13/13; ASan CPU đạt 7/7. Test mới bao phủ event/effect capacity, dt NaN/âm, lifetime, range/facing, cooldown, stamina, UI busy, tool, crop growth, túi đầy, target destroyed và world collider sau khi hủy. UBSan, save game, UI chữ, effect sprite/font và đường OpenGL dưới ASan chưa được kiểm chứng. Không có benchmark FPS/draw calls.
