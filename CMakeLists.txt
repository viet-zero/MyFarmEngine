cmake_minimum_required(VERSION 3.20)

project(MyFarmEngine VERSION 1.0 LANGUAGES C CXX)

# ==============================
# C++ Standard
# ==============================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#1. Khai báo ThirdParty
#2. Tạo Executable
#3. Link thư viện

# ==============================
# Third Party
# ==============================
add_subdirectory(ThirdParty/glfw)

add_library(glad
    ThirdParty/glad/src/glad.c
 "Game/Systems/ToolComponent.h")

target_include_directories(glad
    PUBLIC
        ThirdParty/glad/include
)

# ==============================
# Executable
# ==============================
add_executable(MyFarmEngine
    Game/main.cpp
    Game/GameApp/Game.cpp

    Engine/Core/Application.cpp
    Engine/Core/Window.cpp
    Engine/Core/Engine.cpp
    Engine/Core/Time.cpp
    
    Engine/Graphics/Renderer.cpp

    Engine/Input/Input.cpp
 "Game/Systems/ToolComponent.h")

target_include_directories(MyFarmEngine
    PRIVATE
        Engine
        ThirdParty/glm
        ThirdParty/stb
)

target_link_libraries(MyFarmEngine
    PRIVATE
        glfw
        glad
)