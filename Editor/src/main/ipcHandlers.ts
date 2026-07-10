import { ipcMain, dialog } from 'electron'
import { readFileSync, writeFileSync, copyFileSync, renameSync, unlinkSync, existsSync, mkdirSync, readdirSync, statSync, rmSync } from 'fs'
import { join, relative, extname, basename, dirname } from 'path'
import { randomUUID } from 'crypto'

// ── 경로 계산 ──────────────────────────────────────────────────────
// Electron-vite dev: __dirname = Editor/out/main
// 프로젝트 루트(Game/) = __dirname/../../../
const PROJECT_ROOT = join(__dirname, '..', '..', '..')
const ASSETS_DIR = join(PROJECT_ROOT, 'Assets')
const REGISTRY_PATH = join(ASSETS_DIR, 'AssetRegistry.json')

// ── 타입 정의 ───────────────────────────────────────────────────────
interface AssetEntry {
  guid: string
  type: 'texture' | 'font' | 'script'
  alias: string | null
  sourcePath: string
  meta?: Record<string, unknown>
}

interface AssetRegistryData {
  version: number
  assets: AssetEntry[]
  aliases: Record<string, string>
}

interface FileEntry {
  name: string
  path: string
  isDirectory: boolean
  extension: string
  size: number
  isRegistered: boolean
}

interface ImportCandidate {
  sourcePath: string
  targetPath: string
  exists: boolean
}

interface ImportDirectoryTarget {
  sourcePath: string
  targetPath: string
  exists: boolean
}

interface ImportPreviewData {
  sources: { sourcePath: string; sourceIsDirectory: boolean; sourceName: string }[]
  sourcePath: string
  sourceIsDirectory: boolean
  targetDir: string
  destinationRoot: string
  directories: ImportDirectoryTarget[]
  candidates: ImportCandidate[]
  conflicts: string[]
}

interface ImportExecuteResult {
  success: boolean
  imported: string[]
  overwritten: string[]
  skipped: string[]
  errors: string[]
}

interface DeleteNodeResult {
  success: boolean
  removedRegistryCount: number
  removedPaths: string[]
  error?: string
}

// ── 헬퍼 ────────────────────────────────────────────────────────────

/** 에셋 타입별 서브디렉토리 매핑 */
function assetTypeToDir(type: string): string {
  switch (type) {
    case 'texture': return 'Images'
    case 'font':    return 'Fonts'
    case 'script':  return 'Scripts'
    default:        return 'Other'
  }
}

/** 파일 확장자로 에셋 타입 추론 */
function extToAssetType(ext: string): 'texture' | 'font' | 'script' | null {
  switch (ext.toLowerCase()) {
    case '.png':
    case '.jpg':
    case '.jpeg':   return 'texture'
    case '.ttf':
    case '.otf':    return 'font'
    case '.txt':    return 'script'
    default:        return null
  }
}

/** 중복 시 접미사 추가 */
function uniquePath(targetPath: string): string {
  if (!existsSync(targetPath)) return targetPath
  const dir = dirname(targetPath)
  const ext = extname(targetPath)
  const base = basename(targetPath, ext)
  for (let i = 1; ; i++) {
    const candidate = join(dir, `${base}_${i}${ext}`)
    if (!existsSync(candidate)) return candidate
  }
}

function normalizeRelativePath(relativePath: string): string {
  return relativePath.replace(/\\/g, '/')
}

function pathsEqual(left: string, right: string): boolean {
  return normalizeRelativePath(left) === normalizeRelativePath(right)
}

function ensureParentDirectory(fullPath: string): void {
  const parent = dirname(fullPath)
  if (!existsSync(parent)) {
    mkdirSync(parent, { recursive: true })
  }
}

function collectSourceFiles(sourcePath: string): string[] {
  if (!existsSync(sourcePath)) {
    return []
  }

  const stat = statSync(sourcePath)
  if (stat.isFile()) {
    return [sourcePath]
  }

  const collected: string[] = []
  const entries = readdirSync(sourcePath, { withFileTypes: true })
  for (const entry of entries) {
    const childPath = join(sourcePath, entry.name)
    if (entry.isDirectory()) {
      collected.push(...collectSourceFiles(childPath))
    } else if (entry.isFile()) {
      collected.push(childPath)
    }
  }

  return collected
}

