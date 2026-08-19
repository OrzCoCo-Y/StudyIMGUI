#include "PlantCallDll.h"

#include <vector>

namespace coco {
namespace {

using LoadLibraryWFn = HMODULE(WINAPI*)(LPCWSTR);

bool InjectDllLoadLibrary(HANDLE process, const std::wstring& dllPath) {
    if (!process || dllPath.empty()) return false;

    const size_t bytes = (dllPath.size() + 1) * sizeof(wchar_t);
    LPVOID remotePath = ::VirtualAllocEx(
        process, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remotePath) return false;

    bool ok = false;
    if (::WriteProcessMemory(process, remotePath, dllPath.c_str(), bytes, nullptr)) {
        HMODULE kernel32 = ::GetModuleHandleW(L"kernel32.dll");
        auto loadLib = reinterpret_cast<LoadLibraryWFn>(
            ::GetProcAddress(kernel32, "LoadLibraryW"));
        if (loadLib) {
            HANDLE thread = ::CreateRemoteThread(
                process, nullptr, 0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLib),
                remotePath, 0, nullptr);
            if (thread) {
                ::WaitForSingleObject(thread, 10000);
                DWORD exitCode = 0;
                ::GetExitCodeThread(thread, &exitCode);
                ::CloseHandle(thread);
                ok = (exitCode != 0);
            }
        }
    }

    ::VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
    return ok;
}

std::wstring ModuleDirectory() {
    wchar_t path[MAX_PATH]{};
    ::GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring full(path);
    const size_t slash = full.find_last_of(L"\\/");
    if (slash != std::wstring::npos) {
        full.resize(slash + 1);
    }
    return full;
}

} // namespace

std::wstring PlantCallDllClient::DefaultDllPath() {
    return ModuleDirectory() + L"CoCoPvZMod.dll";
}

PlantCallDllClient::~PlantCallDllClient() {
    Detach();
}

void PlantCallDllClient::Detach() {
    if (m_shared) {
        m_shared->command = pvz::PlantCommand::Shutdown;
        ::Sleep(100);
        ::UnmapViewOfFile(m_shared);
        m_shared = nullptr;
    }
    if (m_mapFile) {
        ::CloseHandle(m_mapFile);
        m_mapFile = nullptr;
    }
    m_injected = false;
    m_process  = nullptr;
}

bool PlantCallDllClient::OpenSharedMemory(DWORD timeoutMs) {
    const DWORD step = 50;
    for (DWORD waited = 0; waited < timeoutMs; waited += step) {
        m_mapFile = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE,
                                       pvz::kSharedMemName);
        if (m_mapFile) {
            m_shared = static_cast<pvz::SharedPlantBlock*>(::MapViewOfFile(
                m_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(pvz::SharedPlantBlock)));
            if (m_shared) return true;
            ::CloseHandle(m_mapFile);
            m_mapFile = nullptr;
        }
        ::Sleep(step);
    }
    return false;
}

bool PlantCallDllClient::Inject(HANDLE process, const std::wstring& dllPath) {
    Detach();
    if (!process) return false;

    if (!InjectDllLoadLibrary(process, dllPath)) return false;

    m_process  = process;
    m_injected = true;

    if (!OpenSharedMemory(3000)) {
        Detach();
        return false;
    }
    return true;
}

bool PlantCallDllClient::WaitForCommandClear(DWORD timeoutMs) {
    if (!m_shared) return false;

    const DWORD step = 10;
    for (DWORD waited = 0; waited < timeoutMs; waited += step) {
        if (m_shared->command == pvz::PlantCommand::None) return true;
        ::Sleep(step);
    }
    return false;
}

bool PlantCallDllClient::Plant(uint32_t col, uint32_t row, uint32_t plantType,
                               DWORD timeoutMs) {
    if (!m_shared || !m_injected) return false;
    if (!WaitForCommandClear(500)) return false;

    m_shared->col       = col;
    m_shared->row       = row;
    m_shared->plantType = plantType;
    m_shared->result    = 0;
    m_shared->command   = pvz::PlantCommand::Plant;

    const DWORD step = 10;
    for (DWORD waited = 0; waited < timeoutMs; waited += step) {
        if (m_shared->command == pvz::PlantCommand::None) {
            return m_shared->result != 0;
        }
        ::Sleep(step);
    }
    return false;
}

} // namespace coco
