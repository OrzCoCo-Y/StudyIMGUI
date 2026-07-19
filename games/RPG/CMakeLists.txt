# ==============================
# CoCoRPG — RPG Modifier (Template)
# ==============================

add_executable(CoCoRPG WIN32
    main.cpp
    RPGFeature.cpp
)

target_link_libraries(CoCoRPG PRIVATE CoCoFramework)

if(MSVC)
    target_compile_definitions(CoCoRPG PRIVATE _UNICODE UNICODE)
endif()
