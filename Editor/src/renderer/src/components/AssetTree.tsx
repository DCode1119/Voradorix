import { useState, useEffect, useCallback, useRef } from 'react'
import { AssetTreeNode, FileEntry, AssetEntry } from '../types'

// ── Props ──────────────────────────────────────────────────────────

interface AssetTreeProps {
  onSelect: (node: AssetTreeNode | null) => void
  onLog: (msg: string, type: 'info' | 'success' | 'error') => void
  refreshTrigger: number
  selectedNode: AssetTreeNode | null
}

interface ImportModalState {
  visible: boolean
  sources: Array<{ sourcePath: string; sourceIsDirectory: boolean; sourceName: string }>
  targetDir: string
  conflicts: string[]
  selections: Record<string, boolean>
  submitting: boolean
}

interface ContextMenuState {
  visible: boolean
  x: number
  y: number
  node: AssetTreeNode | null
}

interface ActionModalState {
  visible: boolean
  kind: 'rename' | 'createScript' | null
  node: AssetTreeNode | null
  value: string
  submitting: boolean
}

// ── Helpers ────────────────────────────────────────────────────────

function normalizeAssetPath(path: string): string {
  return path.replace(/\\/g, '/')
}

function getParentAssetPath(path: string): string {
  const normalized = normalizeAssetPath(path).replace(/\/+$/, '')
  const parts = normalized.split('/').filter(Boolean)
  parts.pop()
  return parts.join('/')
}

function getTargetDirectoryPath(node: AssetTreeNode | null): string {
  if (!node) {
    return ''
  }

  if (node.isDirectory) {
    return normalizeAssetPath(node.path)
  }

  return getParentAssetPath(node.path)
}

function buildTargetLabel(targetDir: string): string {
  return targetDir ? `Assets/${targetDir}` : 'Assets'
}

function buildSourceLabel(source: { sourcePath: string; sourceIsDirectory: boolean; sourceName: string }): string {
  return `${source.sourceIsDirectory ? 'Dir' : 'File'}: ${source.sourceName}`
}

async function scanDirectory(
  relativePath: string,
  registryAssets: AssetEntry[]
): Promise<AssetTreeNode[]> {
  const entries: FileEntry[] = await window.electronAPI.readDirectory(relativePath)
  const nodes: AssetTreeNode[] = []

  for (const e of entries) {
    const entryPath = normalizeAssetPath(e.path)
    const match = registryAssets.find(a => normalizeAssetPath(a.sourcePath) === entryPath)
    const node: AssetTreeNode = {
      name: e.name,
      path: entryPath,
      isDirectory: e.isDirectory,
      children: [],
      extension: e.extension,
      size: e.size,
      isRegistered: e.isRegistered,
      assetEntry: match || undefined
    }

    if (e.isDirectory) {
      node.children = await scanDirectory(entryPath, registryAssets)
    }

    nodes.push(node)
  }

  // directories first, then alphabetically
  nodes.sort((a, b) => {
    if (a.isDirectory !== b.isDirectory) return a.isDirectory ? -1 : 1
    return a.name.localeCompare(b.name)
  })

  return nodes
}

// ── TreeNodeItem ───────────────────────────────────────────────────

