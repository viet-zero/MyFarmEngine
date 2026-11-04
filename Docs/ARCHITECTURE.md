# MyFarmEngine — Kiến trúc thực tế

## Cập nhật Gameplay11

[GAMEPLAY11](GAMEPLAY11.md) bổ sung SceneOptions truyền từ CLI/GameConfig, input focus ownership, SpriteGeometry cached alpha bounds dùng chung render/cull, responsive hotbar và mipmap minification. Release Windows dùng GUI/WinMain CRT; Debug giữ console/main; stdout/stderr được tee ra log file. Không có engine/renderer song song. Mô tả Phase F bên dưới giữ làm baseline lịch sử; chi tiết placement hiện hành ở [Character Standard](CHARACTER_SCALE_STYLE_STANDARD.md).

Audit 2026-11-04, HEAD eac0715, branch feat/gameplay10-phase-f-depth-directional-harvest. Kết luận thiếu source ở Documents/ChatGPT trước đây không áp dụng cho repo D:/my_code/MyFarmEngine.

## Cấu trúc và integration

- Engine/: Core, Animation, Graphics, Input, Resources và Event có mã được build. ECS, Audio, Scene, Resource là thư mục hiện có nhưng chưa có implementation tích hợp. Core/Engine.h, Engine.cpp, EntryPoint.h rỗng.
- Game/: main, GameApp/Game và GameConfig đang chạy. GameInitializer.cpp chỉ là khung, không nằm trong target.
- Gameplay/: States, World, Graphics, Assets, Systems/CollisionSystem, InteractionSystem và Effects tích hợp; nhiều Components/Systems và JSON khác rỗng.
- Assets/: Original, Workspace, Export, Art, Backgrounds, Library, Shaders và manifests. ThirdParty/: GLFW, GLAD, GLM, stb có sẵn.
- Tests/, Scripts/, HuongDan/, Docs/ và output trong out/ (Git ignore).

Luồng: main → Application.Initialize → Window/OpenGL + Input + Renderer → Game → GameplayState → Canvas2D + AssetTextures + DemoScene2D. Mỗi frame PollEvents → Time.Update → Game.Update(deltaTime) → Render → SwapBuffers. Shutdown texture/shader/buffer trước khi hủy context.

Time dùng deltaTime biến thiên có clamp, chưa fixed-step/interpolation. Không có ECS registry/entity lifecycle: player, dog, controllers và targets nằm trong DemoScene2D/WorldMap/vector. Không có worker jobs, OpenGL chạy trên context thread.

## CMake và render

C++20, extensions OFF, warnings-as-errors ON. CMakeLists yêu cầu 3.20; preset v3 yêu cầu 3.21. Ninja/MSVC được nạp qua Scripts/Build.ps1. Targets: MyFarmAssetData, MyFarmEngineCore, MyFarmGameplayAnimation, MyFarmRenderOrder, MyFarmWorld, MyFarmGameplayLogic, MyFarmVisualAssets; executables MyFarmEngine, AssetAudit và test targets. MyFarmRuntimeAssets copy Export/Shaders/CSV cạnh EXE. Không có SQLite hoặc audio dependency hiện tại.

Window yêu cầu OpenGL 3.3 Core. Renderer chỉ clear/viewport; Canvas2D vẽ thật bằng quad/shader, mỗi sprite một glDrawArrays. GL_LINEAR, straight-alpha blending, atlas UV và world/screen projection có; chưa sprite batching, mipmap hoặc GPU profiling.

Render: visible background chunks → RenderOrder cull và footY/sequence sort → actor/dog/crop/pickup/canopy → effects → UI. WorldMap: grid 96×64, tile 64, chunk 16×16, world 6144×4096. Background world_phase_e là một ảnh 1536×1024 lấy mẫu theo chunk, chưa streaming/unloading hoặc tile artwork độc lập. Hai canopy lấy lại texture nền bằng ellipse mask, chưa phải foreground silhouette chính xác.

AssetPath ưu tiên Assets cạnh EXE. AssetManifest kiểm tra schema/ID/path containment/grid/anchor/decode/alpha. AssetTextures cache và fallback null cho asset thiếu/chưa duyệt. VisualCatalog chọn profile/state/direction; Animator có timeline theo giây, chưa frame events.

## Thiết kế tiếp theo, chưa triển khai

Mở rộng hệ thống hiện có: cross-validator → animation impact transaction → fixed simulation → player tám hướng/outfit layers → typed inventory/ba crop → clock/lighting → NPC/animal/door/shop → SQLite version hóa. Stable IDs làm nền serialization; snapshot DB I/O ngoài render loop. Không tạo renderer/resource/save manager song song. Multiplayer ngoài MVP.

Chi tiết: [progress](PROGRESS.md), [roadmap](ROADMAP.md), [asset bible](WORLD_ASSET_BIBLE.md), [animation](ANIMATION_SPEC.md), [lighting](LIGHTING_SPEC.md).
