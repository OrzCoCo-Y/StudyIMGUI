# ==============================
# Root CMakeLists.txt
# Project: CoCo — Game Modifier Tool
# ==============================

cmake_minimum_required(VERSION 3.10)
project(CoCo VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# ============================================================
# Layer 1: External dependencies
# ============================================================
add_subdirectory(third_party/imgui)

# ============================================================
# Layer 2: Core — infrastructure (pure, no game logic)
#   D3D11Device / ImGuiRenderer / Process / Memory
# ============================================================
add_subdirectory(core)

# ============================================================
# Layer 3: Framework — game-modifier application framework
#   GameFeature interface / OverlayApp / LogBuffer
# ============================================================
add_subdirectory(framework)

# ============================================================
# Layer 4: Games — concrete feature implementations
#   Each game is an executable linking CoCoFramework.
# ============================================================
add_subdirectory(games/PvZ)

# Placeholder games (uncomment when ready):
# add_subdirectory(games/FPS)
# add_subdirectory(games/RPG)
