#include "PvZFeature.h"
#include "PvZOffsets.h"

#include "core/Memory.h"
#include "core/Core.h"

#include "imgui.h"

#include <psapi.h>
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>

#pragma comment(lib, "psapi.lib")

namespace coco {
namespace {

// ==============================
// Palette — matches docs/menu-framework-pvz.html
// 调色板 — 与设计稿一致
// ==============================
constexpr ImVec4 kWindowBg   (0.102f, 0.102f, 0.141f, 1.0f);  // #1a1a24
constexpr ImVec4 kSurfaceBg  (0.133f, 0.133f, 0.180f, 1.0f);  // #22222e
constexpr ImVec4 kHoverBg    (0.173f, 0.173f, 0.227f, 1.0f);  // #2c2c3a
constexpr ImVec4 kActiveBg   (0.208f, 0.208f, 0.290f, 1.0f);  // #35354a
constexpr ImVec4 kBorder     (0.180f, 0.180f, 0.243f, 1.0f);  // #2e2e3e
constexpr ImVec4 kText       (0.831f, 0.831f, 0.894f, 1.0f);  // #d4d4e4
constexpr ImVec4 kTextDim    (0.424f, 0.424f, 0.518f, 1.0f);  // #6c6c84
constexpr ImVec4 kTextBright (0.941f, 0.941f, 1.000f, 1.0f);  // #f0f0ff
constexpr ImVec4 kAccent     (0.424f, 0.549f, 1.000f, 1.0f);  // #6c8cff
constexpr ImVec4 kGreen      (0.435f, 0.812f, 0.592f, 1.0f);  // #6fcf97
constexpr ImVec4 kRed        (0.922f, 0.341f, 0.341f, 1.0f);  // #eb5757
constexpr ImVec4 kOrange     (0.949f, 0.600f, 0.290f, 1.0f);  // #f2994a

const ImVec4 kAccentBg(kAccent.x, kAccent.y, kAccent.z, 0.10f);
const ImVec4 kHoverAccent(kAccent.x, kAccent.y, kAccent.z, 0.18f);
const ImVec4 kActiveAccent(kAccent.x, kAccent.y, kAccent.z, 0.24f);

ImU32 ToU32(const ImVec4& color) {
    return ImGui::ColorConvertFloat4ToU32(color);
}

// ==============================
// Theme push/pop
// 主题样式推入 / 弹出
// ==============================
void PushTheme() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, kWindowBg);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kWindowBg);
    ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
    ImGui::PushStyleColor(ImGuiCol_Text, kText);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, kTextDim);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, kSurfaceBg);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, kHoverBg);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, kActiveBg);
    ImGui::PushStyleColor(ImGuiCol_Button, kSurfaceBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kHoverBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kActiveBg);
    ImGui::PushStyleColor(ImGuiCol_CheckMark, kAccent);
    ImGui::PushStyleColor(ImGuiCol_Header, kAccentBg);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kHoverAccent);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, kActiveAccent);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, kWindowBg);
    ImGui::PushStyleColor(ImGuiCol_Separator, kBorder);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, kSurfaceBg);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, kSurfaceBg);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
}

void PopTheme() {
    ImGui::PopStyleColor(19);
    ImGui::PopStyleVar(5);
}

// ==============================
// Menu structure — from docs/menu-framework-pvz.html
// 菜单结构 — 来自设计稿
// ==============================
struct GroupDef {
    const char* name;  // 分组名
    int         done;  // 已实现数量
    int         total; // 条目总数
};

