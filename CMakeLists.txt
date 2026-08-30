cmake_minimum_required(VERSION 3.16)
project(DxG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- 库：除 SDL 平台外的全部框架代码（不依赖 SDL2） ----
# 递归收集各模块 src 下的源码（world 等模块内部含子包目录，如 src/world/src/terrain/）。
file(GLOB_RECURSE DXG_CORE_SOURCES CONFIGURE_DEPENDS "src/*/src/*.cpp")
# platform 含 SDL 代码，不进 dxg_core。
list(FILTER DXG_CORE_SOURCES EXCLUDE REGEX "[\\/]platform[\\/]")

add_library(dxg_core STATIC ${DXG_CORE_SOURCES})

# 每个模块的 include 根（头文件以 <模块>/<子包>/<头名>.h 引用）。
# 递归发现所有名为 include 的目录，兼容模块内部的子包结构。
file(GLOB_RECURSE DXG_INCLUDE_CANDIDATES CONFIGURE_DEPENDS LIST_DIRECTORIES true "src/*/include")
set(DXG_INCLUDE_DIRS "")
foreach(dir ${DXG_INCLUDE_CANDIDATES})
    if(IS_DIRECTORY "${dir}")
        list(APPEND DXG_INCLUDE_DIRS "${dir}")
    endif()
endforeach()
target_include_directories(dxg_core PUBLIC ${DXG_INCLUDE_DIRS})

if(MSVC)
    target_compile_options(dxg_core PRIVATE /W4)
else()
    target_compile_options(dxg_core PRIVATE -Wall -Wextra)
endif()

# ---- 可执行文件：主程序 + SDL 平台层（需要 SDL2） ----
find_package(SDL2 QUIET)

add_executable(dxg
    src/main.cpp
    src/platform/src/SdlPlatform.cpp
)
target_link_libraries(dxg PRIVATE dxg_core)
target_include_directories(dxg PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/include"
)

if(SDL2_FOUND)
    target_link_libraries(dxg PRIVATE SDL2::SDL2)
else()
    message(WARNING "SDL2 not found: the 'dxg' executable may fail to link. "
                    "dxg_core library itself builds without SDL2.")
endif()
