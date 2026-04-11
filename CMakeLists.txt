cmake_minimum_required(VERSION 3.10)
project(NICOYI_IMGUI VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(${CMAKE_SOURCE_DIR})

set(IMGUI_SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp
    backends/imgui_impl_win32.cpp
    backends/imgui_impl_dx11.cpp
)

add_executable(NICOYI WIN32 main.cpp ImGuiManager.cpp MemoryManager.cpp ${IMGUI_SOURCES})

if(MSVC)
    target_compile_definitions(NICOYI PRIVATE _UNICODE UNICODE)
    target_link_libraries(NICOYI d3d11 dxgi)
endif()
