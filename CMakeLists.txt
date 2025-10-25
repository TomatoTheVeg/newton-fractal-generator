cmake_minimum_required(VERSION 3.15)
project(NewtonGUI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# Paths (adjust IMGUI_DIR if your layout differs)
# ------------------------------------------------------------------------------
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui")

# ------------------------------------------------------------------------------
# System dependencies (Arch: pacman -S base-devel cmake git sdl2 glew mesa ispc)
# ------------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)      # SDL2::SDL2 and SDL2_INCLUDE_DIRS
find_package(GLEW REQUIRED)      # GLEW::GLEW
find_program(ISPC_EXECUTABLE ispc REQUIRED)

# ------------------------------------------------------------------------------
# ISPC kernel build (newtonApprox.ispc -> object + header)
# ------------------------------------------------------------------------------
# Adjust the source path if yours differs:
set(ISPC_SOURCE "${CMAKE_SOURCE_DIR}/newtonApprox.ispc")
set(ISPC_OUT_DIR "${CMAKE_BINARY_DIR}/ispc")
set(ISPC_OBJECT  "${ISPC_OUT_DIR}/newtonApprox_ispc.o")
set(ISPC_HEADER  "${ISPC_OUT_DIR}/newtonApprox_ispc.h")

add_custom_command(
    OUTPUT "${ISPC_OBJECT}" "${ISPC_HEADER}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${ISPC_OUT_DIR}"
    COMMAND "${ISPC_EXECUTABLE}" "${ISPC_SOURCE}"
            --target=avx2-i32x8        # choose a target suitable for your CPU
            -o "${ISPC_OBJECT}"
            -h "${ISPC_HEADER}"
    DEPENDS "${ISPC_SOURCE}"
    COMMENT "Compiling ISPC kernel: ${ISPC_SOURCE}"
    VERBATIM
)

add_custom_target(ispc_newton ALL
    DEPENDS "${ISPC_OBJECT}" "${ISPC_HEADER}"
)

# ------------------------------------------------------------------------------
# Core render library (wraps compute step and calls approxISPC)
# ------------------------------------------------------------------------------
set(NEWTON_CORE_SOURCES
    "${CMAKE_SOURCE_DIR}/render_newton.cpp"
    "${CMAKE_SOURCE_DIR}/ispc_tasksys_min.cpp"   # <-- minimal ISPC task runtime (you create this file)
)

# Optional C++ shim if you have it:
if(EXISTS "${CMAKE_SOURCE_DIR}/newtonApprox.cpp")
    list(APPEND NEWTON_CORE_SOURCES "${CMAKE_SOURCE_DIR}/newtonApprox.cpp")
endif()

add_library(newton_core ${NEWTON_CORE_SOURCES})
add_dependencies(newton_core ispc_newton)

# Link the ISPC object into the core lib
target_sources(newton_core PRIVATE "${ISPC_OBJECT}")

target_include_directories(newton_core PUBLIC
    "${CMAKE_SOURCE_DIR}"     # for render_newton.h, newtonApprox.h (your headers)
    "${ISPC_OUT_DIR}"         # for generated newtonApprox_ispc.h if you include it
)
set_target_properties(newton_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------------------
# LodePNG (PNG writer)
# ------------------------------------------------------------------------------
add_library(lodepng "${CMAKE_SOURCE_DIR}/lodepng.cpp")
target_include_directories(lodepng PUBLIC "${CMAKE_SOURCE_DIR}")
set_target_properties(lodepng PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------------------
# Dear ImGui (SDL2 + OpenGL3 backend)
# ------------------------------------------------------------------------------
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

# Sanity check ImGui files exist
foreach(_f IN LISTS IMGUI_SOURCES)
    if(NOT EXISTS "${_f}")
        message(FATAL_ERROR "ImGui source not found: ${_f}\nSet IMGUI_DIR correctly.")
    endif()
endforeach()

add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    ${SDL2_INCLUDE_DIRS}       # so <SDL.h> resolves (/usr/include/SDL2 on Arch)
)
# Use GLEW as the GL loader in the backend
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(imgui PUBLIC SDL2::SDL2 OpenGL::GL GLEW::GLEW)
set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------------------
# GUI executable
# ------------------------------------------------------------------------------
add_executable(gui_newton
    "${CMAKE_SOURCE_DIR}/gui_newton.cpp"
)

# Windows extras (safe to keep; no effect on Linux)
if(WIN32)
    target_link_libraries(gui_newton PRIVATE SDL2::SDL2main)
    target_compile_definitions(gui_newton PRIVATE SDL_MAIN_HANDLED)
endif()

target_link_libraries(gui_newton PRIVATE
    newton_core
    lodepng
    imgui
    SDL2::SDL2
    OpenGL::GL
    GLEW::GLEW
)

# ------------------------------------------------------------------------------
# Warnings
# ------------------------------------------------------------------------------
if(MSVC)
    target_compile_options(gui_newton PRIVATE /W4)
    target_compile_options(newton_core PRIVATE /W4)
    target_compile_options(imgui PRIVATE /W4)
else()
    target_compile_options(gui_newton PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(newton_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(imgui PRIVATE -Wall -Wextra -Wpedantic)
endif()
