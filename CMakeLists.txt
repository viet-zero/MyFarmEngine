cmake_minimum_required(VERSION 3.20)
project(MyFarmEngine VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build the engine, not the bundled GLFW demos/tools.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW docs")
set(GLFW_INSTALL OFF CACHE BOOL "Install GLFW")
add_subdirectory(ThirdParty/glfw)
add_library(glad STATIC ThirdParty/glad/src/glad.c)
target_include_directories(glad PUBLIC ThirdParty/glad/include)

add_library(MyFarmEngineCore STATIC
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
target_include_directories(MyFarmEngineCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Engine)
target_include_directories(MyFarmEngineCore SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm)
target_include_directories(MyFarmEngineCore SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb)
target_compile_definitions(MyFarmEngineCore PRIVATE MYFARM_SOURCE_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/Assets")
target_compile_definitions(MyFarmEngineCore PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(MyFarmEngineCore PUBLIC glfw glad)

add_executable(MyFarmEngine
    Game/main.cpp
    Game/GameApp/Game.cpp
    Gameplay/States/GameplayState.cpp
)
target_include_directories(MyFarmEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    ThirdParty/glm ThirdParty/stb)
target_link_libraries(MyFarmEngine PRIVATE MyFarmEngineCore)

# Runtime content travels beside the EXE. No source archive or old code is deployed.
add_custom_target(MyFarmRuntimeAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Art" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Art"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Backgrounds" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Backgrounds"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Library/SNHR_v11/Characters" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Library/SNHR_v11/Characters"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Library/SNHR_v11/Animals" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Library/SNHR_v11/Animals"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Library/SNHR_v8" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/Library/SNHR_v8"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Docs/History/Previews" "$<TARGET_FILE_DIR:MyFarmEngine>/Assets/History"
)
add_dependencies(MyFarmEngine MyFarmRuntimeAssets)

function(enable_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()
enable_project_warnings(MyFarmEngineCore)
enable_project_warnings(MyFarmEngine)

include(CTest)
if(BUILD_TESTING)
    add_executable(EngineTimeTests Tests/TimeTests.cpp)
    target_link_libraries(EngineTimeTests PRIVATE MyFarmEngineCore)
    enable_project_warnings(EngineTimeTests)
    add_test(NAME engine.time COMMAND EngineTimeTests)

    add_executable(EngineLifecycleTests Tests/LifecycleTests.cpp)
    target_link_libraries(EngineLifecycleTests PRIVATE MyFarmEngineCore)
    enable_project_warnings(EngineLifecycleTests)
    add_test(NAME engine.lifecycle COMMAND EngineLifecycleTests)
    add_test(NAME game.smoke COMMAND MyFarmEngine --smoke-test)
    add_executable(EngineCanvasTests Tests/CanvasTests.cpp)
    target_include_directories(EngineCanvasTests SYSTEM PRIVATE ThirdParty/stb)
    target_link_libraries(EngineCanvasTests PRIVATE MyFarmEngineCore)
    enable_project_warnings(EngineCanvasTests)
    add_test(NAME engine.canvas COMMAND EngineCanvasTests)
    add_dependencies(EngineCanvasTests MyFarmRuntimeAssets)
    set_tests_properties(engine.lifecycle game.smoke engine.canvas PROPERTIES
        TIMEOUT 30 LABELS "graphics" RUN_SERIAL TRUE)
endif()