constexpr GroupDef kGroups[6][4] = {
    { { "玩家", 0, 5 }, { "世界", 0, 4 }, { "武器", 0, 4 }, { "其他视觉", 0, 4 } },
    { { "战斗", 3, 5 }, { "移动", 0, 4 }, { "自动化", 1, 3 }, { "无敌", 0, 3 } },
    { { "属性", 0, 4 }, { "资源", 1, 3 }, { "进度", 0, 2 }, { "武器属性", 0, 4 } },
    { { "连接", 5, 5 }, { "信息", 3, 3 }, { "内存扫描", 0, 3 }, { nullptr, 0, 0 } },
    { { "快捷键", 2, 3 }, { "外观", 0, 4 }, { "通用", 0, 4 }, { nullptr, 0, 0 } },
    { { "内存", 0, 3 }, { "指针", 0, 3 }, { "脚本", 0, 3 }, { "日志", 4, 4 } },
};

constexpr int kGroupCount[6] = { 4, 4, 4, 3, 3, 4 };

const char* const kTabNames[6] = { "视觉", "辅助", "数值", "进程", "设置", "开发者" };

const char* const kTabDescs[6] = {
    "游戏画面叠加绘制元素",
    "开关型行为修改 — NoCD / 自动采集",
    "数值编辑器 — 阳光值修改",
    "进程连接管理 — 附加 / 分离 / 信息",
    "工具级偏好配置",
    "调试与高级工具 — 日志",
};

// ==============================
// Row helpers
// 行控件辅助函数
// ==============================

// Small status chip (已实现 / 待扩展), drawn without interaction
// 状态小标签，纯展示无交互
void TagChip(const char* text, const ImVec4& color) {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const float padX = 6.0f, padY = 1.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos,
                      ImVec2(pos.x + textSize.x + padX * 2,
                             pos.y + textSize.y + padY * 2),
                      ToU32(ImVec4(color.x, color.y, color.z, 0.10f)), 3.0f);
    dl->AddText(ImVec2(pos.x + padX, pos.y + padY), ToU32(color), text);
    ImGui::Dummy(ImVec2(textSize.x + padX * 2, textSize.y + padY * 2));
}

// Row header: label + status chip, then right-align the control area
// 行头部：标签 + 状态标签，随后将控件右对齐
void RowBegin(const char* label, bool implemented, float controlWidth) {
    const float avail = ImGui::GetContentRegionAvail().x;
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(implemented ? kText : kTextDim, "%s", label);
    ImGui::SameLine();
    TagChip(implemented ? "已实现" : "待扩展", implemented ? kGreen : kTextDim);
    ImGui::SameLine(avail - controlWidth);
}

// Toggle row — returns true when the toggle changed
// 开关行 — 发生变化时返回 true
bool RowToggle(const char* label, bool implemented, bool* value) {
    RowBegin(label, implemented, 46.0f);
    bool changed = false;
    if (!implemented) {
        ImGui::BeginDisabled();
        bool dummy = false;
        ImGui::Checkbox("##cb", &dummy);
        ImGui::EndDisabled();
    } else {
        changed = ImGui::Checkbox("##cb", value);
    }
    ImGui::Spacing();
    return changed;
}

// Read-only value row
// 只读数值行
void RowValue(const char* label, bool implemented, const char* value,
              const ImVec4& valueColor) {
    const float valueWidth = ImGui::CalcTextSize(value).x;
    RowBegin(label, implemented, valueWidth);
    ImGui::TextColored(valueColor, "%s", value);
    ImGui::Spacing();
}

// Key-binding chip row (e.g. Home / End)
// 快捷键标签行
void RowKeyChip(const char* label, bool implemented, const char* key) {
    const float keyWidth = ImGui::CalcTextSize(key).x + 16.0f;
    RowBegin(label, implemented, keyWidth);
    TagChip(key, kAccent);
    ImGui::Spacing();
}

// Button row — returns true when clicked
// 按钮行 — 点击时返回 true
bool RowButton(const char* label, bool implemented, const char* buttonText,
               const ImVec4& color, bool enabled) {
    RowBegin(label, implemented, 92.0f);
    if (!enabled) ImGui::BeginDisabled();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.x, color.y, color.z, 0.14f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x, color.y, color.z, 0.26f));
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    const bool clicked = ImGui::Button(buttonText, ImVec2(84, 0));
    ImGui::PopStyleColor(3);
    if (!enabled) ImGui::EndDisabled();
    ImGui::Spacing();
    return clicked;
}

