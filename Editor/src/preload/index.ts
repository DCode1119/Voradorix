import { contextBridge, ipcRenderer } from 'electron'

contextBridge.exposeInMainWorld('electronAPI', {
  // Registry
  readAssetRegistry: () => ipcRenderer.invoke('registry:read'),
  writeAssetRegistry: (data: unknown) => ipcRenderer.invoke('registry:write', data),

  // File system
  readDirectory: (path: string) => ipcRenderer.invoke('fs:readDirectory', path),
  readFileBase64: (path: string) => ipcRenderer.invoke('fs:readFileBase64', path),
  readFileText: (path: string) => ipcRenderer.invoke('fs:readFileText', path),
  writeFileText: (path: string, content: string) => ipcRenderer.invoke('fs:writeFileText', path, content),
  createDirectory: (relativePath: string) => ipcRenderer.invoke('fs:createDirectory', relativePath),

  // Import helpers
  pickImportSources: () => ipcRenderer.invoke('asset:pickImportSources'),
  previewImport: (sources: Array<{ sourcePath: string; sourceIsDirectory: boolean }>, targetDirRelPath: string) => ipcRenderer.invoke('asset:previewImport', sources, targetDirRelPath),
  executeImport: (sources: Array<{ sourcePath: string; sourceIsDirectory: boolean }>, targetDirRelPath: string, overwriteTargets: string[]) => ipcRenderer.invoke('asset:executeImport', sources, targetDirRelPath, overwriteTargets),

  // Asset operations
  importAsset: (sourcePath: string, type: string) => ipcRenderer.invoke('asset:import', sourcePath, type),
  registerAsset: (relativePath: string, type: string) => ipcRenderer.invoke('asset:register', relativePath, type),
  createScriptAsset: (targetDirRelPath: string, fileName: string) => ipcRenderer.invoke('asset:createScript', targetDirRelPath, fileName),
  renameAssetNode: (sourceRelPath: string, newName: string, isDirectory: boolean) => ipcRenderer.invoke('asset:renameNode', sourceRelPath, newName, isDirectory),
  deleteAsset: (guid: string) => ipcRenderer.invoke('asset:delete', guid),
  deleteNode: (relativePath: string, isDirectory: boolean, guid?: string) => ipcRenderer.invoke('asset:deleteNode', relativePath, isDirectory, guid),
  updateAlias: (guid: string, alias: string | null) => ipcRenderer.invoke('asset:updateAlias', guid, alias),
  importExternal: () => ipcRenderer.invoke('asset:importExternal'),
  importFile: (filePath: string) => ipcRenderer.invoke('asset:importFile', filePath),
  moveAsset: (sourceRelPath: string, targetDirRelPath: string) => ipcRenderer.invoke('asset:move', sourceRelPath, targetDirRelPath),

  // Dialog
  openFileDialog: (type: string) => ipcRenderer.invoke('dialog:openFile', type),

  // Game launcher (Phase 3)
  launchGame: () => ipcRenderer.invoke('game:launch'),
  stopGame: () => ipcRenderer.invoke('game:stop'),
  onGameStatusChanged: (callback: (status: { isRunning: boolean; message: string }) => void) => {
    const listener = (_event: Electron.IpcRendererEvent, status: { isRunning: boolean; message: string }) => callback(status)
    ipcRenderer.on('game:status', listener)
    return () => ipcRenderer.removeListener('game:status', listener)
  }
})
