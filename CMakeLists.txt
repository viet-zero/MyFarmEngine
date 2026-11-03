cmake_minimum_required(VERSION 3.20)
project(MyFarmEngine VERSION 0.1.0 LANGUAGES C CXX)

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
    Engine/Input/Input.cpp
)
target_include_directories(MyFarmEngineCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Engine)
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
    set_tests_properties(engine.lifecycle game.smoke PROPERTIES
        TIMEOUT 30 LABELS "graphics" RUN_SERIAL TRUE)
endif()
