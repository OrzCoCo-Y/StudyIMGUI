# ==============================
# CoCoPvZ — Plants vs Zombies Modifier
# ==============================

add_executable(CoCoPvZ WIN32
    main.cpp
    PvZFeature.cpp
)

target_link_libraries(CoCoPvZ PRIVATE CoCoFramework)

if(MSVC)
    target_compile_definitions(CoCoPvZ PRIVATE _UNICODE UNICODE)
endif()
