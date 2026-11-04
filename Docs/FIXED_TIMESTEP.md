# P1-003 — Fixed timestep contract

2026-11-05. DONE kỹ thuật; giữ cấu trúc Application/Game/Gameplay hiện có.

## Vòng lặp

Application lấy raw double frame delta từ Time (legacy GetDeltaTime vẫn clamp 100ms), gọi ProcessInput một lần/frame, FixedStepClock.Advance, 0..6 Update với float 1/60, rồi Render(alpha). Input/UI không phụ thuộc có tick hay không. Run(maxFrames) đếm frame render, không đếm simulation ticks. Refresh/WM_PAINT chỉ repaint, không chạy simulation.

Clock nhận tối đa 250ms/frame và thực hiện tối đa 6 ticks (100ms game time). Toàn bộ nợ whole-tick dư bị bỏ, chỉ giữ phần lẻ; có TotalSteps/DroppedSeconds cho chẩn đoán. Stall dài cố ý làm game time chậm hơn wall clock, không tăng tốc bù về sau. NaN/infinite/nonpositive delta không phá accumulator. Đây không phải deterministic networking hoặc benchmark FPS.

## Input và pause

SceneInputBuffer giữ edge qua các frame chưa có tick, tick đầu consume, các tick tiếp chỉ dùng held movement/running. Các tap cùng lệnh trước một tick coalesce; teleport/item mới nhất thắng, zoom cộng có bound. Cancel/reset giữ precedence của scene. GLFW không bị poll lại trong các substep.

Preview, mất focus và minimize dừng simulation, bỏ accumulator và command cũ; resume frame đầu không catch-up thời gian pause. Input mới hợp lệ trên frame resume được giữ tới tick kế tiếp. Resize/refresh snap camera/previous snapshot, không cộng dt kéo cửa sổ. Reset xóa buffer; teleport snap toàn snapshot để tránh nội suy xuyên map. Action đang pause được giữ, không bị cancel chỉ vì nội suy/pause.

## Presentation

DemoScene2D lưu previous/current feet của player/NPC/pet và camera. Render nội suy các vị trí bằng alpha; draw, culling và Y-sort dùng cùng feet. Render camera là bản copy; viewport update được thông báo riêng để simulation camera vẫn đúng aspect. Nội suy trễ tối đa một tick so với model, không dùng presentation làm collision hoặc target validation. Animation frame, crops và particles vẫn discrete; chỉ actor/camera position được nội suy trong task này. Renderer không advance animation hoặc impact.

## Verification

Scripts/Build.ps1 -Phase phase-f -Configuration Debug: 24/24 PASS, 22.51s.
Release: 24/24 PASS, 11.19s. Graphics chạy OpenGL thật; game.smoke và game.windows_launch PASS.

- engine.fixed_step: 10 giây ở 30/60/144 Hz đều 600 ticks; phần lẻ, stall 5s chỉ 6 ticks/bỏ 4.9s, invalid delta, reset và input edges.
- gameplay.fixed_rate: trace input 6 giây, render cadence 30/60/144 Hz đều 360 ticks; actor/NPC/pet/camera/animation/inventory/action tương đương, hai apple impacts. Render/presentation không đổi model hoặc phát impact; kiểm interpolation/pause/reset/teleport/viewport.
- engine.lifecycle: dispatcher thật, zero-tick tap/catch-up, suspend/resume, minimize/restore native Windows và WM_PAINT/resize/reentrancy.
- Regression: action transactions, runtime toggles, collision/render ordering, launch/console, missing asset fallbacks.
- CheckAssets: 189 imported, 18 art files, 204 immutable Original khớp SHA256. Không sửa ảnh hoặc CSV.

Không có manual soak session mới hoặc chứng nhận FPS 30/60/144 thực tế. Artwork bốn hướng và action một pose không được cải thiện bằng fixed timestep. Tám hướng/outfit, inventory đầy đủ, SQLite và lighting vẫn thuộc roadmap sau.