function TreeNodeItem({
  node,
  depth,
  onSelect,
  selectedPath,
  onContextMenu
}: {
  node: AssetTreeNode
  depth: number
  onSelect: (node: AssetTreeNode) => void
  selectedPath: string | null
  onContextMenu: (event: React.MouseEvent, node: AssetTreeNode) => void
}) {
  const [expanded, setExpanded] = useState(depth < 1)

  const handleClick = () => {
    if (node.isDirectory) {
      setExpanded(prev => !prev)
      onSelect(node)
      return
    }

    onSelect(node)
  }

  const handleDragStart = (e: React.DragEvent) => {
    if (node.isDirectory) return
    e.dataTransfer.setData('text/x-voradorix-asset-path', node.path)
    e.dataTransfer.effectAllowed = 'move'
  }

  const isSelected = selectedPath === node.path
  const hasAlias = Boolean(node.assetEntry?.alias)
  const needsAlias = Boolean(node.assetEntry && !hasAlias)

  const classes = [
    'tree-item',
    isSelected ? 'selected' : '',
    node.isDirectory ? 'directory' : 'file',
    node.isRegistered ? 'registered' : 'unregistered',
    needsAlias ? 'alias-missing' : ''
  ].filter(Boolean).join(' ')

  return (
    <div>
      <div
        className={classes}
        style={{ paddingLeft: `${depth * 16 + 8}px` }}
        onClick={handleClick}
        onDoubleClick={node.isDirectory ? undefined : handleClick}
        draggable={!node.isDirectory}
        onDragStart={handleDragStart}
        onContextMenu={event => onContextMenu(event, node)}
      >
        <span className="tree-icon">
          {node.isDirectory
            ? (expanded ? '▼' : '▶')
            : (node.extension === '.png' || node.extension === '.jpg' || node.extension === '.jpeg'
              ? '🖼'
              : node.extension === '.ttf' || node.extension === '.otf'
                ? '🔤'
                : node.extension === '.txt'
                  ? '📜'
                  : '📄')
          }
        </span>
        <span
          className={`tree-status ${needsAlias ? 'alias-missing' : ''}`}
          title={needsAlias ? 'Alias 없음' : (node.isRegistered ? 'Registered' : 'Not registered')}
        >
          {needsAlias ? '🟡' : (node.isRegistered ? '🟢' : '⚪')}
        </span>
        <span className="tree-name">{node.name}</span>
      </div>
      {node.isDirectory && expanded && (
        <div className="tree-children">
          {node.children.map(child => (
            <TreeNodeItem
              key={child.path}
              node={child}
              depth={depth + 1}
              onSelect={onSelect}
              selectedPath={selectedPath}
              onContextMenu={onContextMenu}
            />
          ))}
          {node.children.length === 0 && (
            <div className="tree-empty" style={{ paddingLeft: `${(depth + 1) * 16 + 8}px` }}>
              (empty)
            </div>
          )}
        </div>
      )}
    </div>
  )
}

// ── AssetTree ──────────────────────────────────────────────────────

