#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>

// List of known Roblox process names
const std::vector<std::wstring> ROBLOX_PROCESSES = {
    L"RobloxPlayerBeta.exe",
    L"Windows10Universal.exe",
    L"RobloxPlayer.exe"
};

// Helper to find Process ID by name (exact match)
DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (std::wstring(entry.szExeFile) == processName) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

// Helper to find any active Roblox process from the known list
DWORD FindAnyRobloxProcess(std::wstring& foundName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (Process32FirstW(snapshot, &entry)) {
            do {
                std::wstring currentName(entry.szExeFile);
                for (const auto& target : ROBLOX_PROCESSES) {
                    if (currentName == target) {
                        pid = entry.th32ProcessID;
                        foundName = currentName;
                        break;
                    }
                }
                if (pid != 0) break;
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: injector.exe <ProcessName> <DllPath>" << std::endl;
        return 1;
    }

    // Convert arguments
    std::string dllPath = argv[2];
    std::wstring processName;

    std::cout << "Searching for Roblox process..." << std::endl;

    // 1. Try to find any known Roblox process automatically
    DWORD pid = FindAnyRobloxProcess(processName);

    // 2. If not found, try the argument provided (as fallback)
    if (pid == 0) {
        std::string argNameStr = argv[1];
        std::wstring argName(argNameStr.begin(), argNameStr.end());
        std::cout << "Auto-detection failed. Trying argument: " << argNameStr << std::endl;
        pid = GetProcessIdByName(argName);
        if (pid != 0) {
            processName = argName;
        }
    }

    if (pid == 0) {
        std::cerr << "Error: No Roblox process found." << std::endl;
        return 1;
    }

    std::wcout << L"Target Process: " << processName << L" (PID: " << pid << L")" << std::endl;
    std::cout << "DLL Path: " << dllPath << std::endl;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Error: Could not open process." << std::endl;
        return 1;
    }

    // Allocate memory in target process for the DLL path
    LPVOID pRemotePath = VirtualAllocEx(hProcess, NULL, dllPath.length() + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemotePath) {
        std::cerr << "Error: Could not allocate memory in target." << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, pRemotePath, dllPath.c_str(), dllPath.length() + 1, NULL)) {
        std::cerr << "Error: Could not write memory to target." << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Create a remote thread to load the DLL
    // Kernel32.dll is loaded at the same address in all processes (usually), so we can use GetProcAddress locally.
    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemotePath, 0, NULL);
    if (!hThread) {
        std::cerr << "Error: Could not create remote thread." << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);
    
    std::cout << "Injection Successful!" << std::endl;

    // Cleanup handles
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return 0;
}