cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0063 NEW)
project(Res)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Include RayLib
FetchContent_Declare(
        RayLib
        GIT_REPOSITORY "https://github.com/raysan5/raylib"
        GIT_TAG "5.5"
)
FetchContent_MakeAvailable(RayLib)

# Include Jolt Physics
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.3.0"
        SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)

# Include flecs
FetchContent_Declare(
        flecs
        GIT_REPOSITORY "https://github.com/SanderMertens/flecs"
        GIT_TAG "v4.1.0"
)
FetchContent_MakeAvailable(flecs)

set_target_properties(flecs PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
)

# Include ImGui (fetch to build directory)
FetchContent_Declare(
        ImGui
        GIT_REPOSITORY "https://github.com/ocornut/imgui"
        GIT_TAG "v1.92.2"
)
FetchContent_MakeAvailable(ImGui)

# Include rlImGui
FetchContent_Declare(
        rlImGui
        GIT_REPOSITORY "https://github.com/raylib-extras/rlImGui"
        GIT_TAG "main"
)
FetchContent_MakeAvailable(rlImGui)

set(SOURCES
        src/TransformComponents.h
        src/Window.h
        src/Window.cpp
        src/RenderComponents.h
        src/HelperComponents.h
        src/Phases.h
        src/Phases.cpp
        src/RenderSystems.h
        src/RenderSystems.cpp
        src/DebugSystems.h
        src/DebugSystems.cpp
        src/TransformSystems.h
        src/TransformSystems.cpp
        src/MathUtils.h
        src/PhysicsSystems.h
        src/PhysicsSystems.cpp
        src/PhysicsComponents.h
        src/JoltUtils.h
        src/JoltUtils.cpp
)

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

set(RLIMGUI_SOURCES
        ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

add_library(${PROJECT_NAME} STATIC
        ${SOURCES}
        ${IMGUI_SOURCES}
        ${RLIMGUI_SOURCES}
)

add_library(Res::Res ALIAS ${PROJECT_NAME})
# add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${RLIMGUI_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${imgui_SOURCE_DIR}
        ${rlimgui_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC raylib Jolt flecs::flecs_static)

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_SOURCE_DIR}/assets"
#        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
#        COMMENT "Copying assets to output directory"
#)
