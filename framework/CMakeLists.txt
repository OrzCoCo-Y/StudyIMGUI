# ==============================
# CoCoFramework — Game modifier application framework
# Depends on CoCoCore. Each game registers as a GameFeature plugin.
# ==============================

add_library(CoCoFramework STATIC
    OverlayApp.cpp
    LogBuffer.cpp
)

target_include_directories(CoCoFramework
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(CoCoFramework PUBLIC CoCoCore)

if(MSVC)
    target_compile_definitions(CoCoFramework PRIVATE _UNICODE UNICODE)
endif()