function collectSourceDirectories(sourcePath: string): string[] {
  if (!existsSync(sourcePath)) {
    return []
  }

  const stat = statSync(sourcePath)
  if (!stat.isDirectory()) {
    return []
  }

  const collected: string[] = [sourcePath]
  const entries = readdirSync(sourcePath, { withFileTypes: true })
  for (const entry of entries) {
    const childPath = join(sourcePath, entry.name)
    if (entry.isDirectory()) {
      collected.push(...collectSourceDirectories(childPath))
    }
  }

  return collected
}

function cleanupEmptyParentDirectories(startFullPath: string): void {
  let current = startFullPath

  while (current.startsWith(ASSETS_DIR) && current !== ASSETS_DIR) {
    if (!existsSync(current)) {
      current = dirname(current)
      continue
    }

    const entries = readdirSync(current)
    if (entries.length > 0) {
      break
    }

    try {
      rmSync(current)
    } catch {
      break
    }

    current = dirname(current)
  }
}

function deleteRegistryEntries(targetRelPath: string, isDirectory: boolean, guid?: string): { removedRegistryCount: number; removedPaths: string[] } {
  const registry = readRegistry()
  const normalizedTarget = normalizeRelativePath(targetRelPath)
  const directoryPrefix = normalizedTarget ? `${normalizedTarget}/` : ''
  const removedPaths: string[] = []

  const nextAssets = registry.assets.filter(asset => {
    const assetPath = normalizeRelativePath(asset.sourcePath)
    const matchesGuid = guid ? asset.guid === guid : false
    const matchesPath = isDirectory
      ? assetPath === normalizedTarget || (directoryPrefix.length > 0 && assetPath.startsWith(directoryPrefix))
      : assetPath === normalizedTarget

    if (matchesGuid || matchesPath) {
      removedPaths.push(assetPath)
      return false
    }

    return true
  })

  const removedRegistryCount = registry.assets.length - nextAssets.length
  if (removedRegistryCount > 0) {
    registry.assets = nextAssets
    saveRegistry(registry)
  }

  return { removedRegistryCount, removedPaths }
}

function buildImportPreview(sources: { sourcePath: string; sourceIsDirectory: boolean }[], targetDirRelPath: string): ImportPreviewData {
  const normalizedTargetDir = normalizeRelativePath(targetDirRelPath)
  const directories: ImportDirectoryTarget[] = []
  const candidates: ImportCandidate[] = []
  const conflicts: string[] = []
  const previewSources = sources.map(source => ({
    sourcePath: source.sourcePath,
    sourceIsDirectory: source.sourceIsDirectory,
    sourceName: basename(source.sourcePath)
  }))

  for (const source of sources) {
    const sourceFiles = collectSourceFiles(source.sourcePath)
    const destinationRoot = source.sourceIsDirectory
      ? normalizeRelativePath(join(normalizedTargetDir, basename(source.sourcePath)))
      : normalizedTargetDir

    if (source.sourceIsDirectory) {
      for (const sourceDir of collectSourceDirectories(source.sourcePath)) {
        const targetDir = normalizeRelativePath(join(destinationRoot, relative(source.sourcePath, sourceDir)))
        const exists = existsSync(join(ASSETS_DIR, targetDir))
        directories.push({
          sourcePath: sourceDir,
          targetPath: targetDir,
          exists
        })
      }
    }

    for (const sourceFile of sourceFiles) {
      const targetRelPath = source.sourceIsDirectory
        ? normalizeRelativePath(join(destinationRoot, relative(source.sourcePath, sourceFile)))
        : normalizeRelativePath(join(destinationRoot, basename(sourceFile)))
      const targetFullPath = join(ASSETS_DIR, targetRelPath)
      const exists = existsSync(targetFullPath)

      candidates.push({
        sourcePath: sourceFile,
        targetPath: targetRelPath,
        exists
      })

      if (exists) {
        conflicts.push(targetRelPath)
      }
    }
  }

  const first = sources[0]
  const destinationRoot = first
    ? (first.sourceIsDirectory
      ? normalizeRelativePath(join(normalizedTargetDir, basename(first.sourcePath)))
      : normalizedTargetDir)
    : normalizedTargetDir

  return {
    sources: previewSources,
    sourcePath: first?.sourcePath ?? '',
    sourceIsDirectory: first?.sourceIsDirectory ?? false,
    targetDir: normalizedTargetDir,
    destinationRoot,
    directories,
    candidates,
    conflicts: Array.from(new Set(conflicts))
  }
}

