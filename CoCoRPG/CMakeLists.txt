# ==============================
# CoCoRPG — RPG Modifier (Placeholder)
# Uncomment in root CMakeLists.txt to activate
# ==============================

add_executable(CoCoRPG WIN32 main.cpp)
target_link_libraries(CoCoRPG PRIVATE CoCoCore)

if(MSVC)
    target_compile_definitions(CoCoRPG PRIVATE _UNICODE UNICODE)
endif()
