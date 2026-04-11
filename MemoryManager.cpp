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

bool MemoryManager::ReadSunshine(int& sunshine) {
	if (!IsAttached()) {
		return false;
	}

	DWORD address = GetSunshineAddress();
	if (address == 0) {
		return false;
	}

	return ReadMemory(address, &sunshine, sizeof(int));
}

bool MemoryManager::WriteSunshine(int sunshine) {
	if (!IsAttached()) {
		return false;
	}

	DWORD address = GetSunshineAddress();
	if (address == 0) {
		return false;
	}

	return WriteMemory(address, &sunshine, sizeof(int));
}

bool MemoryManager::WriteCDSlot(int slot, bool enabled) {
	if (!IsAttached()) {
		return false;
	}

	// Calculate CD slot address
	DWORD baseAddress = BASE_ADDRESS;
	DWORD firstAddress;
	DWORD secondAddress;
	DWORD thirdAddress;

	if (!ReadMemory(baseAddress, &firstAddress, sizeof(DWORD))) {
		return false;
	}

	firstAddress += OFFSET_1;  // 768
	if (!ReadMemory(firstAddress, &secondAddress, sizeof(DWORD))) {
		return false;
	}

	secondAddress += 0x144; // 144
	if (!ReadMemory(secondAddress, &thirdAddress, sizeof(DWORD))) {
		return false;
	}
	// Calculate offset based on slot number
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

	DWORD cdAddress = thirdAddress + slotOffset;
	// 返回cdAddress 十六进制显示
	cout << "Base Address: 0x" << hex << baseAddress << dec << endl;
	cout << "First Address: 0x" << hex << firstAddress << dec << endl;
	cout << "Second Address: 0x" << hex << secondAddress << dec << endl;
	cout << "Third Address: 0x" << hex << thirdAddress << dec << endl;
	cout << "Third Address: 0x" << hex << cdAddress << dec << endl;
	//login("CD Slot %d Address: 0x%X\n", slot, cdAddress);
	BYTE value = enabled ? 1 : 0;
	return WriteMemory(cdAddress, &value, sizeof(BYTE));
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
	return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead) && (bytesRead == size);
}

bool MemoryManager::WriteMemory(DWORD address, const void* buffer, SIZE_T size) {
	SIZE_T bytesWritten;
	return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &bytesWritten) && (bytesWritten == size);
}

DWORD MemoryManager::GetSunshineAddress() {
	DWORD baseAddress = BASE_ADDRESS;
	DWORD firstAddress;
	DWORD secondAddress;

	if (!ReadMemory(baseAddress, &firstAddress, sizeof(DWORD))) {
		return 0;
	}

	firstAddress += OFFSET_1;
	if (!ReadMemory(firstAddress, &secondAddress, sizeof(DWORD))) {
		return 0;
	}

	secondAddress += OFFSET_2;
	return secondAddress;
}