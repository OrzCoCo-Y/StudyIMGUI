# ==============================
# Root CMakeLists.txt
# 项目：植物大战僵尸阳光修改器 (NICOYI)
# 顶层配置，委托子目录完成具体构建。
# ==============================

cmake_minimum_required(VERSION 3.10)
project(NICOYI_IMGUI VERSION 1.0)

# ---------- 全局编译选项 ----------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# ---------- 第三方依赖 ----------
add_subdirectory(third_party/imgui)

# ---------- 应用源码 ----------
add_subdirectory(src)
