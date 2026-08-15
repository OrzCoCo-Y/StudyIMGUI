# CoCo — Game Modifier Tool

A generic, extensible Windows game modifier overlay built with **Dear ImGui + DirectX 11**.

Modular game-specific executables sharing a common core library and a framework layer.
6-tab universal menu: Visual / Assist / Numeric / Process / Settings / Developer.

## Project Structure

```
├── CMakeLists.txt              # Root — delegates to sub-projects
├── .clang-format               # Code style (Microsoft, Allman braces)
├── .gitignore
│
├── core/                       # Shared core library (CoCoCore)
│   ├── CMakeLists.txt
│   ├── D3D11Device.h/cpp       # D3D11 device / swap chain / render target
│   ├── ImGuiRenderer.h/cpp     # ImGui context, fonts & render pipeline
│   ├── Process.h/cpp           # Target process find / open / liveness
│   └── Memory.h/cpp            # Remote memory read/write & pointer chains
│
├── framework/                  # Shared application framework (CoCoFramework)
│   ├── CMakeLists.txt
│   ├── GameFeature.h           # Feature plugin interface
│   ├── OverlayApp.h/cpp        # Window, message loop, feature host
│   └── LogBuffer.h/cpp         # Ring-buffer log
│
├── games/                      # Concrete game executables
│   ├── PvZ/                    # ★ Current: Plants vs Zombies modifier
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp
│   │   ├── PvZFeature.h/cpp
│   │   └── PvZOffsets.h
│   ├── FPS/                    # Placeholder: FPS modifier template
│   └── RPG/                    # Placeholder: RPG modifier template
│
├── third_party/imgui/          # Dear ImGui (v1.92.x)
├── docs/
│   ├── menu-framework.md       # 6-tab menu framework spec
│   └── menu-framework-pvz.html # PvZ UI design mockup
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

1. Create `games/GameName/` with `CMakeLists.txt`, `main.cpp`, and a `GameFeature` implementation
2. Write game-specific feature logic using `core/` and `framework/` APIs
3. Add `add_subdirectory(games/GameName)` in root `CMakeLists.txt`
4. Build — the new executable links CoCoFramework (which links CoCoCore) automatically

See `docs/menu-framework.md` for the menu structure.
