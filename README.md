# CoCo — Game Modifier Tool

A generic, extensible Windows game modifier overlay built with **Dear ImGui + DirectX 11**.

Modular game-specific executables sharing a common core library.
6-tab universal menu: Visual / Assist / Numeric / Process / Settings / Developer.

## Project Structure

```
├── CMakeLists.txt              # Root — delegates to sub-projects
├── .clang-format               # Code style (Microsoft, Allman braces)
├── .gitignore
│
├── core/                       # Shared core library (CoCoCore)
│   ├── CMakeLists.txt
│   ├── ImGuiManager.h/cpp      # ImGui render pipeline & UI
│   └── MemoryManager.h/cpp     # Process memory operations
│
├── CoCoPvZ/                    # ★ Current: Plants vs Zombies modifier
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── CoCoFPS/                    # Placeholder: FPS modifier
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── CoCoRPG/                    # Placeholder: RPG modifier
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── third_party/imgui/          # Dear ImGui (v1.91.6)
├── docs/
│   └── menu-framework.md       # 6-tab menu framework spec
├── designs/                    # UI mockups (reserved)
├── cmake/                      # CMake helpers (reserved)
└── scripts/                    # Build scripts (reserved)
```

## Building

### Prerequisites
- Visual Studio 2022+ (C++ desktop workload)
- CMake 3.10+

### Steps
1. Open the project root in Visual Studio
2. Select `x64-Debug` configuration
3. Build > Build All
4. Select **CoCoPvZ** as the startup project
5. Launch PlantsVsZombies.exe, then run CoCoPvZ

### Hotkeys
- **HOME** — Show overlay
- **END**  — Hide overlay

## Adding a New Game

1. Create `CoCoGameName/` with `CMakeLists.txt` and `main.cpp`
2. Write game-specific feature logic using `core/` APIs
3. Uncomment `add_subdirectory(CoCoGameName)` in root `CMakeLists.txt`
4. Build — the new executable links CoCoCore automatically

See `docs/menu-framework.md` for the menu structure.
