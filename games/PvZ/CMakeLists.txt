# ==============================
# CoCoPvZ — Plants vs Zombies Modifier
# ==============================

add_executable(CoCoPvZ WIN32
    main.cpp
    PvZFeature.cpp
    PlantCallCommon.cpp
    PlantCallA.cpp
    PlantCallB.cpp
    PlantCallDll.cpp
)

target_link_libraries(CoCoPvZ PRIVATE CoCoFramework)

if(MSVC)
    target_compile_definitions(CoCoPvZ PRIVATE _UNICODE UNICODE)
endif()

# --- Injected mod DLL (scheme C) — must match PvZ bitness (32-bit) ---
add_library(CoCoPvZMod SHARED
    injected/CoCoPvZMod.cpp
)

target_include_directories(CoCoPvZMod PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(MSVC)
    target_compile_definitions(CoCoPvZMod PRIVATE _UNICODE UNICODE)
endif()

# Place DLL next to CoCoPvZ.exe
set_target_properties(CoCoPvZMod PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:CoCoPvZ>"
    LIBRARY_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:CoCoPvZ>"
)

add_dependencies(CoCoPvZ CoCoPvZMod)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING
        "CoCoPvZMod targets 32-bit Plants vs Zombies. "
        "Build Win32 (x86) configuration for DLL injection to work.")
endif()
