# ==============================
# CMake 构建配置：植物大战僵尸阳光修改器
# 项目类型：Windows Win32 桌面应用 + Dear ImGui + DirectX 11
# ==============================

cmake_minimum_required(VERSION 3.10)
project(NICOYI_IMGUI VERSION 1.0)

# 要求 C++11 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# MSVC 下强制源文件以 UTF-8 编码编译，避免中文注释乱码
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 将项目根目录加入头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR})

# Dear ImGui 核心源文件 + Win32/DX11 后端
set(IMGUI_SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp
    backends/imgui_impl_win32.cpp
    backends/imgui_impl_dx11.cpp
)

# 生成 Win32 GUI 应用程序（不自动打开控制台窗口）
add_executable(NICOYI WIN32 main.cpp ImGuiManager.cpp MemoryManager.cpp ${IMGUI_SOURCES})

if(MSVC)
    # 定义 UNICODE 宏以使用宽字符 Win32 API
    target_compile_definitions(NICOYI PRIVATE _UNICODE UNICODE)
    # 链接 DirectX 11 库
    target_link_libraries(NICOYI d3d11 dxgi)
endif()