// Planned-only placeholder rows
// 仅占位（待扩展）的行列表
void RowPlaceholders(const char* const* names, int count) {
    for (int i = 0; i < count; ++i)
        RowToggle(names[i], false, nullptr);
}

// Pane header: title + description + status chip
// 面板头部：标题 + 描述 + 状态徽章
void PaneHeader(const char* title, const char* desc, int done, int total) {
    ImGui::TextColored(kTextBright, "%s", title);
    ImGui::TextColored(kTextDim, "%s", desc);
    ImGui::Spacing();

    TagChip(done == 0 ? "待扩展" : "已实现", done == 0 ? kTextDim : kGreen);
    ImGui::SameLine();
    char buf[32];
    snprintf(buf, sizeof(buf), "%d/%d", done, total);
    ImGui::TextColored(kTextDim, "%s", buf);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

// ==============================
// Process helpers
// 进程信息辅助函数
// ==============================

// Process uptime string (HH:MM:SS)
// 进程运行时长（HH:MM:SS）
bool FormatUptime(HANDLE handle, char* out, size_t size) {
    if (!handle) return false;
    FILETIME create{}, exit{}, kernel{}, user{};
    if (!::GetProcessTimes(handle, &create, &exit, &kernel, &user)) return false;
    FILETIME now{};
    ::GetSystemTimeAsFileTime(&now);
    ULARGE_INTEGER created{}, current{};
    created.LowPart = create.dwLowDateTime;
    created.HighPart = create.dwHighDateTime;
    current.LowPart = now.dwLowDateTime;
    current.HighPart = now.dwHighDateTime;
    const ULONGLONG secs = (current.QuadPart - created.QuadPart) / 10000000ULL;
    snprintf(out, size, "%02llu:%02llu:%02llu",
             secs / 3600, (secs % 3600) / 60, secs % 60);
    return true;
}

// Enumerate modules and write the first few to the log
// 枚举模块并将前几个写入日志
void LogModules(HANDLE handle, LogBuffer& log) {
    if (!handle) return;
    DWORD needed = 0;
    if (!::EnumProcessModulesEx(handle, nullptr, 0, &needed, LIST_MODULES_ALL)) return;
    const int count = static_cast<int>(needed / sizeof(HMODULE));
    std::vector<HMODULE> modules(count);
    if (!::EnumProcessModulesEx(handle, modules.data(), needed, &needed,
                                LIST_MODULES_ALL))
        return;

    std::string list;
    char name[MAX_PATH];
    int shown = 0;
    for (int i = 0; i < count && shown < 5; ++i) {
        if (::GetModuleBaseNameA(handle, modules[i], name, MAX_PATH) == 0) continue;
        if (!list.empty()) list += ", ";
        list += name;
        ++shown;
    }
    if (count > shown) {
        char tail[32];
        snprintf(tail, sizeof(tail), " (+%d)", count - shown);
        list += tail;
    }
    log.Add(LogLevel::Info, "模块列表 (%d): %s", count, list.c_str());
}

} // namespace

// ==============================
// 构造
// ==============================

PvZFeature::PvZFeature(LogBuffer& logger) : m_log(logger) {}

const char* PvZFeature::GetName() const {
    return "Plants vs Zombies";
}

const wchar_t* PvZFeature::GetProcessName() const {
    return L"PlantsVsZombies.exe";
}

void PvZFeature::OnAttach(Memory& mem) {
    (void)mem;
    m_log.Add(LogLevel::Info, "PvZ: 已附加到进程");
}

void PvZFeature::OnDetach() {
    m_log.Add(LogLevel::Info, "PvZ: 已与进程断开");
}

// ==============================
// 每帧更新：数据同步 + 持续写入
// ==============================

