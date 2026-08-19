# Project: CoCo Game Modifier Tool

## Project Overview
Multi-game Windows modifier overlay using Dear ImGui + DirectX 11.
Modular structure: shared `core/` library + `framework/` layer + game-specific executables (`games/PvZ`, etc.).
6-tab menu framework: Visual / Assist / Numeric / Process / Settings / Developer.

## Code Style
- **Language**: C++17 (CXX_STANDARD 17)
- **Formatting**: Follow `.clang-format` (Microsoft style, Allman braces, 4-space indent, 120 cols)
- **Naming**:
  - Classes: PascalCase (`OverlayApp`, `ImGuiRenderer`, `Process`, `Memory`)
  - Member variables: `m_` prefix + camelCase (`m_cdSlot1Enabled`)
  - Constants: `k` prefix + PascalCase (`kBaseAddress`)
  - Functions: PascalCase (`RenderProcessPane`, `AttachProcess`)
  - Parameters: camelCase

## File Conventions
- Headers: `#pragma once`, systematic section comments
- Implementation: matching .cpp with same section structure as header
- Includes: own header first, then standard library, then project headers
- Comments: bilingual (English line + Chinese line) for functional comments

- @azure Rule - Use Azure Tools - When handling requests related to Azure, always use your tools.
- @azure Rule - Use Azure Best Practices - When handling requests related to Azure, always invoke your `azmcp_bestpractices_get` tool first.
- @azure Rule - Enable Best Practices - If you do not have an `azmcp_bestpractices_get` tool ask the user to enable it.