function upsertRegistryEntry(relativePath: string): void {
  const registry = readRegistry()
  const normalizedRelativePath = normalizeRelativePath(relativePath)

  if (registry.assets.some(asset => pathsEqual(asset.sourcePath, normalizedRelativePath))) {
    return
  }

  const assetType = extToAssetType(extname(normalizedRelativePath))
  if (!assetType) {
    return
  }

  registry.assets.push({
    guid: randomUUID(),
    type: assetType,
    alias: null,
    sourcePath: normalizedRelativePath
  })

  saveRegistry(registry)
}

function copyImportCandidates(preview: ImportPreviewData, overwriteTargets: string[]): ImportExecuteResult {
  const overwriteSet = new Set(overwriteTargets.map(normalizeRelativePath))
  const result: ImportExecuteResult = {
    success: true,
    imported: [],
    overwritten: [],
    skipped: [],
    errors: []
  }

  for (const directory of preview.directories ?? []) {
    try {
      mkdirSync(join(ASSETS_DIR, normalizeRelativePath(directory.targetPath)), { recursive: true })
    } catch (err) {
      result.success = false
      result.errors.push(`Failed to create directory ${directory.targetPath}: ${(err as Error).message}`)
    }
  }

  for (const candidate of preview.candidates) {
    const targetPath = normalizeRelativePath(candidate.targetPath)
    const targetFullPath = join(ASSETS_DIR, targetPath)
    const shouldOverwrite = overwriteSet.has(targetPath)

    if (candidate.exists && !shouldOverwrite) {
      result.skipped.push(targetPath)
      continue
    }

    try {
      ensureParentDirectory(targetFullPath)
      copyFileSync(candidate.sourcePath, targetFullPath)

      if (candidate.exists) {
        result.overwritten.push(targetPath)
      } else {
        result.imported.push(targetPath)
      }

      upsertRegistryEntry(targetPath)
    } catch (err) {
      result.success = false
      result.errors.push(`Failed to copy ${targetPath}: ${(err as Error).message}`)
    }
  }

  return result
}

/** Registry 읽기 (없으면 기본값) */
function readRegistry(): AssetRegistryData {
  if (!existsSync(REGISTRY_PATH)) {
    return { version: 1, assets: [], aliases: {} }
  }
  try {
    return JSON.parse(readFileSync(REGISTRY_PATH, 'utf-8'))
  } catch {
    return { version: 1, assets: [], aliases: {} }
  }
}

/** Registry 저장 */
function saveRegistry(data: AssetRegistryData): void {
  if (!existsSync(dirname(REGISTRY_PATH))) {
    mkdirSync(dirname(REGISTRY_PATH), { recursive: true })
  }
  writeFileSync(REGISTRY_PATH, JSON.stringify(data, null, 2), 'utf-8')
}

// ── IPC 핸들러 등록 ────────────────────────────────────────────────

