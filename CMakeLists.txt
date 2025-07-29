cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(Resident)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
# Dependencies
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib

    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
    endif ()
endif ()

# Include Jolt
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.3.0"
        SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)

# Include Flecs
FetchContent_Declare(
        flecs
        GIT_REPOSITORY "https://github.com/SanderMertens/flecs"
)
FetchContent_MakeAvailable(flecs)

# Our Project
set(SOURCES
        src/main.cpp
        src/App.h
        src/App.cpp
        src/TransformComponents.h
        src/Window.h
        src/Window.cpp
        src/RenderComponents.h
        src/HelperComponents.h
)

set(VENDOR_SOURCES
        vendor/rlImGui/rlImGui.cpp
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_tables.cpp
        vendor/imgui/imgui_widgets.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${VENDOR_SOURCES})


target_link_libraries(${PROJECT_NAME} raylib)

target_include_directories(${PROJECT_NAME} PUBLIC ${JoltPhysics_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rlimGui
)

target_link_libraries(${PROJECT_NAME} Jolt)

target_link_libraries(${PROJECT_NAME} flecs::flecs_static)

add_custom_command(TARGET Resident POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Resident>/assets"
        COMMENT "Copying assets to output directory"
)