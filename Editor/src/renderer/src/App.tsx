import { useState, useCallback, useEffect, useRef } from 'react'
import AssetTree from './components/AssetTree'
import PreviewPanel from './components/PreviewPanel'
import LogPanel from './components/LogPanel'
import { AssetTreeNode, LogMessage } from './types'
import './App.css'

let logIdCounter = 0

export default function App() {
  const [selectedNode, setSelectedNode] = useState<AssetTreeNode | null>(null)
  const [refreshTrigger, setRefreshTrigger] = useState(0)
  const [messages, setMessages] = useState<LogMessage[]>([])
  const [dragOver, setDragOver] = useState(false)
  const [gameRunning, setGameRunning] = useState(false)
  const [gameStatusText, setGameStatusText] = useState('Game stopped')
  const dragCounter = useRef(0)

  const addLog = useCallback((text: string, type: 'info' | 'success' | 'error' = 'info') => {
    setMessages(prev => [...prev, { id: ++logIdCounter, text, type }])
  }, [])

  const handleSelect = useCallback((node: AssetTreeNode | null) => {
    setSelectedNode(node)
  }, [])

  const handleAssetUpdate = useCallback(() => {
    setRefreshTrigger(prev => prev + 1)
  }, [])

  const handlePlay = useCallback(async () => {
    const result = await window.electronAPI.launchGame()
    if (!result.success) {
      addLog(`Play failed: ${result.error ?? 'unknown error'}`, 'error')
      return
    }

    setGameRunning(true)
    setGameStatusText(result.status === 'already-running' ? 'Game already running' : 'Game running')
    addLog(result.status === 'already-running' ? 'Game is already running.' : 'Game launched.', 'success')
  }, [addLog])

  const handleStop = useCallback(async () => {
    const result = await window.electronAPI.stopGame()
    if (!result.success) {
      addLog(`Stop failed: ${result.error ?? 'unknown error'}`, 'error')
      return
    }

    setGameRunning(false)
    setGameStatusText(result.status === 'not-running' ? 'Game already stopped' : 'Game stopped')
    addLog(result.status === 'not-running' ? 'Game was not running.' : 'Game stopped.', 'info')
  }, [addLog])

  useEffect(() => {
    return window.electronAPI.onGameStatusChanged(status => {
      setGameRunning(status.isRunning)
      setGameStatusText(status.message)
    })
  }, [])

  // ── Drag & Drop ──────────────────────────────────────────────────

  const importDroppedFile = useCallback(async (filePath: string) => {
    const result = await window.electronAPI.importFile(filePath)
    if (result.error) {
      addLog(`Drop import failed: ${result.error}`, 'error')
      return
    }
    addLog(`Imported via drop: ${result.sourcePath}`, 'success')
    handleAssetUpdate()
  }, [addLog, handleAssetUpdate])

  useEffect(() => {
    const handleDragEnter = (e: DragEvent) => {
      e.preventDefault()
      e.stopPropagation()
      dragCounter.current++
      if (dragCounter.current === 1) {
        setDragOver(true)
      }
    }

    const handleDragLeave = (e: DragEvent) => {
      e.preventDefault()
      e.stopPropagation()
      dragCounter.current--
      if (dragCounter.current <= 0) {
        dragCounter.current = 0
        setDragOver(false)
      }
    }

    const handleDragOver = (e: DragEvent) => {
      e.preventDefault()
      e.stopPropagation()
    }

    const handleDrop = async (e: DragEvent) => {
      e.preventDefault()
      e.stopPropagation()
      setDragOver(false)
      dragCounter.current = 0

      const files = e.dataTransfer?.files
      if (!files || files.length === 0) return

      for (let i = 0; i < files.length; i++) {
        const file = files[i]
        // Electron adds .path to File objects
        const path = (file as any).path
        if (path) {
          await importDroppedFile(path)
        }
      }
    }

    // Bind on window to catch drags from outside
    window.addEventListener('dragenter', handleDragEnter)
    window.addEventListener('dragleave', handleDragLeave)
    window.addEventListener('dragover', handleDragOver)
    window.addEventListener('drop', handleDrop)

    return () => {
      window.removeEventListener('dragenter', handleDragEnter)
      window.removeEventListener('dragleave', handleDragLeave)
      window.removeEventListener('dragover', handleDragOver)
      window.removeEventListener('drop', handleDrop)
    }
  }, [importDroppedFile])

  return (
    <div className="app-layout">
      {/* Drag overlay */}
      {dragOver && (
        <div className="drag-overlay">
          <div className="drag-message">📥 Drop files to import</div>
        </div>
      )}

      {/* Header */}
      <header className="app-header">
        <span className="app-title">Voradorix Editor</span>
        <div className="header-actions">
          <span className="header-status">{gameStatusText}</span>
          <button className="btn btn-play" title="Launch Game (Phase 3)" onClick={handlePlay} disabled={gameRunning}>
            ▶ Play
          </button>
          <button className="btn btn-stop" title="Stop Game (Phase 3)" onClick={handleStop} disabled={!gameRunning}>
            ■ Stop
          </button>
        </div>
      </header>

      {/* Main content */}
      <div className="app-body">
        <AssetTree
          onSelect={handleSelect}
          onLog={addLog}
          refreshTrigger={refreshTrigger}
          selectedNode={selectedNode}
        />
        <PreviewPanel
          node={selectedNode}
          onLog={addLog}
          onAssetUpdate={handleAssetUpdate}
        />
      </div>

      {/* Log */}
      <LogPanel messages={messages} />
    </div>
  )
}
