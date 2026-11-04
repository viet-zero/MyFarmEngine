# Gameplay11 — Runtime, scale và demo behavior

Status: DONE cho sửa kỹ thuật A–F đã kiểm thử; PARTIAL cho art review/polish.

- Goal: Release không console phụ, actor consistency, opt-in demo/NPC/follow, stable state, debug overlay và hotbar rõ ràng.
- Relevant files: xem Docs/GAMEPLAY11.md và CHARACTER_SCALE_STYLE_STANDARD.md.
- Dependencies: Phase F buildable baseline eac0715, audit documentation 59645e5, existing CMake/asset/animation/collision systems.
- Acceptance criteria: default no autoplay/overlay; controls độc lập, walk theo displacement; uniform aspect/ground anchor và Y-sort; Release GUI/Debug console, errors persistent; source images bất biến. Phần kỹ thuật đạt theo evidence; artwork không tự nhận DONE.
- Tests: Debug 19/19, Release 19/19; native/PowerShell/CMD launch và logs; scene integration/pixel/geometry tests; Release UI observations; CheckAssets SHA256 pass.
- Out of scope: tám hướng, thay art Original, NPC production AI, inventory đầy đủ, impact events/fixed timestep/lighting/save/multiplayer.

Hạn chế kiểm thử: chưa manual held-WASD nhiều frame rates hoặc toàn gameplay loop; automated integration xác minh chuyển động/state. Art NEEDS_ART_REVIEW và acceptance còn lại của P0-001 giữ mở. Không push.
