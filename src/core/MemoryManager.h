#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <type_traits>

// ==============================
// 内存管理器
// 封装目标游戏进程的附加、分离与内存读写操作。
// 提供指针链解析、泛型模板读写等通用接口，
// 以及阳光、CD 格、自动采集等专用功能方法。
// ==============================
class MemoryManager {
private:
    HANDLE m_processHandle = nullptr;  // 目标进程句柄（OpenProcess 返回）
    DWORD m_processId = 0;             // 目标进程 PID

    // --- 阳光值指针链常量 ---
    // 阳光地址: [[006A9EC0] + 0x768] + 0x5560
    const DWORD kBaseAddress = 0x006A9EC0;  // 一级基地址
    const DWORD kOffset1 = 0x768;            // 一级偏移
    const DWORD kOffset2 = 0x5560;           // 二级偏移（阳光最终值）

public:
    ~MemoryManager();

    // --- 进程生命周期 ---

    // 按进程名查找并附加目标进程
    bool AttachProcess(const std::wstring& processName);
    // 关闭进程句柄，重置状态
    void DetachProcess();
    // 当前是否已附加到目标进程
    bool IsAttached() const;

    // --- 专用功能：阳光修改 ---

    // 从游戏内存读取当前阳光值
    bool ReadSunshine(int& sunshine);
    // 向游戏内存写入指定阳光值
    bool WriteSunshine(int sunshine);

    // --- 专用功能：CD 格修改 ---

    // 写入指定格位的 CD 状态（slot: 1/2/3）
    bool WriteCDSlot(int slot, bool enabled);

    // --- 专用功能：自动采集阳光 ---

    // 通过远程线程调用游戏内部函数采集阳光
    bool CollectSunshine();

    // ==================== 通用指针链操作方法 ====================

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
    // 遍历系统进程快照，按名称查找目标进程 PID
    DWORD GetProcessIdByName(const std::wstring& processName);
    // 从目标进程的指定地址读取内存
    bool ReadMemory(uintptr_t address, void* buffer, SIZE_T size);
    // 向目标进程的指定地址写入内存
    bool WriteMemory(uintptr_t address, const void* buffer, SIZE_T size);
};
