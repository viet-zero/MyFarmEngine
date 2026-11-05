cmake_minimum_required(VERSION 3.20)
project(MyFarmEngine VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(MYFARM_WARNINGS_AS_ERRORS "Treat project warnings as errors" ON)
option(MYFARM_ENABLE_ASAN "Instrument project targets with AddressSanitizer" OFF)

# Build the engine, not the bundled GLFW demos/tools.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW docs")
set(GLFW_INSTALL OFF CACHE BOOL "Install GLFW")
add_subdirectory(ThirdParty/glfw)
add_library(glad STATIC ThirdParty/glad/src/glad.c)
target_include_directories(glad PUBLIC ThirdParty/glad/include)

add_library(MyFarmEngineCore STATIC
    Engine/Animation/Animator.cpp
    Engine/Core/Application.cpp
    Engine/Core/Window.cpp
    Engine/Core/Time.cpp
    Engine/Graphics/Renderer.cpp
    Engine/Graphics/Texture2D.cpp
    Engine/Graphics/Canvas2D.cpp
    Engine/Graphics/Shader.cpp
    Engine/Resources/AssetPath.cpp
    Engine/Input/Input.cpp
)
add_library(MyFarmAssetData STATIC
    Engine/Resources/ImageData.cpp
    Gameplay/Assets/AssetManifest.cpp
)
target_include_directories(MyFarmAssetData PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Engine)
target_include_directories(MyFarmAssetData SYSTEM PRIVATE ThirdParty/stb)
target_link_libraries(MyFarmEngineCore PUBLIC MyFarmAssetData)
add_library(MyFarmGameplayAnimation STATIC Gameplay/Graphics/VisualCatalog.cpp)
target_link_libraries(MyFarmGameplayAnimation PUBLIC MyFarmEngineCore)
add_library(MyFarmRenderOrder STATIC Gameplay/Graphics/RenderOrder.cpp)
target_link_libraries(MyFarmRenderOrder PUBLIC MyFarmEngineCore)
add_library(MyFarmWorld STATIC Gameplay/World/WorldMap.cpp Gameplay/World/ForegroundTrees.cpp Gameplay/Systems/CollisionSystem.cpp)
target_link_libraries(MyFarmWorld PUBLIC MyFarmEngineCore)
add_library(MyFarmGameplayLogic STATIC Gameplay/Save/Save.cpp Gameplay/Crops/Crop.cpp Gameplay/Inventory/Inventory.cpp Engine/Event/EventBus.cpp Gameplay/Effects/EffectPool.cpp Gameplay/Systems/InteractionSystem.cpp)
target_link_libraries(MyFarmGameplayLogic PUBLIC MyFarmEngineCore)
if(WIN32)
    target_link_libraries(MyFarmGameplayLogic PRIVATE odbc32)
endif()
add_library(MyFarmVisualAssets STATIC Gameplay/Graphics/AssetTextures.cpp Gameplay/Graphics/SpriteGeometry.cpp)
target_link_libraries(MyFarmVisualAssets PUBLIC MyFarmGameplayAnimation)

add_executable(AssetAudit Gameplay/Assets/AssetAudit.cpp)
target_link_libraries(AssetAudit PRIVATE MyFarmGameplayAnimation)
target_compile_definitions(AssetAudit PRIVATE
    MYFARM_SOURCE_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/Assets"
    MYFARM_SOURCE_MANIFEST="${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/Assets.csv"
    MYFARM_SOURCE_VISUALS="${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/VisualClips.csv"
    MYFARM_SOURCE_COVERAGE="${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/VisualCoverage.csv")
target_include_directories(MyFarmEngineCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Engine)
target_include_directories(MyFarmEngineCore SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm)
target_include_directories(MyFarmEngineCore SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb)
target_compile_definitions(MyFarmEngineCore PRIVATE MYFARM_SOURCE_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/Assets")
target_compile_definitions(MyFarmEngineCore PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(MyFarmEngineCore PUBLIC glfw glad)

add_executable(MyFarmEngine
    Game/main.cpp
    Game/RuntimeLog.cpp
    Game/GameApp/Game.cpp
    Gameplay/States/GameplayState.cpp
    Gameplay/World/DemoScene2D.cpp
)
# Only Release is a GUI application. Debug keeps its normal developer console.
if(WIN32)
    set_property(TARGET MyFarmEngine PROPERTY WIN32_EXECUTABLE "$<CONFIG:Release>")
    target_compile_definitions(MyFarmEngine PRIVATE $<$<CONFIG:Release>:MYFARM_WINDOWED_APP>)
endif()

target_include_directories(MyFarmEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    ThirdParty/glm ThirdParty/stb)
target_link_libraries(MyFarmEngine PRIVATE MyFarmVisualAssets MyFarmWorld MyFarmGameplayLogic MyFarmRenderOrder)

# Runtime content travels beside the EXE. No source archive or old code is deployed.
add_custom_target(MyFarmRuntimeAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Export" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Export"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Shaders"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Data"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/Assets.csv" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Data/Assets.csv"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/VisualClips.csv" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Data/VisualClips.csv"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/Objects.csv" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Data/Objects.csv"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/ForegroundTrees.csv" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Data/ForegroundTrees.csv"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/StaticColliders.csv" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Data/StaticColliders.csv"
)
add_dependencies(MyFarmEngine MyFarmRuntimeAssets)

function(enable_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
        if(MYFARM_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
        if(MYFARM_ENABLE_ASAN)
            target_compile_options(${target} PRIVATE /fsanitize=address)
            target_link_options(${target} PRIVATE /INCREMENTAL:NO)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(MYFARM_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
        if(MYFARM_ENABLE_ASAN)
            target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif()
    endif()
endfunction()
enable_project_warnings(MyFarmEngineCore)
enable_project_warnings(MyFarmAssetData)
enable_project_warnings(MyFarmVisualAssets)
enable_project_warnings(MyFarmGameplayAnimation)
enable_project_warnings(MyFarmRenderOrder)
enable_project_warnings(MyFarmWorld)
enable_project_warnings(MyFarmGameplayLogic)
enable_project_warnings(AssetAudit)
enable_project_warnings(MyFarmEngine)

include(CTest)
if(BUILD_TESTING)
    add_executable(AssetPipelineTests Tests/AssetTests.cpp)
    target_link_libraries(AssetPipelineTests PRIVATE MyFarmAssetData)
    enable_project_warnings(AssetPipelineTests)
    add_test(NAME assets.validation COMMAND AssetPipelineTests)
    add_test(NAME assets.catalog COMMAND AssetAudit)
    add_executable(EngineCameraTests Tests/CameraTests.cpp)
    target_include_directories(EngineCameraTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(EngineCameraTests PRIVATE MyFarmEngineCore)
    enable_project_warnings(EngineCameraTests)
    add_test(NAME engine.camera COMMAND EngineCameraTests)
    add_executable(EngineTimeTests Tests/TimeTests.cpp)
    target_link_libraries(EngineTimeTests PRIVATE MyFarmEngineCore)
    enable_project_warnings(EngineTimeTests)
    add_test(NAME engine.time COMMAND EngineTimeTests)
    add_executable(FixedStepTests Tests/FixedStepTests.cpp)
    target_link_libraries(FixedStepTests PRIVATE MyFarmEngineCore)
    enable_project_warnings(FixedStepTests)
    add_test(NAME engine.fixed_step COMMAND FixedStepTests)
    add_executable(EngineAnimatorTests Tests/AnimatorTests.cpp)
    target_link_libraries(EngineAnimatorTests PRIVATE MyFarmEngineCore)
    enable_project_warnings(EngineAnimatorTests)
    add_test(NAME engine.animator COMMAND EngineAnimatorTests)
    add_executable(GameplayVisualTests Tests/VisualCatalogTests.cpp)
    target_link_libraries(GameplayVisualTests PRIVATE MyFarmGameplayAnimation)
    enable_project_warnings(GameplayVisualTests)
    add_test(NAME gameplay.visuals COMMAND GameplayVisualTests ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/VisualClips.csv)
    add_executable(VisualAssetValidationTests Tests/VisualAssetValidationTests.cpp)
    target_link_libraries(VisualAssetValidationTests PRIVATE MyFarmGameplayAnimation)
    enable_project_warnings(VisualAssetValidationTests)
    add_test(NAME assets.visual_cross_validation COMMAND VisualAssetValidationTests
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/Assets.csv
        ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/VisualClips.csv ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/VisualCoverage.csv)
    add_test(NAME assets.visual_audit_cli COMMAND ${CMAKE_COMMAND}
        -DAUDIT=$<TARGET_FILE:AssetAudit> -DBUILD=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/Tests/VisualAuditCli.cmake)
    add_executable(GameplayRenderOrderTests Tests/RenderOrderTests.cpp)
    target_link_libraries(GameplayRenderOrderTests PRIVATE MyFarmRenderOrder)
    enable_project_warnings(GameplayRenderOrderTests)
    add_test(NAME gameplay.render_order COMMAND GameplayRenderOrderTests)
    add_executable(GameplayWorldTests Tests/WorldMapTests.cpp)
    target_link_libraries(GameplayWorldTests PRIVATE MyFarmWorld)
    enable_project_warnings(GameplayWorldTests)
    add_test(NAME gameplay.world COMMAND GameplayWorldTests ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/Objects.csv ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/StaticColliders.csv)
    add_executable(GameplayCollisionTests Tests/CollisionTests.cpp)
    target_link_libraries(GameplayCollisionTests PRIVATE MyFarmWorld)
    enable_project_warnings(GameplayCollisionTests)
    add_test(NAME gameplay.collision COMMAND GameplayCollisionTests ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/Objects.csv ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/StaticColliders.csv)
    add_executable(GameplayForegroundTreeTests Tests/ForegroundTreeTests.cpp)
    target_link_libraries(GameplayForegroundTreeTests PRIVATE MyFarmWorld)
    enable_project_warnings(GameplayForegroundTreeTests)
    add_test(NAME gameplay.foreground_trees COMMAND GameplayForegroundTreeTests ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/Objects.csv ${CMAKE_CURRENT_SOURCE_DIR}/Gameplay/Data/ForegroundTrees.csv)
    add_executable(GameplayEffectTests Tests/EffectTests.cpp)
    target_link_libraries(GameplayEffectTests PRIVATE MyFarmGameplayLogic)
    enable_project_warnings(GameplayEffectTests)
    add_test(NAME gameplay.effects COMMAND GameplayEffectTests)
    add_executable(FarmingTests Tests/FarmingTests.cpp)
    target_link_libraries(FarmingTests PRIVATE MyFarmGameplayLogic)
    enable_project_warnings(FarmingTests)
    add_test(NAME gameplay.farming COMMAND FarmingTests)

    add_executable(SaveTests Tests/SaveTests.cpp)
    target_link_libraries(SaveTests PRIVATE MyFarmGameplayLogic)
    enable_project_warnings(SaveTests)
    add_test(NAME gameplay.save_codec COMMAND SaveTests)

    add_executable(InventoryTests Tests/InventoryTests.cpp)
    target_link_libraries(InventoryTests PRIVATE MyFarmGameplayLogic)
    enable_project_warnings(InventoryTests)
    add_test(NAME gameplay.inventory COMMAND InventoryTests)

    add_executable(GameplayInteractionTests Tests/InteractionTests.cpp)
    target_link_libraries(GameplayInteractionTests PRIVATE MyFarmGameplayLogic)
    enable_project_warnings(GameplayInteractionTests)
    add_test(NAME gameplay.interaction COMMAND GameplayInteractionTests)

    add_executable(SpriteGeometryTests Tests/SpriteGeometryTests.cpp)
    target_link_libraries(SpriteGeometryTests PRIVATE MyFarmVisualAssets)
    enable_project_warnings(SpriteGeometryTests)
    add_test(NAME gameplay.sprite_geometry COMMAND SpriteGeometryTests)
    add_executable(GameplayRuntimeTests Tests/GameplayRuntimeTests.cpp
        Gameplay/World/DemoScene2D.cpp Gameplay/States/GameplayState.cpp)
    target_link_libraries(GameplayRuntimeTests PRIVATE MyFarmVisualAssets MyFarmWorld MyFarmGameplayLogic MyFarmRenderOrder)
    enable_project_warnings(GameplayRuntimeTests)
    add_dependencies(GameplayRuntimeTests MyFarmRuntimeAssets)
    add_test(NAME gameplay.runtime COMMAND GameplayRuntimeTests)
    set_tests_properties(gameplay.runtime PROPERTIES TIMEOUT 60 LABELS "graphics" RUN_SERIAL TRUE)

    if(WIN32)
        find_program(MYFARM_POWERSHELL NAMES pwsh powershell REQUIRED)
        add_test(NAME game.windows_launch COMMAND ${MYFARM_POWERSHELL} -NoProfile
            -File ${CMAKE_CURRENT_SOURCE_DIR}/Tests/WindowsLaunchTests.ps1
            -Game $<TARGET_FILE:MyFarmEngine> -ExpectedSubsystem $<IF:$<CONFIG:Release>,2,3>)
        set_tests_properties(game.windows_launch PROPERTIES TIMEOUT 75 LABELS "graphics" RUN_SERIAL TRUE)
    endif()
    add_executable(ActionTransactionTests Tests/ActionTransactionTests.cpp)
    target_link_libraries(ActionTransactionTests PRIVATE MyFarmGameplayLogic)
    enable_project_warnings(ActionTransactionTests)
    add_test(NAME gameplay.action_transaction COMMAND ActionTransactionTests)
    add_executable(GameplayFixedRateTests Tests/GameplayFixedRateTests.cpp Gameplay/World/DemoScene2D.cpp)
    target_link_libraries(GameplayFixedRateTests PRIVATE MyFarmVisualAssets MyFarmWorld MyFarmGameplayLogic MyFarmRenderOrder)
    enable_project_warnings(GameplayFixedRateTests)
    add_dependencies(GameplayFixedRateTests MyFarmRuntimeAssets)
    add_test(NAME gameplay.fixed_rate COMMAND GameplayFixedRateTests)
    set_tests_properties(gameplay.fixed_rate PROPERTIES TIMEOUT 60 LABELS "graphics" RUN_SERIAL TRUE)
    add_executable(EngineLifecycleTests Tests/LifecycleTests.cpp)
    target_link_libraries(EngineLifecycleTests PRIVATE MyFarmEngineCore)
    enable_project_warnings(EngineLifecycleTests)
    add_test(NAME engine.lifecycle COMMAND EngineLifecycleTests)
    add_test(NAME game.smoke COMMAND MyFarmEngine --smoke-test)
    add_executable(EngineCanvasTests Tests/CanvasTests.cpp)
    target_include_directories(EngineCanvasTests SYSTEM PRIVATE ThirdParty/stb)
    target_link_libraries(EngineCanvasTests PRIVATE MyFarmVisualAssets)
    enable_project_warnings(EngineCanvasTests)
    add_test(NAME engine.canvas COMMAND EngineCanvasTests)
    add_dependencies(EngineCanvasTests MyFarmRuntimeAssets)
    add_test(NAME assets.fallback COMMAND ${CMAKE_COMMAND}
        -DGAME=$<TARGET_FILE:MyFarmEngine>
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}
        -DBUILD=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/Tests/AssetFallback.cmake)
    set_tests_properties(engine.lifecycle game.smoke engine.canvas assets.fallback PROPERTIES
        TIMEOUT 30 LABELS "graphics" RUN_SERIAL TRUE)
endif()
