import { contextBridge, ipcRenderer } from 'electron';

contextBridge.exposeInMainWorld('windowControls', {

  minimize: () => ipcRenderer.send('window-minimize'),

  maximize: () => ipcRenderer.send('window-maximize'),

  close: () => ipcRenderer.send('window-close'),

  sendScript: (script: string) => ipcRenderer.send('send-script', script),

  inject: () => ipcRenderer.send('inject'),

  onLog: (callback: (text: string, type: string) => void) => 
    ipcRenderer.on('log-message', (_event, text, type) => callback(text, type)),

});
