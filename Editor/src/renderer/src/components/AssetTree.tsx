import { useState, useEffect, useCallback, useRef } from 'react'
import { AssetTreeNode, FileEntry, AssetEntry } from '../types'

// ── Props ──────────────────────────────────────────────────────────

interface AssetTreeProps {
  onSelect: (node: AssetTreeNode | null) => void
  onLog: (msg: string, type: 'info' | 'success' | 'error') => void
  refreshTrigger: number
  selectedNode: AssetTreeNode | null
}

// ── 디렉토리 구조 스캔 (재귀) ──────────────────────────────────────

async function scanDirectory(
  relativePath: string,
  registryAssets: AssetEntry[]
): Promise<AssetTreeNode[]> {
  const entries: FileEntry[] = await window.electronAPI.readDirectory(relativePath)
  const nodes: AssetTreeNode[] = []

  for (const e of entries) {
    const match = registryAssets.find(a => a.sourcePath === e.path)
    const node: AssetTreeNode = {
      name: e.name,
      path: e.path,
      isDirectory: e.isDirectory,
      children: [],
      extension: e.extension,
      size: e.size,
      isRegistered: e.isRegistered,
      assetEntry: match || undefined
    }

    if (e.isDirectory) {
      node.children = await scanDirectory(e.path, registryAssets)
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
  onMove,
  selectedPath
}: {
  node: AssetTreeNode
  depth: number
  onSelect: (node: AssetTreeNode) => void
  onMove: (srcPath: string, dstDirPath: string) => void
  selectedPath: string | null
}) {
  const [expanded, setExpanded] = useState(depth < 1)
  const [dragOver, setDragOver] = useState(false)

  const handleClick = () => {
    if (!node.isDirectory) {
      onSelect(node)
    }
  }

  const handleToggle = () => {
    if (node.isDirectory) {
      setExpanded(prev => !prev)
      onSelect(node)
    }
  }

  // ── Drag source (files only) ──────────────────────────────────────
  const handleDragStart = (e: React.DragEvent) => {
    if (node.isDirectory) return
    e.dataTransfer.setData('text/x-voradorix-asset-path', node.path)
    e.dataTransfer.effectAllowed = 'move'
  }

  // ── Drop target (directories only) ────────────────────────────────
  const handleDragOver = (e: React.DragEvent) => {
    if (!node.isDirectory) return
    // Only accept internal drags
    if (!e.dataTransfer.types.includes('text/x-voradorix-asset-path')) return
    e.preventDefault()
    e.dataTransfer.dropEffect = 'move'
    setDragOver(true)
  }

  const handleDragLeave = (e: React.DragEvent) => {
    if (!node.isDirectory) return
    setDragOver(false)
  }

  const handleDrop = (e: React.DragEvent) => {
    setDragOver(false)
    if (!node.isDirectory) return

    const srcPath = e.dataTransfer.getData('text/x-voradorix-asset-path')
    if (!srcPath || srcPath === node.path) return

    // Prevent dropping onto own parent chain
    const srcParts = srcPath.replace(/\\/g, '/').split('/')
    const dstParts = node.path.replace(/\\/g, '/').split('/')
    if (srcParts[0] === dstParts[0] && srcParts.length === 1) {
      // Same root directory level, skip
      return
    }

    onMove(srcPath, node.path)
  }

  const isSelected = selectedPath === node.path

  const classes = [
    'tree-item',
    isSelected ? 'selected' : '',
    node.isDirectory ? 'directory' : 'file',
    node.isRegistered ? 'registered' : 'unregistered',
    dragOver ? 'drag-over' : ''
  ].filter(Boolean).join(' ')

  return (
    <div>
      <div
        className={classes}
        style={{ paddingLeft: `${depth * 16 + 8}px` }}
        onClick={node.isDirectory ? handleToggle : handleClick}
        onDoubleClick={node.isDirectory ? undefined : handleClick}
        draggable={!node.isDirectory}
        onDragStart={handleDragStart}
        onDragOver={handleDragOver}
        onDragLeave={handleDragLeave}
        onDrop={handleDrop}
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
        <span className="tree-status">{node.isRegistered ? '🟢' : '⚪'}</span>
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
              onMove={onMove}
              selectedPath={selectedPath}
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

  const loadTree = useCallback(async () => {
    setLoading(true)
    try {
      const registry = await window.electronAPI.readAssetRegistry()
      const roots = await scanDirectory('', registry.assets)
      setTreeData(roots)
    } catch (err) {
      onLog(`Failed to load asset tree: ${(err as Error).message}`, 'error')
    } finally {
      setLoading(false)
    }
  }, [onLog])

  useEffect(() => {
    loadTree()
  }, [loadTree, refreshTrigger])

  // Focus input when New Folder mode activates
  useEffect(() => {
    if (showNewFolder) {
      newFolderInputRef.current?.focus()
    }
  }, [showNewFolder])

  // ── Import External File ──────────────────────────────────────────
  const handleImportFile = async () => {
    const result = await window.electronAPI.importExternal()
    if (result.error) {
      if (result.error !== 'Canceled') {
        onLog(`Import failed: ${result.error}`, 'error')
      }
      return
    }
    onLog(`Imported: ${result.sourcePath} (GUID: ${result.guid})`, 'success')
    loadTree()
  }

  // ── Create New Folder ─────────────────────────────────────────────
  const handleStartNewFolder = () => {
    setNewFolderName('')
    setShowNewFolder(true)
  }

  const handleConfirmNewFolder = async () => {
    const name = newFolderName.trim()
    if (!name) return

    setCreatingFolder(true)
    try {
      const result = await window.electronAPI.createDirectory(name)
      if (result.success) {
        onLog(`Created folder: ${name}`, 'success')
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

  // ── Filter ────────────────────────────────────────────────────────
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
        <button className="btn btn-sm" onClick={handleImportFile} title="Import file from outside project">
          📥 Import File
        </button>
        <button className="btn btn-sm" onClick={handleStartNewFolder} title="Create new folder">
          📁 New Folder
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
              selectedPath={selectedNode?.path ?? null}
            />
          ))
        )}
      </div>
    </div>
  )
}
