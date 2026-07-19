# ==============================
# CoCoFPS — FPS Modifier (Placeholder)
# Uncomment in root CMakeLists.txt to activate
# ==============================

add_executable(CoCoFPS WIN32 main.cpp)
target_link_libraries(CoCoFPS PRIVATE CoCoCore)

if(MSVC)
    target_compile_definitions(CoCoFPS PRIVATE _UNICODE UNICODE)
endif()