export function registerIpcHandlers(): void {

  // Registry 읽기
  ipcMain.handle('registry:read', async (): Promise<AssetRegistryData> => {
    return readRegistry()
  })

  // Registry 쓰기
  ipcMain.handle('registry:write', async (_event, data: AssetRegistryData): Promise<void> => {
    saveRegistry(data)
  })

  // 디렉토리 읽기 (Game/Assets/ 기준 상대경로)
  ipcMain.handle('fs:readDirectory', async (_event, relativePath: string): Promise<FileEntry[]> => {
    const fullPath = join(ASSETS_DIR, relativePath)
    if (!existsSync(fullPath)) return []

    const registry = readRegistry()
    const entries = readdirSync(fullPath, { withFileTypes: true })

    return entries.map(e => {
      const entryRelPath = normalizeRelativePath(relativePath ? join(relativePath, e.name) : e.name)
      let entrySize = 0
      if (e.isFile()) {
        try { entrySize = statSync(join(fullPath, e.name)).size } catch { /* ignore */ }
      }
      return {
        name: e.name,
        path: entryRelPath,
        isDirectory: e.isDirectory(),
        extension: e.isFile() ? extname(e.name).toLowerCase() : '',
        size: entrySize,
        isRegistered: registry.assets.some(a => pathsEqual(a.sourcePath, entryRelPath))
      }
    })
  })

  // 파일 읽기 (Base64 반환 — 미리보기용)
  ipcMain.handle('fs:readFileBase64', async (_event, relativePath: string): Promise<string | null> => {
    const fullPath = join(ASSETS_DIR, relativePath)
    if (!existsSync(fullPath)) return null
    try {
      const buffer = readFileSync(fullPath)
      return buffer.toString('base64')
    } catch {
      return null
    }
  })

  // 파일 읽기 (텍스트 — 스크립트 미리보기용)
  ipcMain.handle('fs:readFileText', async (_event, relativePath: string): Promise<string | null> => {
    const fullPath = join(ASSETS_DIR, relativePath)
    if (!existsSync(fullPath)) return null
    try {
      return readFileSync(fullPath, 'utf-8')
    } catch {
      return null
    }
  })

  // 에셋 Import
  ipcMain.handle('asset:import', async (_event, sourcePath: string, type: string): Promise<{ guid: string; sourcePath: string } | { error: string }> => {
    const assetType = extToAssetType(extname(sourcePath))
    if (!assetType) {
      return { error: 'Unsupported file type' }
    }

    const targetDir = join(ASSETS_DIR, assetTypeToDir(type))
    if (!existsSync(targetDir)) {
      mkdirSync(targetDir, { recursive: true })
    }

    const fileName = basename(sourcePath)
    const destPath = uniquePath(join(targetDir, fileName))

    try {
      copyFileSync(sourcePath, destPath)
    } catch (err) {
      return { error: `Failed to copy file: ${(err as Error).message}` }
    }

    const guid = randomUUID()
    const relPath = normalizeRelativePath(relative(ASSETS_DIR, destPath))
    const registry = readRegistry()

    registry.assets.push({
      guid,
      type: assetType,
      alias: null,
      sourcePath: relPath
    })

    saveRegistry(registry)

    return { guid, sourcePath: relPath }
  })

  // 에셋 Register (이미 Assets/ 안에 있는 파일을 Registry에만 등록)
  ipcMain.handle('asset:register', async (_event, relativePath: string, type: string): Promise<{ guid: string; sourcePath: string } | { error: string }> => {
    const normalizedRelativePath = normalizeRelativePath(relativePath)
    const fullPath = join(ASSETS_DIR, normalizedRelativePath)
    if (!existsSync(fullPath)) {
      return { error: 'File not found' }
    }

    const assetType = extToAssetType(extname(normalizedRelativePath))
    if (!assetType) {
      return { error: 'Unsupported file type' }
    }

    const guid = randomUUID()
    const registry = readRegistry()

    // 중복 등록 방지
    if (registry.assets.some(a => pathsEqual(a.sourcePath, normalizedRelativePath))) {
      return { error: 'Already registered' }
    }

    registry.assets.push({
      guid,
      type: assetType,
      alias: null,
      sourcePath: normalizedRelativePath
    })

    saveRegistry(registry)

    return { guid, sourcePath: normalizedRelativePath }
  })

  // 에셋 삭제 (Registry에서 제거, 파일은 유지)
  ipcMain.handle('asset:delete', async (_event, guid: string): Promise<void> => {
    const registry = readRegistry()
    registry.assets = registry.assets.filter(a => a.guid !== guid)
    saveRegistry(registry)
  })

  // 노드 삭제 (파일/디렉토리 + Registry)
  ipcMain.handle('asset:deleteNode', async (_event, relativePath: string, isDirectory: boolean, guid?: string): Promise<DeleteNodeResult> => {
    const normalizedRelativePath = normalizeRelativePath(relativePath)
    const fullPath = join(ASSETS_DIR, normalizedRelativePath)

    try {
      const registryResult = deleteRegistryEntries(normalizedRelativePath, isDirectory, guid)

      if (existsSync(fullPath)) {
        if (isDirectory) {
          rmSync(fullPath, { recursive: true, force: true })
        } else {
          unlinkSync(fullPath)
        }
      }

      cleanupEmptyParentDirectories(dirname(fullPath))

      return {
        success: true,
        removedRegistryCount: registryResult.removedRegistryCount,
        removedPaths: registryResult.removedPaths
      }
    } catch (err) {
      return {
        success: false,
        removedRegistryCount: 0,
        removedPaths: [],
        error: (err as Error).message
      }
    }
  })

  // Alias 업데이트
  ipcMain.handle('asset:updateAlias', async (_event, guid: string, alias: string | null): Promise<void> => {
    const registry = readRegistry()
    const asset = registry.assets.find(a => a.guid === guid)
    if (asset) {
      asset.alias = alias
      saveRegistry(registry)
    }
  })

  // 디렉토리 생성 (Assets/ 아래)
  ipcMain.handle('fs:createDirectory', async (_event, relativePath: string): Promise<{ success: boolean; error?: string }> => {
    const fullPath = join(ASSETS_DIR, relativePath)
    try {
      mkdirSync(fullPath, { recursive: true })
      return { success: true }
    } catch (err) {
      return { success: false, error: (err as Error).message }
    }
  })

  // Import용 파일 선택
  ipcMain.handle('asset:pickImportSources', async (): Promise<{ canceled: boolean; sources: { sourcePath: string; sourceIsDirectory: boolean }[] }> => {
    const result = await dialog.showOpenDialog({
      properties: ['openFile', 'openDirectory', 'multiSelections'],
      filters: [
        { name: 'All Supported', extensions: ['png', 'jpg', 'jpeg', 'ttf', 'otf', 'txt'] },
        { name: 'Images', extensions: ['png', 'jpg', 'jpeg'] },
        { name: 'Fonts', extensions: ['ttf', 'otf'] },
        { name: 'Scripts', extensions: ['txt'] }
      ]
    })

    return {
      canceled: result.canceled,
      sources: result.filePaths.map(path => ({
        sourcePath: path,
        sourceIsDirectory: existsSync(path) && statSync(path).isDirectory()
      }))
    }
  })

  // Import 미리보기 (충돌 목록)
  ipcMain.handle('asset:previewImport', async (_event, sources: { sourcePath: string; sourceIsDirectory: boolean }[], targetDirRelPath: string): Promise<ImportPreviewData | { error: string }> => {
    if (!sources.length) {
      return { error: 'No sources selected' }
    }

    try {
      return buildImportPreview(sources, targetDirRelPath)
    } catch (err) {
      return { error: (err as Error).message }
    }
  })

  // Import 실행 (선택된 충돌만 덮어씀)
  ipcMain.handle('asset:executeImport', async (_event, sources: { sourcePath: string; sourceIsDirectory: boolean }[], targetDirRelPath: string, overwriteTargets: string[]): Promise<ImportExecuteResult | { error: string }> => {
    if (!sources.length) {
      return { error: 'No sources selected' }
    }

    try {
      const preview = buildImportPreview(sources, targetDirRelPath)
      return copyImportCandidates(preview, overwriteTargets)
    } catch (err) {
      return { error: (err as Error).message }
    }
  })

  // 외부 파일 Import (파일 선택 → Assets/로 복사 → Registry 등록)
  ipcMain.handle('asset:importExternal', async (_event): Promise<{ guid?: string; sourcePath?: string; error?: string }> => {
    // 파일 선택 대화상자
    const result = await dialog.showOpenDialog({
      properties: ['openFile'],
      filters: [
        { name: 'All Supported', extensions: ['png', 'jpg', 'jpeg', 'ttf', 'otf', 'txt'] },
        { name: 'Images', extensions: ['png', 'jpg', 'jpeg'] },
        { name: 'Fonts', extensions: ['ttf', 'otf'] },
        { name: 'Scripts', extensions: ['txt'] }
      ]
    })

    if (result.canceled || result.filePaths.length === 0) {
      return { error: 'Canceled' }
    }

    const sourcePath = result.filePaths[0]
    const ext = extname(sourcePath).toLowerCase()
    const assetType = extToAssetType(ext)

    if (!assetType) {
      return { error: `Unsupported file type: ${ext}` }
    }

    const targetDir = join(ASSETS_DIR, assetTypeToDir(assetType))
    if (!existsSync(targetDir)) {
      mkdirSync(targetDir, { recursive: true })
    }

    const fileName = basename(sourcePath)
    const destPath = uniquePath(join(targetDir, fileName))

    try {
      copyFileSync(sourcePath, destPath)
    } catch (err) {
      return { error: `Failed to copy file: ${(err as Error).message}` }
    }

    const guid = randomUUID()
    const relPath = normalizeRelativePath(relative(ASSETS_DIR, destPath))
    const registry = readRegistry()

    registry.assets.push({
      guid,
      type: assetType,
      alias: null,
      sourcePath: relPath
    })

    saveRegistry(registry)

    return { guid, sourcePath: relPath }
  })

  // 외부 파일 Import (드래그 앤 드롭 — 경로 직접 지정)
  ipcMain.handle('asset:importFile', async (_event, filePath: string): Promise<{ guid?: string; sourcePath?: string; error?: string }> => {
    const ext = extname(filePath).toLowerCase()
    const assetType = extToAssetType(ext)

    if (!assetType) {
      return { error: `Unsupported file type: ${ext}` }
    }

    const targetDir = join(ASSETS_DIR, assetTypeToDir(assetType))
    if (!existsSync(targetDir)) {
      mkdirSync(targetDir, { recursive: true })
    }

    const fileName = basename(filePath)
    const destPath = uniquePath(join(targetDir, fileName))

    try {
      copyFileSync(filePath, destPath)
    } catch (err) {
      return { error: `Failed to copy file: ${(err as Error).message}` }
    }

    const guid = randomUUID()
    const relPath = normalizeRelativePath(relative(ASSETS_DIR, destPath))
    const registry = readRegistry()

    registry.assets.push({
      guid,
      type: assetType,
      alias: null,
      sourcePath: relPath
    })

    saveRegistry(registry)

    return { guid, sourcePath: relPath }
  })

  // 파일/폴더 이동 (Assets/ 내부에서 드래그 앤 드롭)
  ipcMain.handle('asset:move', async (_event, sourceRelPath: string, targetDirRelPath: string): Promise<{ success: boolean; error?: string }> => {
    const srcFull = join(ASSETS_DIR, sourceRelPath)
    const dstFull = join(ASSETS_DIR, targetDirRelPath, basename(sourceRelPath))

    if (!existsSync(srcFull)) {
      return { success: false, error: 'Source not found' }
    }

    if (!existsSync(dirname(dstFull))) {
      mkdirSync(dirname(dstFull), { recursive: true })
    }

    try {
      renameSync(srcFull, dstFull)
    } catch (err) {
      // cross-device: copy + delete
      try {
        copyFileSync(srcFull, dstFull)
        unlinkSync(srcFull)
      } catch (err2) {
        return { success: false, error: `Failed to move: ${(err2 as Error).message}` }
      }
    }

    // Registry 경로 업데이트
    const newRelPath = normalizeRelativePath(join(targetDirRelPath, basename(sourceRelPath)))
    const registry = readRegistry()
    let updated = false
    for (const asset of registry.assets) {
      if (pathsEqual(asset.sourcePath, sourceRelPath)) {
        asset.sourcePath = newRelPath
        updated = true
        break
      }
    }
    if (updated) {
      saveRegistry(registry)
    }

    return { success: true }
  })

  // 파일 선택 대화상자 열기
  ipcMain.handle('dialog:openFile', async (_event, type: string): Promise<{ canceled: boolean; filePath: string | null }> => {
    const filters: Electron.FileFilter[] = []
    switch (type) {
      case 'texture':
        filters.push({ name: 'Images', extensions: ['png', 'jpg', 'jpeg'] })
        break
      case 'font':
        filters.push({ name: 'Fonts', extensions: ['ttf', 'otf'] })
        break
      case 'script':
        filters.push({ name: 'Scripts', extensions: ['txt'] })
        break
      default:
        filters.push({ name: 'All Files', extensions: ['*'] })
    }

    const result = await dialog.showOpenDialog({
      properties: ['openFile'],
      filters
    })

    return {
      canceled: result.canceled,
      filePath: result.filePaths[0] ?? null
    }
  })
}
