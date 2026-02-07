import { contextBridge, ipcRenderer } from 'electron';

contextBridge.exposeInMainWorld('windowControls', {

  minimize: () => ipcRenderer.send('window-minimize'),

  maximize: () => ipcRenderer.send('window-maximize'),

  close: () => ipcRenderer.send('window-close'),

  sendScript: (script: string) => ipcRenderer.send('send-script', script),

});
