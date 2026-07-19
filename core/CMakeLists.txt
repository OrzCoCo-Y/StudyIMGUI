# ==============================
# CoCoCore - Shared static library
# ==============================

add_library(CoCoCore STATIC
    ImGuiManager.cpp
    MemoryManager.cpp
)

if(MSVC)
    target_compile_definitions(CoCoCore PRIVATE _UNICODE UNICODE)
endif()

# Allow #include "core/xxx.h" and #include "xxx.h"
target_include_directories(CoCoCore
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(CoCoCore PUBLIC imgui)
