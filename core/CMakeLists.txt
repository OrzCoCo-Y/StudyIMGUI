# ==============================
# CoCoCore — Shared static library (Foundation Layer)
# Pure infrastructure: D3D11, ImGui, Process, Memory.
# No game-specific code belongs here.
# ==============================

add_library(CoCoCore STATIC
    D3D11Device.cpp
    ImGuiRenderer.cpp
    Process.cpp
    Memory.cpp
)

target_include_directories(CoCoCore
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(CoCoCore PUBLIC imgui)

if(MSVC)
    target_compile_definitions(CoCoCore PRIVATE _UNICODE UNICODE)
endif()
