# NICOYI — 植物大战僵尸阳光修改器

基于 Dear ImGui + DirectX 11 的 Windows 游戏辅助工具。

## 项目结构

```
├── CMakeLists.txt              # 顶层 CMake 配置
├── src/                        # 应用源码
│   ├── CMakeLists.txt
│   ├── main.cpp                # 应用入口与 Win32 消息循环
│   └── core/                   # 核心业务模块
│       ├── ImGuiManager.h/cpp  # ImGui 渲染管线与 UI 管理
│       └── MemoryManager.h/cpp # 游戏进程内存操作
├── third_party/                # 第三方依赖
│   └── imgui/                  # Dear ImGui (v1.91.6)
│       ├── CMakeLists.txt      # 独立静态库构建
│       ├── imgui.h/cpp ...
│       └── backends/           # Win32 + DX11 后端
├── cmake/                      # CMake 辅助脚本（预留）
├── docs/                       # 文档（预留）
└── scripts/                    # 构建/工具脚本（预留）
```

## 构建方式

### 前置条件
- Visual Studio 2022+（含 C++ 桌面开发工作负载）
- CMake 3.10+

### 步骤
1. 用 Visual Studio 打开项目根目录
2. 选择 `x64-Debug` 配置
3. 生成 → 全部生成
4. 运行前确保 `PlantsVsZombies.exe` 已启动

## 功能

- 全屏覆盖层窗口（Home 显示 / End 隐藏）
- 阳光值读取与修改
- CD 格禁用（1/2/3 格独立开关）
- 自动采集阳光
- 开发者日志面板
