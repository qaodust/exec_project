import { app, BrowserWindow, ipcMain } from 'electron';
import path from 'node:path';
import started from 'electron-squirrel-startup';
import net from 'node:net';
import { execFile } from 'node:child_process';

// Handle creating/removing shortcuts on Windows when installing/uninstalling.
if (started) {
  app.quit();
}

const PIPE_NAME = '\\\\.\\pipe\\exec_project_pipe';

const createWindow = () => {
  // Create the browser window.
  const mainWindow = new BrowserWindow({
    width: 1000,
    height: 600,
    frame: false, // Frameless window
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
    },
  });

  // Handle window controls
  ipcMain.on('window-minimize', () => {
    mainWindow.minimize();
  });

  ipcMain.on('window-maximize', () => {
    if (mainWindow.isMaximized()) {
      mainWindow.unmaximize();
    } else {
      mainWindow.maximize();
    }
  });

  ipcMain.on('window-close', () => {
    mainWindow.close();
  });

  ipcMain.on('inject', (event) => {
    let injectorPath: string;
    let dllPath: string;

    if (app.isPackaged) {
      injectorPath = path.join(process.resourcesPath, 'injector.exe');
      dllPath = path.join(process.resourcesPath, 'backend.dll');
    } else {
       injectorPath = path.join(__dirname, '../../src/native/injector.exe');
       dllPath = path.join(__dirname, '../../src/native/backend.dll');
    }

    const targetProcess = 'RobloxPlayerBeta.exe'; // Target process
    console.log(`Injecting ${dllPath} into ${targetProcess} using ${injectorPath}`);
    
    event.sender.send('log-message', `Attempting to inject...`, 'info');

    execFile(injectorPath, [targetProcess, dllPath], (error, stdout, stderr) => {
      if (error) {
        console.error('Injection failed:', error);
        event.sender.send('log-message', `Injection Failed: ${error.message} (Is Roblox open?)`, 'error');
        return;
      }
      
      console.log('Injector Output:', stdout);
      event.sender.send('log-message', `Injector: ${stdout}`, 'success');
    });
  });

  ipcMain.on('send-script', (event, script) => {
    const client = net.createConnection(PIPE_NAME, () => {
      console.log('Connected to pipe!');
      client.write(script);
      client.end();
      event.sender.send('log-message', 'Script sent successfully.', 'success');
    });

    client.on('error', (err) => {
      console.error('Pipe connection error:', err.message);
      event.sender.send('log-message', `Connection Error: ${err.message} (Is DLL injected?)`, 'error');
    });
  });

  // and load the index.html of the app.
  if (MAIN_WINDOW_VITE_DEV_SERVER_URL) {
    mainWindow.loadURL(MAIN_WINDOW_VITE_DEV_SERVER_URL);
  } else {
    mainWindow.loadFile(
      path.join(__dirname, `../renderer/${MAIN_WINDOW_VITE_NAME}/index.html`),
    );
  }

  // Open the DevTools.
  mainWindow.webContents.openDevTools();
};

app.on('ready', createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});