export default function AssetTree({ onSelect, onLog, refreshTrigger, selectedNode }: AssetTreeProps) {
  const [treeData, setTreeData] = useState<AssetTreeNode[]>([])
  const [loading, setLoading] = useState(true)
  const [filter, setFilter] = useState('')
  const [showNewFolder, setShowNewFolder] = useState(false)
  const [newFolderName, setNewFolderName] = useState('')
  const [creatingFolder, setCreatingFolder] = useState(false)
  const newFolderInputRef = useRef<HTMLInputElement>(null)
  const contextMenuRef = useRef<HTMLDivElement>(null)

  const [actionModal, setActionModal] = useState<ActionModalState>({
    visible: false,
    kind: null,
    node: null,
    value: '',
    submitting: false
  })

  const [importModal, setImportModal] = useState<ImportModalState>({
    visible: false,
    sources: [],
    targetDir: '',
    conflicts: [],
    selections: {},
    submitting: false
  })

  const [contextMenu, setContextMenu] = useState<ContextMenuState>({
    visible: false,
    x: 0,
    y: 0,
    node: null
  })

  const loadTree = useCallback(async () => {
    setLoading(true)
    try {
      const registry = await window.electronAPI.readAssetRegistry()
      const children = await scanDirectory('', registry.assets)
      const rootNode: AssetTreeNode = {
        name: 'Assets',
        path: '',
        isDirectory: true,
        children,
        extension: '',
        size: 0,
        isRegistered: false
      }
      setTreeData([rootNode])
    } catch (err) {
      onLog(`Failed to load asset tree: ${(err as Error).message}`, 'error')
    } finally {
      setLoading(false)
    }
  }, [onLog])

  useEffect(() => {
    loadTree()
  }, [loadTree, refreshTrigger])

  useEffect(() => {
    if (showNewFolder) {
      newFolderInputRef.current?.focus()
    }
  }, [showNewFolder])

  useEffect(() => {
    if (contextMenu.visible) {
      const handleClose = (event: PointerEvent) => {
        const menu = contextMenuRef.current
        if (menu && menu.contains(event.target as Node)) {
          return
        }
        setContextMenu(prev => ({ ...prev, visible: false }))
      }

      window.addEventListener('pointerdown', handleClose, true)
      return () => window.removeEventListener('pointerdown', handleClose, true)
    }
  }, [contextMenu.visible])

  const handleContextMenu = (event: React.MouseEvent, node: AssetTreeNode) => {
    event.preventDefault()
    event.stopPropagation()

    if (node.path === '' && node.isDirectory) {
      return
    }

    onSelect(node)
    setContextMenu({
      visible: true,
      x: event.clientX,
      y: event.clientY,
      node
    })
  }

  const handleDeleteNode = async () => {
    const node = contextMenu.node
    if (!node) return

    setContextMenu(prev => ({ ...prev, visible: false }))

    const label = node.isDirectory ? `directory "${node.name}"` : `file "${node.name}"`
    const confirmed = window.confirm(`Delete ${label} and all related asset records?\nThis cannot be undone.`)
    if (!confirmed) return

    try {
      const result = await window.electronAPI.deleteNode(node.path, node.isDirectory, node.assetEntry?.guid)
      if (result.success) {
        onLog(`Deleted ${label}`, 'success')
        loadTree()
        if (selectedNode?.path === node.path) {
          onSelect(null)
        }
      } else {
        onLog(`Delete failed: ${result.error ?? 'unknown error'}`, 'error')
      }
    } catch (err) {
      onLog(`Delete failed: ${(err as Error).message}`, 'error')
    }
  }

  const handleRenameNode = async () => {
    const node = contextMenu.node
    if (!node) return

    setContextMenu(prev => ({ ...prev, visible: false }))
    if (node.path === '' && node.isDirectory) return

    setActionModal({
      visible: true,
      kind: 'rename',
      node,
      value: node.name,
      submitting: false
    })
  }

  const handleCreateScriptAsset = async () => {
    const node = contextMenu.node
    if (!node || !node.isDirectory) return

    setContextMenu(prev => ({ ...prev, visible: false }))
    setActionModal({
      visible: true,
      kind: 'createScript',
      node,
      value: 'NewScript',
      submitting: false
    })
  }

  const closeActionModal = () => {
    if (actionModal.submitting) return
    setActionModal({ visible: false, kind: null, node: null, value: '', submitting: false })
  }

  const submitActionModal = async () => {
    const node = actionModal.node
    const value = actionModal.value.trim()
    if (!node || !value) return

    setActionModal(prev => ({ ...prev, submitting: true }))
    try {
      if (actionModal.kind === 'rename') {
        const result = await window.electronAPI.renameAssetNode(node.path, value, node.isDirectory)
        if (result.success) {
          onLog(`Renamed ${node.isDirectory ? 'directory' : 'file'} to ${value}`, 'success')
          loadTree()
          if (selectedNode?.path === node.path) {
            onSelect(null)
          }
          setActionModal({ visible: false, kind: null, node: null, value: '', submitting: false })
        } else {
          onLog(`Rename failed: ${result.error ?? 'unknown error'}`, 'error')
          setActionModal(prev => ({ ...prev, submitting: false }))
        }
      } else if (actionModal.kind === 'createScript') {
        const result = await window.electronAPI.createScriptAsset(node.path, value)
        if (result.success) {
          onLog(`Created script asset: ${result.sourcePath}`, 'success')
          loadTree()
          setActionModal({ visible: false, kind: null, node: null, value: '', submitting: false })
        } else {
          onLog(`Create script failed: ${result.error ?? 'unknown error'}`, 'error')
          setActionModal(prev => ({ ...prev, submitting: false }))
        }
      }
    } catch (err) {
      onLog(`Action failed: ${(err as Error).message}`, 'error')
      setActionModal(prev => ({ ...prev, submitting: false }))
    }
  }

  const handleContextMenuItem = (action: () => void | Promise<void>) => (event: React.MouseEvent | React.PointerEvent) => {
    event.preventDefault()
    event.stopPropagation()
    void action()
  }

  const handleCloseImportModal = () => {
    if (importModal.submitting) return
    setImportModal(prev => ({ ...prev, visible: false }))
  }

  const executeImport = useCallback(async (
    sources: Array<{ sourcePath: string; sourceIsDirectory: boolean }>,
    targetDir: string,
    overwriteTargets: string[]
  ) => {
    setImportModal(prev => ({ ...prev, submitting: true }))
    try {
      const result = await window.electronAPI.executeImport(sources, targetDir, overwriteTargets)

      if ('error' in result) {
        onLog(`Import failed: ${result.error}`, 'error')
        return
      }

      if (result.errors.length > 0) {
        onLog(`Import finished with ${result.errors.length} error(s)`, 'error')
        for (const error of result.errors.slice(0, 5)) {
          onLog(`  ${error}`, 'error')
        }
      }

      if (result.imported.length > 0 || result.overwritten.length > 0) {
        const importedCount = result.imported.length + result.overwritten.length
        onLog(`Import complete: ${importedCount} file(s) (${result.overwritten.length} overwritten)`, 'success')
        loadTree()
      } else {
        onLog('Import skipped: no files were copied', 'info')
      }
    } catch (err) {
      onLog(`Import failed: ${(err as Error).message}`, 'error')
    } finally {
      setImportModal(prev => ({ ...prev, submitting: false, visible: false }))
    }
  }, [loadTree, onLog])

  const openImportDialog = useCallback(async () => {
    const pickerResult = await window.electronAPI.pickImportSources()
    if (pickerResult.canceled || pickerResult.sources.length === 0) {
      return
    }

    const targetDir = getTargetDirectoryPath(selectedNode)
    const preview = await window.electronAPI.previewImport(pickerResult.sources, targetDir)
    if ('error' in preview) {
      onLog(`Import preview failed: ${preview.error}`, 'error')
      return
    }

    if (preview.conflicts.length === 0) {
      await executeImport(pickerResult.sources, targetDir, [])
      return
    }

    setImportModal({
      visible: true,
      sources: pickerResult.sources.map(source => ({
        ...source,
        sourceName: source.sourcePath.split(/[\\/]/).filter(Boolean).pop() ?? source.sourcePath
      })),
      targetDir,
      conflicts: preview.conflicts,
      selections: Object.fromEntries(preview.conflicts.map(conflict => [conflict, true])),
      submitting: false
    })
  }, [executeImport, onLog, selectedNode])

  const handleImportFile = async () => {
    await openImportDialog()
  }

  const handleConfirmImportModal = async () => {
    if (!importModal.visible) return

    const overwriteTargets = importModal.conflicts.filter(path => importModal.selections[path])
    await executeImport(
      importModal.sources,
      importModal.targetDir,
      overwriteTargets
    )
  }

  const handleToggleConflict = (targetPath: string) => {
    setImportModal(prev => ({
      ...prev,
      selections: {
        ...prev.selections,
        [targetPath]: !prev.selections[targetPath]
      }
    }))
  }

  const handleStartNewFolder = () => {
    setNewFolderName('')
    setShowNewFolder(true)
  }

  const handleConfirmNewFolder = async () => {
    const name = newFolderName.trim()
    if (!name) return

    const targetDir = getTargetDirectoryPath(selectedNode)
    const relativePath = normalizeAssetPath(targetDir ? `${targetDir}/${name}` : name)

    setCreatingFolder(true)
    try {
      const result = await window.electronAPI.createDirectory(relativePath)
      if (result.success) {
        onLog(`Created folder: ${relativePath}`, 'success')
        setShowNewFolder(false)
        setNewFolderName('')
        loadTree()
      } else {
        onLog(`Failed to create folder: ${result.error}`, 'error')
      }
    } catch (err) {
      onLog(`Failed to create folder: ${(err as Error).message}`, 'error')
    } finally {
      setCreatingFolder(false)
    }
  }

  const handleCancelNewFolder = () => {
    setShowNewFolder(false)
    setNewFolderName('')
  }

  const handleNewFolderKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      handleConfirmNewFolder()
    } else if (e.key === 'Escape') {
      handleCancelNewFolder()
    }
  }

  const filterTree = (nodes: AssetTreeNode[], query: string): AssetTreeNode[] => {
    const lower = query.toLowerCase()
    return nodes.reduce((acc: AssetTreeNode[], node) => {
      if (node.isDirectory) {
        const filteredChildren = filterTree(node.children, query)
        if (filteredChildren.length > 0 || node.name.toLowerCase().includes(lower)) {
          acc.push({ ...node, children: filteredChildren })
        }
      } else if (node.name.toLowerCase().includes(lower)) {
        acc.push(node)
      }
      return acc
    }, [])
  }

  const displayData = filter ? filterTree(treeData, filter) : treeData

  // ── Render ────────────────────────────────────────────────────────
  return (
    <div className="asset-tree-panel">
      <div className="panel-header">Asset Browser</div>

      {/* Toolbar */}
      <div className="tree-toolbar">
        <button className="btn btn-sm" onClick={handleImportFile} title="Import files or folders into selected folder">
          📥 Import
        </button>
        <button className="btn btn-sm" onClick={handleStartNewFolder} title="Create new folder">
          🗂 New Folder
        </button>
      </div>

      {/* New Folder inline input */}
      {showNewFolder && (
        <div className="new-folder-row">
          <input
            ref={newFolderInputRef}
            className="new-folder-input"
            type="text"
            placeholder="Folder name..."
            value={newFolderName}
            onChange={e => setNewFolderName(e.target.value)}
            onKeyDown={handleNewFolderKeyDown}
            disabled={creatingFolder}
          />
          <button className="btn btn-sm btn-primary" onClick={handleConfirmNewFolder} disabled={creatingFolder || !newFolderName.trim()}>
            ✓
          </button>
          <button className="btn btn-sm" onClick={handleCancelNewFolder} disabled={creatingFolder}>
            ✗
          </button>
        </div>
      )}

      {/* Import conflict modal */}
      {importModal.visible && (
        <div className="import-modal-overlay">
          <div className="import-modal">
            <div className="import-modal-title">Import Sources</div>
            <div className="import-modal-meta">
              <div>Selected: <strong>{importModal.sources.length} item(s)</strong></div>
              <div>Target: <strong>{buildTargetLabel(importModal.targetDir)}</strong></div>
            </div>

            <div className="import-modal-hint">
              Selected files and folders will be copied into the target location. Uncheck any conflicts you do not want to overwrite.
            </div>

            <div className="import-modal-list">
              {importModal.sources.map(source => (
                <div key={source.sourcePath} className="import-modal-item">
                  <span className="import-modal-item-path">{buildSourceLabel(source)}</span>
                  <span className="import-modal-item-path">{source.sourcePath}</span>
                </div>
              ))}
            </div>

            <div className="import-modal-hint">Conflicts</div>
            <div className="import-modal-list">
              {importModal.conflicts.map(targetPath => (
                <label key={targetPath} className="import-modal-item">
                  <input
                    type="checkbox"
                    checked={importModal.selections[targetPath] ?? false}
                    onChange={() => handleToggleConflict(targetPath)}
                    disabled={importModal.submitting}
                  />
                  <span className="import-modal-item-path">{targetPath}</span>
                </label>
              ))}
            </div>

            <div className="import-modal-actions">
              <button className="btn btn-sm" onClick={handleCloseImportModal} disabled={importModal.submitting}>
                Cancel
              </button>
              <button className="btn btn-sm btn-primary" onClick={handleConfirmImportModal} disabled={importModal.submitting}>
                {importModal.submitting ? 'Importing...' : 'Import Selected'}
              </button>
            </div>
          </div>
        </div>
      )}

      {actionModal.visible && actionModal.kind && actionModal.node && (
        <div className="import-modal-overlay">
          <div className="import-modal asset-action-modal">
            <div className="import-modal-title">
              {actionModal.kind === 'rename' ? 'Rename Node' : 'Create Script Asset'}
            </div>
            <div className="import-modal-meta">
              <div>Target: <strong>{actionModal.node.path || 'Assets'}</strong></div>
            </div>
            <div className="import-modal-hint">
              {actionModal.kind === 'rename'
                ? 'Enter the new file or folder name.'
                : 'Enter a script file name. .txt will be added automatically if omitted.'}
            </div>
            <input
              className="filter-input asset-action-input"
              type="text"
              value={actionModal.value}
              autoFocus
              onChange={e => setActionModal(prev => ({ ...prev, value: e.target.value }))}
              onKeyDown={e => {
                if (e.key === 'Enter') {
                  void submitActionModal()
                } else if (e.key === 'Escape') {
                  closeActionModal()
                }
              }}
              disabled={actionModal.submitting}
            />
            <div className="import-modal-actions">
              <button className="btn btn-sm" onClick={closeActionModal} disabled={actionModal.submitting}>
                Cancel
              </button>
              <button className="btn btn-sm btn-primary" onClick={submitActionModal} disabled={actionModal.submitting || !actionModal.value.trim()}>
                {actionModal.submitting ? 'Working...' : 'OK'}
              </button>
            </div>
          </div>
        </div>
      )}

      <input
        className="filter-input"
        type="text"
        placeholder="🔍 Filter..."
        value={filter}
        onChange={e => setFilter(e.target.value)}
      />

      <div className="tree-scroll">
        {loading ? (
          <div className="tree-loading">Loading...</div>
        ) : displayData.length === 0 ? (
          <div className="tree-empty">No assets found.</div>
        ) : (
          displayData.map(root => (
            <TreeNodeItem
              key={root.path}
              node={root}
              depth={0}
              onSelect={onSelect}
              selectedPath={selectedNode ? normalizeAssetPath(selectedNode.path) : null}
              onContextMenu={handleContextMenu}
            />
          ))
        )}
      </div>

      {contextMenu.visible && contextMenu.node && (
        <div
          ref={contextMenuRef}
          className="context-menu"
          style={{ left: contextMenu.x, top: contextMenu.y }}
          onPointerDown={e => e.stopPropagation()}
        >
          {contextMenu.node.isDirectory && (
            <button type="button" className="context-menu-item" onPointerDown={handleContextMenuItem(handleCreateScriptAsset)}>
              Add Script
            </button>
          )}
          <button type="button" className="context-menu-item" onPointerDown={handleContextMenuItem(handleRenameNode)}>
            Rename
          </button>
          <button type="button" className="context-menu-item danger" onPointerDown={handleContextMenuItem(handleDeleteNode)}>
            Delete {contextMenu.node.isDirectory ? 'Directory' : 'File'}
          </button>
        </div>
      )}
    </div>
  )
}
