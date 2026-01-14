cmake_minimum_required(VERSION 3.16)
project(sentinel_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- resolve projects root ----
get_filename_component(PROJECTS_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/.."
    ABSOLUTE
)

message(STATUS "Projects root = ${PROJECTS_ROOT}")

# ---- orchestra dependency ----
add_subdirectory(
    "${PROJECTS_ROOT}/sentinel-orchestra"
    "${CMAKE_BINARY_DIR}/orchestra_build"
)

# ---- SDL2 ----
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# ---- demo executable ----
add_executable(sentinel-demo
    src/main.cpp
)

target_include_directories(sentinel-demo
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${PROJECTS_ROOT}/sentinel-orchestra/src
)

target_link_libraries(sentinel-demo
    PRIVATE
        sentinel-orchestra
        ${SDL2_LIBRARIES}
)

target_compile_options(sentinel-demo
    PRIVATE ${SDL2_CFLAGS_OTHER}
)
