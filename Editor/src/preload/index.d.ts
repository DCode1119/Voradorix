export interface AssetEntry {
  guid: string
  type: 'texture' | 'font' | 'script'
  alias: string | null
  sourcePath: string
  meta?: Record<string, unknown>
}

export interface AssetRegistryData {
  version: number
  assets: AssetEntry[]
  aliases: Record<string, string>
}

export interface FileEntry {
  name: string
  path: string
  isDirectory: boolean
  extension: string
  size: number
  isRegistered: boolean
}

export interface ElectronAPI {
  // Registry
  readAssetRegistry: () => Promise<AssetRegistryData>
  writeAssetRegistry: (data: AssetRegistryData) => Promise<void>

  // File system
  readDirectory: (path: string) => Promise<FileEntry[]>
  readFileBase64: (path: string) => Promise<string | null>
  readFileText: (path: string) => Promise<string | null>
  createDirectory: (relativePath: string) => Promise<{ success: boolean; error?: string }>

  // Asset operations
  importAsset: (sourcePath: string, type: string) => Promise<{ guid: string; sourcePath: string } | { error: string }>
  registerAsset: (relativePath: string, type: string) => Promise<{ guid: string; sourcePath: string } | { error: string }>
  deleteAsset: (guid: string) => Promise<void>
  updateAlias: (guid: string, alias: string | null) => Promise<void>
  importExternal: () => Promise<{ guid?: string; sourcePath?: string; error?: string }>
  importFile: (filePath: string) => Promise<{ guid?: string; sourcePath?: string; error?: string }>
  moveAsset: (sourceRelPath: string, targetDirRelPath: string) => Promise<{ success: boolean; error?: string }>

  // Dialog
  openFileDialog: (type: string) => Promise<{ canceled: boolean; filePath: string | null }>

  // Game launcher (Phase 3)
  launchGame: () => Promise<void>
  stopGame: () => Promise<void>
}

declare global {
  interface Window {
    electronAPI: ElectronAPI
  }
}
