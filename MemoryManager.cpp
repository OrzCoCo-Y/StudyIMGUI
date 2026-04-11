#include "MemoryManager.h"
#include <tlhelp32.h>
#include <iostream>

using namespace std;

MemoryManager::~MemoryManager() {
	DetachProcess();
}

bool MemoryManager::AttachProcess(const std::wstring& processName) {
	processId = GetProcessIdByName(processName);
	if (processId == 0) {
		return false;
	}

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	return hProcess != NULL;
}

void MemoryManager::DetachProcess() {
	if (hProcess != NULL) {
		CloseHandle(hProcess);
		hProcess = NULL;
	}
	processId = 0;
}

// ==================== 通用指针链读取方法实现 ====================

DWORD MemoryManager::ResolvePointerChain(DWORD baseAddress, const std::vector<DWORD>& offsets) {
	if (!IsAttached()) {
		return 0;
	}

	if (baseAddress == 0) {
		return 0;
	}

	DWORD currentAddress = baseAddress;
	DWORD currentValue = 0;

	// 读取基地址的值
	if (!ReadMemory(currentAddress, &currentValue, sizeof(DWORD))) {
		return 0;
	}

	// 遍历偏移链
	for (size_t i = 0; i < offsets.size(); ++i) {
		currentAddress = currentValue + offsets[i];
		
		// 如果是最后一个偏移，直接返回地址
		if (i == offsets.size() - 1) {
			return currentAddress;
		}

		// 否则继续读取下一级指针
		if (!ReadMemory(currentAddress, &currentValue, sizeof(DWORD))) {
			return 0;
		}

		// 检查空指针
		if (currentValue == 0) {
			return 0;
		}
	}

	// 如果没有偏移，返回基地址读取的值
	return currentValue;
}

bool MemoryManager::ReadSunshine(int& sunshine) {
	if (!IsAttached()) {
		return false;
	}

	// 使用新的指针链读取方法
	// 阳光地址: [[006A9EC0] + 768] + 5560
	return ReadPointerChainValue<int>(BASE_ADDRESS, {OFFSET_1, OFFSET_2}, sunshine);
}

bool MemoryManager::WriteSunshine(int sunshine) {
	if (!IsAttached()) {
		return false;
	}

	// 使用新的指针链写入方法
	return WritePointerChainValue<int>(BASE_ADDRESS, {OFFSET_1, OFFSET_2}, sunshine);
}

bool MemoryManager::WriteCDSlot(int slot, bool enabled) {
	if (!IsAttached()) {
		return false;
	}

	// CD格地址: [[[006A9EC0] + 768] + 144] + slotOffset
	DWORD slotOffset = 0;
	switch (slot) {
	case 1:
		slotOffset = 0x70;
		break;
	case 2:
		slotOffset = 0x70 + 0x50;
		break;
	case 3:
		slotOffset = 0x70 + 0x50 + 0x50;
		break;
	default:
		return false;
	}

	BYTE value = enabled ? 1 : 0;
	return WritePointerChainValue<BYTE>(BASE_ADDRESS, {OFFSET_1, 0x144, slotOffset}, value);
}

bool MemoryManager::CollectSunshine() {
	if (!IsAttached()) {
		return false;
	}

	// 自动采集阳光地址: [[006A9EC0] + 768] + E4
	DWORD targetAddress = ResolvePointerChain(BASE_ADDRESS, {OFFSET_1, 0xE4});
	if (targetAddress == 0) {
		return false;
	}
	
	// Allocate memory in the target process for the parameter
	LPVOID paramAddress = VirtualAllocEx(hProcess, NULL, sizeof(DWORD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (paramAddress == NULL) {
		return false;
	}

	// Write the parameter value to the allocated memory
	if (!WriteProcessMemory(hProcess, paramAddress, &targetAddress, sizeof(DWORD), NULL)) {
		VirtualFreeEx(hProcess, paramAddress, 0, MEM_RELEASE);
		return false;
	}

	// Create a remote thread to call the function at 0x004309D0
	HANDLE hThread = CreateRemoteThread(
		hProcess,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)0x004309D0,
		paramAddress,
		0,
		NULL
	);

	if (hThread == NULL) {
		VirtualFreeEx(hProcess, paramAddress, 0, MEM_RELEASE);
		return false;
	}

	// Wait for the thread to finish
	WaitForSingleObject(hThread, INFINITE);
	
	// Clean up
	CloseHandle(hThread);
	VirtualFreeEx(hProcess, paramAddress, 0, MEM_RELEASE);
	
	return true;
}

bool MemoryManager::IsAttached() const {
	return hProcess != NULL;
}

DWORD MemoryManager::GetProcessIdByName(const std::wstring& processName) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}

	if (Process32First(snapshot, &entry)) {
		do {
			if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return 0;
}

bool MemoryManager::ReadMemory(DWORD address, void* buffer, SIZE_T size) {
    SIZE_T bytesRead;
    return ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer, size, &bytesRead) && (bytesRead == size);
}

bool MemoryManager::WriteMemory(DWORD address, const void* buffer, SIZE_T size) {
    SIZE_T bytesWritten;
    return WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), buffer, size, &bytesWritten) && (bytesWritten == size);
}

DWORD MemoryManager::GetSunshineAddress() {
	return ResolvePointerChain(BASE_ADDRESS, {OFFSET_1, OFFSET_2});
}
