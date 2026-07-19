#include "PvZFeature.h"
#include "PvZOffsets.h"

#include "core/Memory.h"
#include "core/Core.h"

#include "imgui.h"

namespace coco {

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

    // 如果有待写入的阳光值，写入并清空
    if (m_pendingSunshine != 0 && m_pendingSunshine != m_sunshine) {
        WriteSunshine(mem, m_pendingSunshine);
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
// UI 渲染
// ==============================

void PvZFeature::OnRenderUI() {
    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50, 30), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(GetName(), nullptr,
                      ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("PvZTabs")) {
        if (ImGui::BeginTabItem("概览")) {
            RenderOverviewTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("修改")) {
            RenderSunshineControls();
            ImGui::Separator();
            RenderFeatureToggles();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("日志")) {
            RenderLogPanel();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void PvZFeature::RenderOverviewTab() {
    ImGui::Text("游戏: Plants vs Zombies (1.2.0.1073)");
    ImGui::Separator();
    ImGui::Text("当前阳光值: %d", m_sunshine);
    ImGui::Separator();
    ImGui::Text("功能状态:");
    ImGui::TextColored(m_cdSlot1Enabled ? ImVec4(0,1,0,1) : ImVec4(0.5f,0.5f,0.5f,1),
                       "  1格CD: %s", m_cdSlot1Enabled ? "锁定 (无CD)" : "未启用");
    ImGui::TextColored(m_cdSlot2Enabled ? ImVec4(0,1,0,1) : ImVec4(0.5f,0.5f,0.5f,1),
                       "  2格CD: %s", m_cdSlot2Enabled ? "锁定 (无CD)" : "未启用");
    ImGui::TextColored(m_cdSlot3Enabled ? ImVec4(0,1,0,1) : ImVec4(0.5f,0.5f,0.5f,1),
                       "  3格CD: %s", m_cdSlot3Enabled ? "锁定 (无CD)" : "未启用");
    ImGui::TextColored(m_autoCollectSunshine ? ImVec4(0,1,0,1) : ImVec4(0.5f,0.5f,0.5f,1),
                       "  自动采集: %s", m_autoCollectSunshine ? "已启用" : "已禁用");
}

void PvZFeature::RenderSunshineControls() {
    ImGui::Text("当前读取值: %d", m_sunshine);
    ImGui::InputInt("目标阳光值", &m_pendingSunshine, 10, 100);
    if (ImGui::Button("写入阳光")) {
        // 值会在下一帧 OnUpdate 中被写入
    }
    ImGui::SameLine();
    if (ImGui::Button("读取当前值")) {
        // 值已在 OnUpdate 中持续读取，无需额外操作
    }
}

void PvZFeature::RenderFeatureToggles() {
    ImGui::Text("CD 锁定 (每帧持续写入):");

    if (ImGui::Checkbox("1格 无CD", &m_cdSlot1Enabled))
        m_log.Add(LogLevel::Info, "1格CD %s",
                  m_cdSlot1Enabled ? "启用" : "禁用");
    if (ImGui::Checkbox("2格 无CD", &m_cdSlot2Enabled))
        m_log.Add(LogLevel::Info, "2格CD %s",
                  m_cdSlot2Enabled ? "启用" : "禁用");
    if (ImGui::Checkbox("3格 无CD", &m_cdSlot3Enabled))
        m_log.Add(LogLevel::Info, "3格CD %s",
                  m_cdSlot3Enabled ? "启用" : "禁用");

    ImGui::Separator();
    if (ImGui::Checkbox("自动采集阳光", &m_autoCollectSunshine))
        m_log.Add(LogLevel::Info, "自动采集 %s",
                  m_autoCollectSunshine ? "启用" : "禁用");
}

void PvZFeature::RenderLogPanel() {
    m_log.Draw("PvZLog");
}

} // namespace coco
