# ==============================
# Root CMakeLists.txt
# Project: CoCo - Game Modifier Tool
# ==============================

cmake_minimum_required(VERSION 3.10)
project(CoCo VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Shared third-party library
add_subdirectory(third_party/imgui)

# Shared core library (ImGui manager + memory operations)
add_subdirectory(core)

# Game-specific executables
add_subdirectory(CoCoPvZ)
# add_subdirectory(CoCoFPS)    # Placeholder: uncomment when FPS game is added
# add_subdirectory(CoCoRPG)    # Placeholder: uncomment when RPG game is added
