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

export interface ImportCandidate {
  sourcePath: string
  targetPath: string
  exists: boolean
}

export interface ImportPreviewData {
  sources: Array<{ sourcePath: string; sourceIsDirectory: boolean; sourceName: string }>
  sourcePath: string
  sourceIsDirectory: boolean
  targetDir: string
  destinationRoot: string
  directories: Array<{ sourcePath: string; targetPath: string; exists: boolean }>
  candidates: ImportCandidate[]
  conflicts: string[]
}

export interface ImportExecuteResult {
  success: boolean
  imported: string[]
  overwritten: string[]
  skipped: string[]
  errors: string[]
}

export interface GameStatusPayload {
  isRunning: boolean
  message: string
}

export interface GameLaunchResult {
  success: boolean
  status?: 'started' | 'already-running'
  exePath?: string
  error?: string
}

export interface GameStopResult {
  success: boolean
  status?: 'stopped' | 'not-running'
  error?: string
}

export interface ElectronAPI {
  // Registry
  readAssetRegistry: () => Promise<AssetRegistryData>
  writeAssetRegistry: (data: AssetRegistryData) => Promise<void>

  // File system
  readDirectory: (path: string) => Promise<FileEntry[]>
  readFileBase64: (path: string) => Promise<string | null>
  readFileText: (path: string) => Promise<string | null>
  writeFileText: (path: string, content: string) => Promise<{ success: boolean; error?: string }>
  createDirectory: (relativePath: string) => Promise<{ success: boolean; error?: string }>
  pickImportSources: () => Promise<{ canceled: boolean; sources: Array<{ sourcePath: string; sourceIsDirectory: boolean }> }>
  previewImport: (sources: Array<{ sourcePath: string; sourceIsDirectory: boolean }>, targetDirRelPath: string) => Promise<ImportPreviewData | { error: string }>
  executeImport: (sources: Array<{ sourcePath: string; sourceIsDirectory: boolean }>, targetDirRelPath: string, overwriteTargets: string[]) => Promise<ImportExecuteResult | { error: string }>

  // Asset operations
  importAsset: (sourcePath: string, type: string) => Promise<{ guid: string; sourcePath: string } | { error: string }>
  registerAsset: (relativePath: string, type: string) => Promise<{ guid: string; sourcePath: string } | { error: string }>
  deleteAsset: (guid: string) => Promise<void>
  deleteNode: (relativePath: string, isDirectory: boolean, guid?: string) => Promise<{ success: boolean; removedRegistryCount: number; removedPaths: string[]; error?: string }>
  updateAlias: (guid: string, alias: string | null) => Promise<void>
  importExternal: () => Promise<{ guid?: string; sourcePath?: string; error?: string }>
  importFile: (filePath: string) => Promise<{ guid?: string; sourcePath?: string; error?: string }>
  moveAsset: (sourceRelPath: string, targetDirRelPath: string) => Promise<{ success: boolean; error?: string }>

  // Dialog
  openFileDialog: (type: string) => Promise<{ canceled: boolean; filePath: string | null }>

  // Game launcher (Phase 3)
  launchGame: () => Promise<GameLaunchResult>
  stopGame: () => Promise<GameStopResult>
  onGameStatusChanged: (callback: (status: GameStatusPayload) => void) => () => void
}

declare global {
  interface Window {
    electronAPI: ElectronAPI
  }
}
