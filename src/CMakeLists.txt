# ==============================
# 应用可执行文件
# ==============================

add_executable(NICOYI WIN32
    main.cpp
    core/ImGuiManager.cpp
    core/MemoryManager.cpp
)

# 使 #include "core/..." 样式生效
target_include_directories(NICOYI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接 ImGui 静态库
target_link_libraries(NICOYI PRIVATE imgui)

if(MSVC)
    target_compile_definitions(NICOYI PRIVATE _UNICODE UNICODE)
endif()
