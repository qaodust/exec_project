#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

// Define the Pipe Name
#define PIPE_NAME "\.\pipe\exec_project_pipe"

void ConsoleSetup() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
    std::cout << "[INFO] Console Attached Successfully!" << std::endl;
}

DWORD WINAPI PipeThread(LPVOID lpParam) {
    ConsoleSetup();
    std::cout << "[INFO] Waiting for Pipe Connection..." << std::endl;

    HANDLE hPipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        1024 * 16,
        1024 * 16,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERROR] Failed to create pipe: " << GetLastError() << std::endl;
        return 1;
    }

    // Wait for the client (Electron) to connect
    // Note: In a real scenario, we might want a loop to accept multiple connections/reconnections
    // or keep the pipe open.
    
    while (true) {
        std::cout << "[INFO] Pipe Server Listening on " << PIPE_NAME << std::endl;
        
        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            std::cout << "[INFO] Client Connected!" << std::endl;
            
            char buffer[1024];
            DWORD bytesRead;

            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                buffer[bytesRead] = '\0';
                std::cout << "[EXECUTE] " << buffer << std::endl;
                
                // HERE IS WHERE YOU WOULD PASS 'buffer' TO THE LUA ENGINE
                // e.g. run_script(buffer);
            }
            
            std::cout << "[INFO] Client Disconnected." << std::endl;
            DisconnectNamedPipe(hPipe);
        } else {
             std::cerr << "[ERROR] Pipe Connection Failed." << std::endl;
             DisconnectNamedPipe(hPipe);
        }
    }

    CloseHandle(hPipe);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, PipeThread, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