void PvZFeature::OnUpdate(Memory& mem) {
    using namespace pvz;

    // 读取当前阳光
    mem.ReadPointerChain(kBaseAddress,
                         {kPtrChainMemMgr, kPtrChainSunshine},
                         m_sunshine);

    // 写入待提交的阳光值（支持写 0）
    if (m_sunshineDirty) {
        WriteSunshine(mem, m_pendingSunshine);
        m_sunshineDirty = false;
    }

    // 持续写入功能（类似 CE 锁定）
    if (m_cdSlot1Enabled)
        mem.WritePointerChain(kBaseAddress,
                              {kPtrChainMemMgr, kPtrChainCDBase, kCDSlot1Offset},
                              uint8_t(1));
    if (m_cdSlot2Enabled)
        mem.WritePointerChain(kBaseAddress,
                              {kPtrChainMemMgr, kPtrChainCDBase,
                               kCDSlot1Offset + kCDSlotStride},
                              uint8_t(1));
    if (m_cdSlot3Enabled)
        mem.WritePointerChain(kBaseAddress,
                              {kPtrChainMemMgr, kPtrChainCDBase,
                               kCDSlot1Offset + kCDSlotStride * 2},
                              uint8_t(1));

    if (m_autoCollectSunshine)
        CollectSunshineRemote(mem);
}

// ==============================
// 阳光写入
// ==============================

bool PvZFeature::WriteSunshine(Memory& mem, int value) {
    using namespace pvz;
    bool ok = mem.WritePointerChain(kBaseAddress,
                                    {kPtrChainMemMgr, kPtrChainSunshine},
                                    value);
    if (ok)
        m_log.Add(LogLevel::Info, "阳光已写入: %d", value);
    else
        m_log.Add(LogLevel::Warning, "阳光写入失败");
    return ok;
}

// ==============================
// 远程线程：自动采集阳光
// ==============================

bool PvZFeature::CollectSunshineRemote(Memory& mem) {
    using namespace pvz;

    uintptr_t addr = mem.ResolvePointerChain(
        kBaseAddress, {kPtrChainMemMgr, kPtrChainCollect});
    if (addr == 0) return false;

    HANDLE hProcess = mem.ProcessHandle();
    if (!hProcess) return false;

    DWORD addr32 = static_cast<DWORD>(addr);

    // 在目标进程中分配参数内存
    LPVOID paramAddr = ::VirtualAllocEx(
        hProcess, nullptr, sizeof(DWORD),
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!paramAddr) return false;

    bool ok = false;
    if (::WriteProcessMemory(hProcess, paramAddr, &addr32,
                             sizeof(DWORD), nullptr)) {
        HANDLE hThread = ::CreateRemoteThread(
            hProcess, nullptr, 0,
            (LPTHREAD_START_ROUTINE)kCollectSunshineFn,
            paramAddr, 0, nullptr);
        if (hThread) {
            ::WaitForSingleObject(hThread, INFINITE);
            ::CloseHandle(hThread);
            ok = true;
        }
    }

    ::VirtualFreeEx(hProcess, paramAddr, 0, MEM_RELEASE);
    return ok;
}

// ==============================
// 宿主桥接
// ==============================

void PvZFeature::OnFrameProcessState(bool attached, DWORD pid, HANDLE handle) {
    m_attached = attached;
    m_pid      = pid;
    m_handle   = handle;
}

GameFeature::UiRequest PvZFeature::ConsumeUiRequest() {
    const UiRequest request = m_uiRequest;
    m_uiRequest = UiRequest::None;
    return request;
}

// ==============================
// UI 渲染 — 6-Tab 框架（docs/menu-framework-pvz.html）
// ==============================

