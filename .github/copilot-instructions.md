# Project: NICOYI Game Modifier Tool

## Project Overview
A generic Windows game modifier overlay using Dear ImGui + DirectX 11.
6-tab universal menu framework: Visual / Assist / Numeric / Process / Settings / Developer.

## Code Style
- **Language**: C++11 (CXX_STANDARD 11)
- **Formatting**: Follow `.clang-format` (Microsoft style, Allman braces, 4-space indent, 120 cols)
- **Naming**:
  - Classes: PascalCase (`ImGuiManager`, `MemoryManager`)
  - Member variables: `m_` prefix + camelCase (`m_cdSlot1Enabled`)
  - Constants: `k` prefix + PascalCase (`kBaseAddress`)
  - Functions: PascalCase (`RenderProcessStatus`, `AttachProcess`)
  - Parameters: camelCase

## File Conventions
- Headers: `#pragma once`, systematic Doxygen-style section comments
- Implementation: matching .cpp with same section structure as header
- Includes: own header first, then standard library, then project headers
