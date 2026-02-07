# PROJECT_CONTEXT.md: exec_project (Roblox Script Executor)

## Project Overview
**exec_project** is a modern, high-performance Roblox Script Executor. It provides a professional-grade User Interface (UI) for managing, editing, and executing Lua scripts within the Roblox game environment.

## Technical Architecture
The project follows a multi-layered architecture to bridge a high-level UI with low-level process manipulation:

1.  **Frontend (UI Layer):** 
    *   **Framework:** React (TypeScript) + Vite.
    *   **Editor:** Monaco Editor (Lua integration).
    *   **Features:** Script discovery (ScriptBlox API), file management, and a real-time status console.
2.  **Middle-End (Electron Layer):**
    *   **Bridge:** Electron Main process handles IPC between the UI and the OS.
    *   **Communication:** Uses `node:net` to communicate with the C++ backend via Named Pipes (`\.\pipe\exec_project_pipe`).
    *   **Orchestration:** Manages the lifecycle of the injector and paths for bundled native resources.
3.  **Native Backend (C++ Layer):**
    *   **Injector (`injector.exe`):** A C++ utility using `CreateRemoteThread` and `LoadLibraryA` to inject a DLL into `RobloxPlayerBeta.exe`.
    *   **Backend DLL (`backend.dll`):** The code that runs *inside* the target process. It creates a Named Pipe server and attaches a console for script output.

## Build & Deployment
*   **System:** GitHub Actions (`windows-latest`).
*   **Compilation:** Native binaries are compiled using `g++` (MinGW) during the CI/CD process.
*   **Packaging:** Electron Forge bundles the UI and packs the native binaries as `extraResources`.

## Current State (as of Feb 7, 2026)
*   **Status:** **Functional Prototype.**
*   **Completed:** 
    *   Full UI/UX with Sidebar and Console.
    *   Monaco Editor integration.
    *   ScriptBlox API search and execution.
    *   Named Pipe communication bridge.
    *   C++ Injector and Pipe Server DLL.
    *   Automated Windows CI/CD build pipeline.
*   **In Progress / Future Scope:**
    *   Advanced Lua VM integration (The DLL currently logs scripts to a console rather than executing them in a Lua environment).
    *   Anti-Cheat bypass mechanisms.
    *   UI Custom Themes and Preferences.

## Critical Paths
*   **Native Source:** `src/native/`
*   **Main Entry:** `src/main.ts`
*   **UI Entry:** `src/App.tsx`
*   **Pipe Name:** `\.\pipe\exec_project_pipe`
*   **Target Process:** `RobloxPlayerBeta.exe`