void PvZFeature::OnRenderUI() {
    PushTheme();

    ImGui::SetNextWindowSize(ImVec2(680, 540), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(640, 420), ImVec2(960, 900));
    ImGui::SetNextWindowPos(ImVec2(60, 30), ImGuiCond_FirstUseEver);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse;
    if (!ImGui::Begin("Plants vs Zombies##menu", nullptr, flags)) {
        ImGui::End();
        PopTheme();
        return;
    }

    RenderHeader();
    RenderTabBar();

    ImGui::BeginChild("##body", ImVec2(0, 0));
    RenderSidebar();
    ImGui::SameLine();
    ImGui::BeginChild("##pane", ImVec2(0, 0));
    RenderPane();
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::End();
    PopTheme();
}

// ==============================
// 标题栏
// ==============================

void PvZFeature::RenderHeader() {
    const float avail = ImGui::GetContentRegionAvail().x;

    // 游戏标题 + 版本
    ImGui::TextColored(kTextBright, "Plants vs Zombies");
    ImGui::SameLine();
    ImGui::TextColored(kTextDim, "v1.2.0.1073");

    // 连接状态（右对齐）
    const char* statusText = m_attached ? "已附加" : "未附加";
    const ImVec4& statusColor = m_attached ? kGreen : kRed;
    const float statusWidth = ImGui::CalcTextSize("●").x + 8.0f +
                              ImGui::CalcTextSize(statusText).x;
    ImGui::SameLine(avail - statusWidth);
    ImGui::TextColored(statusColor, "●");
    ImGui::SameLine();
    ImGui::TextColored(statusColor, "%s", statusText);

    ImGui::Spacing();
    ImGui::Separator();
}

// ==============================
// 顶部标签栏（6 Tab + 徽章）
// ==============================

void PvZFeature::RenderTabBar() {
    const float avail = ImGui::GetContentRegionAvail().x;
    const float tabWidth = (avail - 5.0f * ImGui::GetStyle().ItemSpacing.x) / 6.0f;
    const float tabHeight = 32.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (int i = 0; i < 6; ++i) {
        const bool active = (m_uiTab == i);

        // Tab 徽章：已实现 / 总数
        int done = 0, total = 0;
        for (int g = 0; g < kGroupCount[i]; ++g) {
            done  += kGroups[i][g].done;
            total += kGroups[i][g].total;
        }
        char badge[16];
        snprintf(badge, sizeof(badge), "%d/%d", done, total);

        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Button,
                              active ? kAccentBg : kWindowBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kHoverBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, kActiveBg);
        if (ImGui::Button("##tab", ImVec2(tabWidth, tabHeight)))
            m_uiTab = i;
        ImGui::PopStyleColor(3);

        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        const float midY = (min.y + max.y) * 0.5f;

        // 标签文字
        const ImVec2 labelSize = ImGui::CalcTextSize(kTabNames[i]);
        drawList->AddText(ImVec2(min.x + 12, midY - labelSize.y * 0.5f),
                          ToU32(active ? kAccent : kText), kTabNames[i]);

        // 徽章（右对齐）
        const ImVec2 badgeSize = ImGui::CalcTextSize(badge);
        drawList->AddText(ImVec2(max.x - 10 - badgeSize.x,
                                 midY - badgeSize.y * 0.5f),
                          ToU32(kTextDim), badge);

        // 选中下划线
        if (active)
            drawList->AddRectFilled(ImVec2(min.x + 8, max.y - 2),
                                    ImVec2(max.x - 8, max.y),
                                    ToU32(kAccent), 1.0f);

        if (i < 5) ImGui::SameLine();
        ImGui::PopID();
    }

    ImGui::Spacing();
    ImGui::Separator();
}

// ==============================
// 左侧分组导航
// ==============================

