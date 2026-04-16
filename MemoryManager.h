#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <type_traits>

class MemoryManager {
private:
    HANDLE m_processHandle = nullptr;
    DWORD m_processId = 0;
    const DWORD kBaseAddress = 0x006A9EC0;
    const DWORD kOffset1 = 0x768;
    const DWORD kOffset2 = 0x5560;

public:
    ~MemoryManager();
    bool AttachProcess(const std::wstring& processName);
    void DetachProcess();
    bool ReadSunshine(int& sunshine);
    bool WriteSunshine(int sunshine);
    bool IsAttached() const;
    bool WriteCDSlot(int slot, bool enabled);
    bool CollectSunshine();

    // ==================== 通用指针链读取方法 ====================
    
    // 计算指针链最终地址
    // baseAddress: 基地址
    // offsets: 偏移量数组，例如 {0x768, 0x5560} 表示 [[base] + 0x768] + 0x5560
    // 返回: 最终地址，失败返回 0
    uintptr_t ResolvePointerChain(uintptr_t baseAddress, const std::vector<DWORD>& offsets);

    // 读取指针链最终地址的值（模板方法，支持任意类型）
    // baseAddress: 基地址
    // offsets: 偏移量数组
    // outValue: 输出值
    // 返回: 是否成功
    template<typename T>
    bool ReadPointerChainValue(uintptr_t baseAddress, const std::vector<DWORD>& offsets, T& outValue) {
        if (!IsAttached()) {
            return false;
        }

        uintptr_t finalAddress = ResolvePointerChain(baseAddress, offsets);
        if (finalAddress == 0) {
            return false;
        }

        return ReadValue<T>(finalAddress, outValue);
    }

    // 写入值到指定地址（模板方法，支持任意类型）
    // address: 目标地址
    // value: 要写入的值
    // 返回: 是否成功
    template<typename T>
    bool WriteValue(uintptr_t address, const T& value) {
        if (!IsAttached()) {
            return false;
        }

        if (address == 0) {
            return false;
        }

        return WriteMemory(address, &value, sizeof(T));
    }

    // 写入值到指针链最终地址（模板方法，支持任意类型）
    // baseAddress: 基地址
    // offsets: 偏移量数组
    // value: 要写入的值
    // 返回: 是否成功
    template<typename T>
    bool WritePointerChainValue(uintptr_t baseAddress, const std::vector<DWORD>& offsets, const T& value) {
        if (!IsAttached()) {
            return false;
        }

        uintptr_t finalAddress = ResolvePointerChain(baseAddress, offsets);
        if (finalAddress == 0) {
            return false;
        }

        return WriteValue<T>(finalAddress, value);
    }

    // 读取指定地址的值（模板方法，支持任意类型）
    // address: 目标地址
    // outValue: 输出值
    // 返回: 是否成功
    template<typename T>
    bool ReadValue(uintptr_t address, T& outValue) {
        if (!IsAttached()) {
            return false;
        }

        if (address == 0) {
            return false;
        }

        return ReadMemory(address, &outValue, sizeof(T));
    }

private:
    DWORD GetProcessIdByName(const std::wstring& processName);
    bool ReadMemory(uintptr_t address, void* buffer, SIZE_T size);
    bool WriteMemory(uintptr_t address, const void* buffer, SIZE_T size);
};
