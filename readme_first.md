# How to Get the Windows Executable (.exe)

Since this project is being developed on a Linux system, the Windows build process is automated using **GitHub Actions**. Follow these steps to download the latest `.exe` installer:

### 1. Go to the GitHub Actions Page
Navigate to your repository on GitHub:
[https://github.com/qaodust/exec_project/actions](https://github.com/qaodust/exec_project/actions)

### 2. Select the Latest Build
- Look for the most recent workflow run (usually named "Add GitHub Action for Windows Build" or "Build Windows Executable").
- If a build is currently in progress (yellow spinning icon), wait about 3-5 minutes for it to finish (green checkmark).

### 3. Download the Artifact
- Click on the name of the workflow run to open its details.
- Scroll down to the bottom of the page to the **Artifacts** section.
- Click on the link named **`exec_project-windows-build`**.
- This will download a `.zip` file to your computer.

### 4. Install the Application
- Extract the downloaded `.zip` file.
- Inside, you will find a folder (likely `squirrel.windows`).
- Run the `.exe` file (e.g., `exec_project-1.0.0 Setup.exe`) to install the executor UI on your Windows 11 machine.

---
**Note:** To trigger a new build, simply push any changes to the `main` branch, or click "Run workflow" manually from the Actions tab.
