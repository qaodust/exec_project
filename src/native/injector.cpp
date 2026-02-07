#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>

// Helper to find Process ID by name
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

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: injector.exe <ProcessName> <DllPath>" << std::endl;
        return 1;
    }

    // Convert arguments
    std::string processNameStr = argv[1];
    std::wstring processName(processNameStr.begin(), processNameStr.end());
    std::string dllPath = argv[2];

    std::cout << "Target Process: " << processNameStr << std::endl;
    std::cout << "DLL Path: " << dllPath << std::endl;

    DWORD pid = GetProcessIdByName(processName);
    if (pid == 0) {
        std::cerr << "Error: Process not found." << std::endl;
        return 1;
    }

    std::cout << "Found Process ID: " << pid << std::endl;

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

    // Cleanup handles (Memory remains allocated until process exit, or we could free it after LoadLibrary returns)
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return 0;
}
