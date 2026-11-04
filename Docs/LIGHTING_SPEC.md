# MyFarmEngine — Lighting specification

Baseline 2026-11-04. Phần mục tiêu dưới đây chưa triển khai.

## Hiện trạng thực tế

Canvas GLSL 330 chỉ texture*tint và ellipseMask canopy. Chưa sunlight/ambient clock/local-light/shadow pass. DrawActor vẽ rect 44×9 alpha 0.19 dưới chân làm contact proxy; dog chưa có pass bóng riêng. Bóng cây/nhà và ánh sáng trên world_phase_e baked trong RGB, không thể đổi hướng độc lập theo clock.

DayNightSystem.h/WeatherSystem.h rỗng. Chưa game calendar, mùa/thời tiết, quality presets, light culling/GPU timing. Canvas tests đã pass chỉ kiểm canvas hiện tại.

## Mục tiêu theo task

Giữ OpenGL 3.3 và renderer hiện có; chưa cần shadow mapping 3D. Cần artwork tách nền/foreground/shadow proxy để không cộng bóng động lên baked shadows hai lần.

- Clock lưu được: Dawn/Morning/Noon/Afternoon/Sunset/Dusk/Night. Keyframes ambient RGB/intensity, sun direction/elevation, shadow length/softness, weather tint; interpolate qua ranh giới ngày, tách game time/wall clock.
- Contact shadow mềm bám chân, vẽ trước sprite, opacity theo tiếp xúc/độ cao, quality toggle. Rect hiện tại chỉ prototype.
- Directional silhouette/proxy cho cây/nhà, biến dạng theo mặt trời, giới hạn chiều dài ở elevation thấp. Canopy ellipse không phải shadow.
- Local light ID/position/color/radius/intensity/falloff/enabled/flicker nhẹ; camera cull và light budget. Đèn/cửa sổ/bếp có state thật.
- Weather/season sau này thay ambient/gió/particle/nước/wet ground/audio và lưu được.
- Low: contact đơn giản/ít light/particle. Medium: directional proxy/local lights cơ bản. High: soft shadow/nước/ambient nâng cao; toggle và giới hạn rõ.

## Acceptance và profiling

Unit tests clock wrap 24h/interpolation/pause/serialization boundary; visual dawn/noon/night, foot contact, sun direction, đèn bật/tắt; GL/canvas regression. Đo Release với scene/camera path/hardware cố định: CPU frame time, GPU query, draw calls, texture bytes ước tính và VRAM khi công cụ hỗ trợ. Ghi VSync/resolution/preset/percentile. Chưa benchmark; 60 FPS chỉ là mục tiêu.
