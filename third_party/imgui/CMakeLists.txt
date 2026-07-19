# ==============================
# Dear ImGui 静态库
# 封装 ImGui 核心 + Win32/DX11 后端为独立构建单元。
# ==============================

set(IMGUI_SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp
    backends/imgui_impl_win32.cpp
    backends/imgui_impl_dx11.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

# 暴露头文件搜索路径，使下游 target 可通过 #include "imgui.h" 引用
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

if(MSVC)
    target_compile_definitions(imgui PRIVATE _UNICODE UNICODE)
    target_link_libraries(imgui PUBLIC d3d11 dxgi)
endif()
