# Menu Framework — Universal Game Modifier Overlay

> Last updated: 2026-07-19
> This document defines the **generic 6-tab menu framework** used across all game profiles.
> Each game (PvZ, FPS, RPG, ...) fills into this same skeleton at implementation time.

---

## Tab Layout

```
================================================================================
  [ Visual ]  [ Assist ]  [ Numeric ]  [ Process ]  [ Settings ]  [ Developer ]
================================================================================
```

---

## 1. Visual (视觉) — Overlay draw elements

Drawings rendered on top of the game screen.

| Subgroup | Description | Example Items |
|----------|-------------|---------------|
| **Player** | Player-related ESP visuals | Box ESP, Skeleton, Health Bar, Name Tag, Distance |
| **World** | World / environment overlays | Item Highlight, Fullbright, Wireframe, No Fog |
| **Weapon** | Weapon & projectile visuals | Aim FOV Circle, Target Line, Hitbox Display, Bullet Trace |
| **Misc Visual** | Other drawing helpers | Crosshair, FPS Counter, Watermark, Timer |

---

## 2. Assist (辅助) — Toggle behaviors & automation

On/off switches that modify game behavior automatically.

| Subgroup | Description | Example Items |
|----------|-------------|---------------|
| **Combat** | Combat assistance | No Recoil, No Spread, Rapid Fire, Trigger Bot, Auto Aim |
| **Movement** | Movement modifications | BHop, No Clip, Fly, Speed Hack, Infinite Sprint |
| **Automation** | Auto-collect / auto-pickup | Auto Loot, Auto Collect, Auto Dialog, Auto Farm |
| **Invincibility** | God-mode & infinite resources | God Mode, Infinite Ammo, Infinite Shield, One Hit Kill |

---

## 3. Numeric (数值) — Value editors

Numeric input fields with Apply buttons.

| Subgroup | Description | Example Items |
|----------|-------------|---------------|
| **Attributes** | Character stats | HP, MP, STR, DEX, Stamina, Mana |
| **Resources** | Currency & materials | Gold, Diamonds, Ammo Count, Material Qty |
| **Progression** | Level & experience | Level, EXP, Skill Points, Reputation |
| **Weapon Stats** | Weapon-specific values | Damage, Fire Rate, Clip Size, Range |

---

## 4. Process (进程) — Process connection & management

Target process attach/detach and runtime information.

| Subgroup | Description | Example Items |
|----------|-------------|---------------|
| **Connection** | Attach / detach controls | Attach, Detach, Reconnect, PID Display, Process Name |
| **Info** | Process runtime info | Uptime, Module List, Architecture (32/64-bit) |
| **Memory Scan** | Address range config | Start Address, End Address, Scan Region Selector |

---

## 5. Settings (设置) — Tool configuration

Tool-level preferences, not game-specific.

| Subgroup | Description | Example Items |
|----------|-------------|---------------|
| **Hotkeys** | Key bindings | Overlay Toggle (Home / End), Disable All, per-feature binds |
| **Appearance** | UI look & feel | Theme (Dark / Light), Opacity, Font Size, Language |
| **General** | Other preferences | Run on Startup, Admin Mode, Check Updates, Save/Load Config |

---

## 6. Developer (开发者) — Debugging & advanced tools

Memory inspection, pointer resolution, scripting, logging.

| Subgroup | Description | Example Items |
|----------|-------------|---------------|
| **Memory** | Raw memory browsing | Memory Viewer, Hex Editor, Address Navigator |
| **Pointer** | Pointer chain operations | Pointer Resolver, Offset Calculator, Offset Config Table |
| **Script** | Scripting console | Lua Console, Script Loader, Auto-run Scripts |
| **Log** | Operation log | Clear Log, Save to File, Auto-scroll, Timestamped Entries |

---

## Full Structure Overview

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│  [ Visual ]  [ Assist ]  [ Numeric ]  [ Process ]  [ Settings ]  [ Developer ] │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  Visual:     │ Assist:      │ Numeric:     │ Process:   │ Settings:  │ Developer:│
│  ├─ Player   │ ├─ Combat    │ ├─ Attributes │ ├─ Connect ├─ Hotkeys   │ ├─ Memory  │
│  ├─ World    │ ├─ Movement  │ ├─ Resources  │ ├─ Info    ├─ Appearance│ ├─ Pointer │
│  ├─ Weapon   │ ├─ Auto      │ ├─ Progression│ ├─ Scan    ├─ General   │ ├─ Script  │
│  └─ Misc     │ └─ Invincible│ └─ Weapon     │            │            │ └─ Log     │
│              │              │               │            │            │           │
└─────────────────────────────────────────────────────────────────────────────────┘
```

## Game Profile Mapping

Each game profile maps its features into these 6 tabs:

```cpp
// Example: PvZ profile fills into:
//   Visual   → Plant range, Zombie HP bar
//   Assist   → No CD (3 slots), Auto collect
//   Numeric  → Sunshine value
//   Process  → PlantsVsZombies.exe
//   Settings → (shared, not game-specific)
//   Developer→ (shared, not game-specific)

// Example: FPS profile fills into:
//   Visual   → Box ESP, Skeleton, Health bar
//   Assist   → No recoil, Trigger bot, Infinite ammo
//   Numeric  → HP, Ammo count, Move speed
//   Process  → cs2.exe
```

> **Note:** `Settings` and `Developer` tabs are **global** — shared across all game profiles.
> `Visual`, `Assist`, `Numeric` are **per-game** — switched automatically on process detection.


---

## Comment Convention (注释规范)

All source code uses **bilingual comments** — English first, Chinese below — to ensure
accessibility for both international and Chinese-speaking developers.

### Format

`cpp
// English comment describing the purpose
// 中文注释说明用途
int value = 42;

// ==============================
// Section Header (English)
// 中文段落标题
// ==============================

// Inline comment for a member variable
int m_someValue;  // English (中文说明)
`

### Rules

| Rule | Description |
|------|-------------|
| **File headers** | English section header \n Chinese section header |
| **Section dividers** | Bilingual block comments (// =====) |
| **Member comments** | Inline // English (中文) on the same line |
| **Functional comments** | English line + Chinese line before the code |
| **UI strings** | Keep in Chinese (target users are Chinese-speaking), with English comments above |
| **Third-party code** | Leave as-is, do not modify |

### Examples

`cpp
// ==============================
// Initialize & Shutdown
// 初始化与销毁
// ==============================

// Global instance declaration
// 全局实例声明
extern MemoryManager g_memoryManager;

// Forward declaration from imgui_impl_win32.cpp
// 来自 imgui_impl_win32.cpp 的前置声明
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(...);
`
