import { contextBridge, ipcRenderer } from 'electron'

contextBridge.exposeInMainWorld('electronAPI', {
  // Registry
  readAssetRegistry: () => ipcRenderer.invoke('registry:read'),
  writeAssetRegistry: (data: unknown) => ipcRenderer.invoke('registry:write', data),

  // File system
  readDirectory: (path: string) => ipcRenderer.invoke('fs:readDirectory', path),
  readFileBase64: (path: string) => ipcRenderer.invoke('fs:readFileBase64', path),
  readFileText: (path: string) => ipcRenderer.invoke('fs:readFileText', path),
  createDirectory: (relativePath: string) => ipcRenderer.invoke('fs:createDirectory', relativePath),

  // Asset operations
  importAsset: (sourcePath: string, type: string) => ipcRenderer.invoke('asset:import', sourcePath, type),
  registerAsset: (relativePath: string, type: string) => ipcRenderer.invoke('asset:register', relativePath, type),
  deleteAsset: (guid: string) => ipcRenderer.invoke('asset:delete', guid),
  updateAlias: (guid: string, alias: string | null) => ipcRenderer.invoke('asset:updateAlias', guid, alias),
  importExternal: () => ipcRenderer.invoke('asset:importExternal'),
  importFile: (filePath: string) => ipcRenderer.invoke('asset:importFile', filePath),
  moveAsset: (sourceRelPath: string, targetDirRelPath: string) => ipcRenderer.invoke('asset:move', sourceRelPath, targetDirRelPath),

  // Dialog
  openFileDialog: (type: string) => ipcRenderer.invoke('dialog:openFile', type),

  // Game launcher (Phase 3)
  launchGame: () => ipcRenderer.invoke('game:launch'),
  stopGame: () => ipcRenderer.invoke('game:stop')
})