void PvZFeature::RenderSidebar() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kSurfaceBg);
    ImGui::BeginChild("##side", ImVec2(158, 0), ImGuiChildFlags_Borders);

    ImGui::Spacing();
    ImGui::TextColored(kTextDim, "%s", kTabNames[m_uiTab]);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const int count = kGroupCount[m_uiTab];
    const float rowHeight = 30.0f;
    const float lineHeight = ImGui::GetTextLineHeight();

    for (int i = 0; i < count; ++i) {
        const GroupDef& group = kGroups[m_uiTab][i];
        const bool active = (m_uiSub[m_uiTab] == i);

        ImGui::PushID(i);
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Header, kAccentBg);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kHoverAccent);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kHoverBg);
        }
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, kActiveBg);

        const float width = ImGui::GetContentRegionAvail().x;
        if (ImGui::Selectable("##group", active, 0, ImVec2(width, rowHeight)))
            m_uiSub[m_uiTab] = i;

        ImGui::PopStyleColor(3);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        const float midY = (min.y + max.y) * 0.5f;

        // 选中左侧强调条
        if (active)
            drawList->AddRectFilled(ImVec2(min.x, min.y),
                                    ImVec2(min.x + 2, max.y),
                                    ToU32(kAccent));

        // 状态圆点：分组内有已实现项时为绿色
        drawList->AddCircleFilled(ImVec2(min.x + 15, midY), 3.5f,
                                  ToU32(group.done > 0 ? kGreen : kTextDim));

        // 分组名
        const float textY = min.y + (rowHeight - lineHeight) * 0.5f;
        drawList->AddText(ImVec2(min.x + 28, textY),
                          ToU32(active ? kAccent : kText), group.name);

        // 已实现 / 总数
        char counter[16];
        snprintf(counter, sizeof(counter), "%d/%d", group.done, group.total);
        const float counterWidth = ImGui::CalcTextSize(counter).x;
        drawList->AddText(ImVec2(max.x - 14 - counterWidth, textY),
                          ToU32(kTextDim), counter);

        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ==============================
// 右侧内容面板
// ==============================

void PvZFeature::RenderPane() {
    switch (m_uiTab) {
        case 0: RenderVisualPane(); break;
        case 1: RenderAssistPane(); break;
        case 2: RenderNumericPane(); break;
        case 3: RenderProcessPane(); break;
        case 4: RenderSettingsPane(); break;
        case 5: RenderDeveloperPane(); break;
        default: break;
    }
}

// --- 视觉 ---

void PvZFeature::RenderVisualPane() {
    static const char* kItems[4][5] = {
        { "方框透视", "骨骼透视", "血量条", "名字标签", "距离" },
        { "物品高亮", "全亮", "线框模式", "去雾" },
        { "自瞄FOV圈", "目标连线", "命中框显示", "弹道轨迹" },
        { "准星", "FPS计数", "水印", "计时器" },
    };
    static const int kCounts[4] = { 5, 4, 4, 4 };

    const GroupDef& group = kGroups[0][m_uiSub[0]];
    PaneHeader(group.name, kTabDescs[0], group.done, group.total);
    RowPlaceholders(kItems[m_uiSub[0]], kCounts[m_uiSub[0]]);
}

// --- 辅助 ---

void PvZFeature::RenderAssistPane() {
    const GroupDef& group = kGroups[1][m_uiSub[1]];
    PaneHeader(group.name, kTabDescs[1], group.done, group.total);

    switch (m_uiSub[1]) {
        case 0: { // 战斗
            if (RowToggle("1格 无CD", true, &m_cdSlot1Enabled))
                m_log.Add(LogLevel::Info, "1格CD %s",
                          m_cdSlot1Enabled ? "启用" : "禁用");
            if (RowToggle("2格 无CD", true, &m_cdSlot2Enabled))
                m_log.Add(LogLevel::Info, "2格CD %s",
                          m_cdSlot2Enabled ? "启用" : "禁用");
            if (RowToggle("3格 无CD", true, &m_cdSlot3Enabled))
                m_log.Add(LogLevel::Info, "3格CD %s",
                          m_cdSlot3Enabled ? "启用" : "禁用");
            static const char* kPlanned[] = { "快速射击", "自动瞄准" };
            RowPlaceholders(kPlanned, 2);
            break;
        }
        case 1: { // 移动
            static const char* kPlanned[] = { "加速", "跳跃", "飞行", "无限冲刺" };
            RowPlaceholders(kPlanned, 4);
            break;
        }
        case 2: { // 自动化
            if (RowToggle("自动采集阳光", true, &m_autoCollectSunshine))
                m_log.Add(LogLevel::Info, "自动采集 %s",
                          m_autoCollectSunshine ? "启用" : "禁用");
            static const char* kPlanned[] = { "自动收集", "自动对话" };
            RowPlaceholders(kPlanned, 2);
            break;
        }
        case 3: { // 无敌
            static const char* kPlanned[] = { "植物无敌", "无限阳光", "一击必杀" };
            RowPlaceholders(kPlanned, 3);
            break;
        }
    }
}

