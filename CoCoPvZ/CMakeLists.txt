# ==============================
# CoCoPvZ - Plants vs Zombies Modifier
# ==============================

add_executable(CoCoPvZ WIN32
    main.cpp
)

target_link_libraries(CoCoPvZ PRIVATE CoCoCore)

if(MSVC)
    target_compile_definitions(CoCoPvZ PRIVATE _UNICODE UNICODE)
endif()
