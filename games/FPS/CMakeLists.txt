# ==============================
# CoCoFPS — FPS Modifier (Template)
# ==============================

add_executable(CoCoFPS WIN32
    main.cpp
    FPSFeature.cpp
)

target_link_libraries(CoCoFPS PRIVATE CoCoFramework)

if(MSVC)
    target_compile_definitions(CoCoFPS PRIVATE _UNICODE UNICODE)
endif()