// --- 数值 ---

void PvZFeature::RenderNumericPane() {
    const GroupDef& group = kGroups[2][m_uiSub[2]];
    PaneHeader(group.name, kTabDescs[2], group.done, group.total);

    // 当前阳光值（顶部展示）
    const float avail = ImGui::GetContentRegionAvail().x;
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(kTextDim, "当前阳光值");
    char sun[24];
    snprintf(sun, sizeof(sun), "%d", m_sunshine);
    ImGui::SameLine(avail - ImGui::CalcTextSize(sun).x);
    ImGui::TextColored(kGreen, "%s", sun);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    switch (m_uiSub[2]) {
        case 0: { // 属性
            static const char* kPlanned[] = { "生命值", "攻击力", "防御力", "速度" };
            RowPlaceholders(kPlanned, 4);
            break;
        }
        case 1: { // 资源
            RowBegin("阳光值", true, 196.0f);
            ImGui::SetNextItemWidth(118);
            ImGui::InputInt("##sunIn", &m_pendingSunshine, 10, 100);
            ImGui::SameLine();
            if (ImGui::Button("写入", ImVec2(62, 0))) {
                m_sunshineDirty = true;
                m_log.Add(LogLevel::Info, "目标阳光: %d", m_pendingSunshine);
            }
            ImGui::Spacing();
            static const char* kPlanned[] = { "金币", "钻石" };
            RowPlaceholders(kPlanned, 2);
            break;
        }
        case 2: { // 进度
            static const char* kPlanned[] = { "关卡", "经验值" };
            RowPlaceholders(kPlanned, 2);
            break;
        }
        case 3: { // 武器属性
            static const char* kPlanned[] = { "伤害", "射速", "冷却时间", "射程" };
            RowPlaceholders(kPlanned, 4);
            break;
        }
    }
}

// --- 进程 ---

void PvZFeature::RenderProcessPane() {
    const GroupDef& group = kGroups[3][m_uiSub[3]];
    PaneHeader(group.name, kTabDescs[3], group.done, group.total);

    switch (m_uiSub[3]) {
        case 0: { // 连接
            if (RowButton("附加进程", true, "附加", kGreen, !m_attached)) {
                m_uiRequest = UiRequest::Attach;
                m_log.Add(LogLevel::Info, "请求附加进程...");
            }
            if (RowButton("分离进程", true, "分离", kRed, m_attached)) {
                m_uiRequest = UiRequest::Detach;
                m_log.Add(LogLevel::Info, "请求分离进程...");
            }
            if (RowButton("重连进程", true, "重连", kOrange, m_attached)) {
                m_uiRequest = UiRequest::Reconnect;
                m_log.Add(LogLevel::Info, "请求重连进程...");
            }

            char pid[64];
            if (m_attached)
                snprintf(pid, sizeof(pid), "0x%X (%u)", m_pid, m_pid);
            else
                snprintf(pid, sizeof(pid), "—");
            RowValue("PID", true, pid, kText);

            RowValue("进程名称", true, m_attached ? "PlantsVsZombies.exe" : "—",
                     m_attached ? kGreen : kTextDim);
            break;
        }
        case 1: { // 信息
            char uptime[64];
            if (!FormatUptime(m_handle, uptime, sizeof(uptime)))
                snprintf(uptime, sizeof(uptime), "—");
            RowValue("运行时间", true, uptime, kText);

            if (RowButton("模块列表", true, "查看", kAccent, m_attached))
                LogModules(m_handle, m_log);

            RowValue("架构", true, "x86 / 32-bit", kText);
            break;
        }
        case 2: { // 内存扫描
            RowValue("起始地址", false, "0x00000000", kTextDim);
            RowValue("结束地址", false, "0x7FFFFFFF", kTextDim);
            RowValue("扫描区域", false, "全部", kTextDim);
            break;
        }
    }
}

