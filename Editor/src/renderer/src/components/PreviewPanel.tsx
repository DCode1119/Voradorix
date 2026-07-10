import { useState, useEffect } from 'react'
import { AssetTreeNode, AssetEntry } from '../types'

// ── Helpers ────────────────────────────────────────────────────────

function extToType(ext: string): 'texture' | 'font' | 'script' | null {
  switch (ext) {
    case '.png':
    case '.jpg':
    case '.jpeg': return 'texture'
    case '.ttf':
    case '.otf':  return 'font'
    case '.txt':  return 'script'
    default:      return null
  }
}

// ── Props ──────────────────────────────────────────────────────────

interface PreviewPanelProps {
  node: AssetTreeNode | null
  onLog: (msg: string, type: 'info' | 'success' | 'error') => void
  onAssetUpdate: () => void
}

// ── PreviewPanel ───────────────────────────────────────────────────

export default function PreviewPanel({ node, onLog, onAssetUpdate }: PreviewPanelProps) {
  const [previewData, setPreviewData] = useState<string | null>(null)
  const [previewType, setPreviewType] = useState<'image' | 'text' | 'font' | 'none'>('none')
  const [alias, setAlias] = useState('')

  // Load preview content
  useEffect(() => {
    if (!node || node.isDirectory) {
      setPreviewData(null)
      setPreviewType('none')
      return
    }

    setAlias(node.assetEntry?.alias ?? '')

    const ext = node.extension
    if (ext === '.png' || ext === '.jpg' || ext === '.jpeg') {
      setPreviewType('image')
      window.electronAPI.readFileBase64(node.path).then(data => {
        setPreviewData(data)
      })
    } else if (ext === '.ttf' || ext === '.otf') {
      setPreviewType('font')
      window.electronAPI.readFileBase64(node.path).then(data => {
        setPreviewData(data)
      })
    } else if (ext === '.txt') {
      setPreviewType('text')
      window.electronAPI.readFileText(node.path).then(data => {
        setPreviewData(data)
      })
    } else {
      setPreviewType('none')
      setPreviewData(null)
    }
  }, [node])

  const handleImport = async () => {
    if (!node || node.isDirectory) return

    if (node.isRegistered) {
      onLog(`Already registered: ${node.name}`, 'info')
      return
    }

    const type = extToType(node.extension)
    if (!type) {
      onLog(`Unsupported file type: ${node.extension}`, 'error')
      return
    }

    // Register existing file directly (no copy needed — already in Assets/)
    const importResult = await window.electronAPI.registerAsset(node.path, type)
    if ('error' in importResult) {
      onLog(`Import failed: ${importResult.error}`, 'error')
      return
    }

    onLog(`Imported: ${node.name} (GUID: ${importResult.guid})`, 'success')
    onAssetUpdate()
  }

  const handleDelete = async () => {
    if (!node || !node.assetEntry) return
    try {
      await window.electronAPI.deleteAsset(node.assetEntry.guid)
      onLog(`Deleted from registry: ${node.name}`, 'success')
      onAssetUpdate()
    } catch (err) {
      onLog(`Delete failed: ${(err as Error).message}`, 'error')
    }
  }

  const handleAliasSave = async () => {
    if (!node?.assetEntry) return
    try {
      await window.electronAPI.updateAlias(node.assetEntry.guid, alias || null)
      onLog(`Alias updated for ${node.name}`, 'success')
      onAssetUpdate()
    } catch (err) {
      onLog(`Alias update failed: ${(err as Error).message}`, 'error')
    }
  }

  // ── Type-specific preview renderers ───────────────────────────────

  const renderImagePreview = () => {
    if (!previewData) return <div className="preview-placeholder">Loading...</div>
    return (
      <div className="preview-image-wrapper">
        <img
          src={`data:image/${node?.extension === '.png' ? 'png' : 'jpeg'};base64,${previewData}`}
          alt={node?.name}
          className="preview-image"
        />
      </div>
    )
  }

  const renderFontPreview = () => {
    if (!previewData || !node) return <div className="preview-placeholder">Loading...</div>
    const fontFace = new FontFace('preview-font', `url(data:font/ttf;base64,${previewData})`)
    return (
      <div className="preview-font-wrapper">
        <style>{`@font-face { font-family: 'preview-font'; src: url(data:font/ttf;base64,${previewData}); }`}</style>
        <div className="font-sample" style={{ fontFamily: 'preview-font, sans-serif' }}>
          <div className="font-sample-large">Aa 가나다 123</div>
          <div className="font-sample-info">{node.name}</div>
        </div>
      </div>
    )
  }

  const renderTextPreview = () => {
    if (previewData === null) return <div className="preview-placeholder">Loading...</div>
    return (
      <pre className="preview-text">{previewData}</pre>
    )
  }

  // ── Render ────────────────────────────────────────────────────────

  if (!node || node.isDirectory) {
    return (
      <div className="preview-panel">
        <div className="preview-placeholder">
          {node?.isDirectory ? '📁 Select a file to preview' : 'Select a file to preview'}
        </div>
      </div>
    )
  }

  return (
    <div className="preview-panel">
      {/* Preview area */}
      <div className="preview-area">
        {previewType === 'image' && renderImagePreview()}
        {previewType === 'font' && renderFontPreview()}
        {previewType === 'text' && renderTextPreview()}
        {previewType === 'none' && (
          <div className="preview-placeholder">No preview available</div>
        )}
      </div>

      {/* Metadata */}
      <div className="metadata-panel">
        <div className="metadata-row">
          <span className="metadata-label">Name:</span>
          <span className="metadata-value">{node.name}</span>
        </div>
        <div className="metadata-row">
          <span className="metadata-label">Type:</span>
          <span className="metadata-value">{node.assetEntry?.type ?? 'unknown'}</span>
        </div>
        <div className="metadata-row">
          <span className="metadata-label">GUID:</span>
          <span className="metadata-value mono">{node.assetEntry?.guid ?? '-'}</span>
        </div>
        <div className="metadata-row">
          <span className="metadata-label">Alias:</span>
          <span className="metadata-value">
            <input
              className="alias-input"
              type="text"
              value={alias}
              onChange={e => setAlias(e.target.value)}
              placeholder="(no alias)"
              onBlur={handleAliasSave}
              onKeyDown={e => { if (e.key === 'Enter') handleAliasSave() }}
            />
          </span>
        </div>
        <div className="metadata-row">
          <span className="metadata-label">Source:</span>
          <span className="metadata-value mono small">{node.path}</span>
        </div>
        <div className="metadata-row">
          <span className="metadata-label">Size:</span>
          <span className="metadata-value">{node.size > 1024 ? `${(node.size / 1024).toFixed(1)} KB` : `${node.size} B`}</span>
        </div>
      </div>

      {/* Action bar */}
      <div className="action-bar">
        {!node.isRegistered && (
          <button className="btn btn-primary" onClick={handleImport}>
            Import
          </button>
        )}
        {node.isRegistered && node.assetEntry && (
          <button className="btn btn-danger" onClick={handleDelete}>
            Delete from Registry
          </button>
        )}
      </div>
    </div>
  )
}
