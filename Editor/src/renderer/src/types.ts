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

/** The flat tree node structure used after scanning Assets/. */
export interface AssetTreeNode {
  name: string
  path: string
  isDirectory: boolean
  children: AssetTreeNode[]
  extension: string
  size: number
  isRegistered: boolean
  assetEntry?: AssetEntry   // matched entry from registry
}

export interface LogMessage {
  id: number
  text: string
  type: 'info' | 'success' | 'error'
}