// --- 设置 ---

void PvZFeature::RenderSettingsPane() {
    const GroupDef& group = kGroups[4][m_uiSub[4]];
    PaneHeader(group.name, kTabDescs[4], group.done, group.total);

    switch (m_uiSub[4]) {
        case 0: { // 快捷键
            RowKeyChip("覆盖层开关", true, "Home");
            RowKeyChip("禁用所有", true, "End");
            RowButton("功能绑定", false, "编辑", kAccent, false);
            break;
        }
        case 1: { // 外观
            RowValue("主题", false, "深色", kTextDim);
            RowValue("透明度", false, "1.00", kTextDim);
            RowValue("字体大小", false, "18", kTextDim);
            RowValue("语言", false, "中文", kTextDim);
            break;
        }
        case 2: { // 通用
            RowToggle("开机启动", false, nullptr);
            RowToggle("管理员模式", false, nullptr);
            RowButton("检查更新", false, "检查", kAccent, false);
            RowButton("保存/加载配置", false, "保存", kAccent, false);
            break;
        }
    }
}

// --- 开发者 ---

void PvZFeature::RenderDeveloperPane() {
    const GroupDef& group = kGroups[5][m_uiSub[5]];
    PaneHeader(group.name, kTabDescs[5], group.done, group.total);

    switch (m_uiSub[5]) {
        case 0: { // 内存
            static const char* kPlanned[] = { "内存查看器", "十六进制编辑", "地址导航" };
            RowPlaceholders(kPlanned, 3);
            break;
        }
        case 1: { // 指针
            RowButton("指针解析", false, "打开", kAccent, false);
            RowButton("偏移计算器", false, "打开", kAccent, false);
            RowButton("偏移配置表", false, "编辑", kAccent, false);
            break;
        }
        case 2: { // 脚本
            RowButton("Lua控制台", false, "打开", kAccent, false);
            RowButton("脚本加载器", false, "打开", kAccent, false);
            RowButton("自动运行脚本", false, "编辑", kAccent, false);
            break;
        }
        case 3: { // 日志
            if (RowButton("清除日志", true, "清除", kRed, true))
                m_log.Clear();
            if (RowButton("保存到文件", true, "保存", kAccent, true)) {
                const bool ok = m_log.SaveToFile("coco_pvz_log.txt");
                m_log.Add(ok ? LogLevel::Info : LogLevel::Warning,
                          ok ? "日志已保存到 coco_pvz_log.txt" : "日志保存失败");
            }
            RowToggle("自动滚动", true, &m_logAutoScroll);
            RowValue("时间戳条目", true, "[HH:MM:SS] 前缀", kTextDim);
            ImGui::Spacing();
            RenderLogView();
            break;
        }
    }
}

// ==============================
// 日志视图
// ==============================

void PvZFeature::RenderLogView() {
    ImGui::BeginChild("##logview", ImVec2(0, 0), ImGuiChildFlags_Borders);
    for (const auto& entry : m_log.Messages()) {
        ImVec4 color = kText;
        switch (entry.level) {
            case LogLevel::Warning: color = kOrange; break;
            case LogLevel::Error:   color = kRed;    break;
            case LogLevel::Debug:   color = kTextDim; break;
            default: break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(entry.text.c_str());
        ImGui::PopStyleColor();
    }
    if (m_logAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() * 0.8f)
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}

} // namespace coco