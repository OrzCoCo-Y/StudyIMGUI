# NICOYI — Game Modifier Tool

A generic, extensible Windows game modifier overlay built with **Dear ImGui + DirectX 11**.

Designed as a universal framework: the same 6-tab menu structure is reused across
different game profiles (PvZ/TD, FPS, RPG, etc.). Automatically detects the target
process and switches feature sets.

## Project Structure

```
├── CMakeLists.txt              # Root CMake config
├── .clang-format               # Code style (Microsoft, Allman braces)
├── .gitignore                  # Ignored files
│
├── src/                        # Application source
│   ├── CMakeLists.txt
│   ├── main.cpp                # Entry point + Win32 message loop
│   └── core/                   # Core modules
│       ├── ImGuiManager.h/cpp  # ImGui render pipeline & UI
│       └── MemoryManager.h/cpp # Process memory operations
│
├── third_party/                # Third-party dependencies
│   └── imgui/                  # Dear ImGui (v1.91.6)
│
├── docs/                       # Design & architecture docs
│   └── menu-framework.md       # Universal menu framework spec
│
├── designs/                    # UI mockups & design assets
├── cmake/                      # CMake helper scripts
└── scripts/                    # Build & utility scripts
```

## Menu Framework

The tool uses a **6-tab universal menu**. See `docs/menu-framework.md` for details.

```
[ Visual ]  [ Assist ]  [ Numeric ]  [ Process ]  [ Settings ]  [ Developer ]
```

| Tab | Purpose |
|-----|---------|
| **Visual** | Overlay draw elements (ESP, boxes, health bars, FOV circles) |
| **Assist** | Toggle behaviors & automation (no CD, auto collect, trigger bot) |
| **Numeric** | Value editors (HP, gold, ammo, sunshine) |
| **Process** | Process attach/detach & runtime info |
| **Settings** | Tool configuration (hotkeys, theme, language) |
| **Developer** | Memory viewer, pointer resolver, scripting, log |

## Build

### Prerequisites
- Visual Studio 2022+ (with C++ desktop workload)
- CMake 3.10+

### Steps
1. Open the project root in Visual Studio
2. Select `x64-Debug` configuration
3. Build > Build All

### Hotkeys
- **HOME** — Show overlay
- **END**  — Hide overlay
