import { BrowserWindow } from 'electron'
import { spawn, type ChildProcess } from 'child_process'
import { existsSync } from 'fs'
import { dirname, join, resolve } from 'path'

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

let gameProcess: ChildProcess | null = null

function sendGameStatus(payload: GameStatusPayload): void {
  for (const window of BrowserWindow.getAllWindows()) {
    window.webContents.send('game:status', payload)
  }
}

function looksLikeEditorRoot(dir: string): boolean {
  return existsSync(join(dir, 'package.json')) && existsSync(join(dir, 'src'))
}

function findEditorRoot(): string {
  const candidates = [process.cwd(), __dirname, resolve(__dirname, '..'), resolve(__dirname, '..', '..')]

  for (const candidate of candidates) {
    let current = candidate
    for (let depth = 0; depth < 4; depth++) {
      if (looksLikeEditorRoot(current)) {
        return current
      }

      const parent = dirname(current)
      if (parent === current) {
        break
      }

      current = parent
    }
  }

  return resolve(__dirname, '..', '..')
}

function getEditorBuildExePath(): string {
  return join(findEditorRoot(), 'build', 'Voradorix.exe')
}

function getFallbackExePath(): string | null {
  const gameRoot = resolve(findEditorRoot(), '..')
  const fallbacks = [
    join(gameRoot, 'bin', 'Debug', 'Voradorix.exe'),
    join(gameRoot, 'bin', 'Release', 'Voradorix.exe')
  ]

  for (const candidate of fallbacks) {
    if (existsSync(candidate)) {
      return candidate
    }
  }

  return null
}

function resolveExePath(): string | null {
  const editorBuildExe = getEditorBuildExePath()
  if (existsSync(editorBuildExe)) {
    return editorBuildExe
  }

  return getFallbackExePath()
}

export function launchGame(): GameLaunchResult {
  if (gameProcess) {
    return { success: true, status: 'already-running', exePath: gameProcess.spawnfile }
  }

  const exePath = resolveExePath()
  if (!exePath) {
    const message = 'Voradorix.exe not found. Build the engine first.'
    sendGameStatus({ isRunning: false, message })
    return { success: false, error: message }
  }

  const gameRoot = resolve(findEditorRoot(), '..')

  try {
    gameProcess = spawn(exePath, [], {
      cwd: gameRoot,
      detached: false,
      windowsHide: false,
      stdio: 'ignore'
    })

    gameProcess.on('exit', () => {
      gameProcess = null
      sendGameStatus({ isRunning: false, message: 'Game process exited.' })
    })

    gameProcess.on('error', (err) => {
      gameProcess = null
      sendGameStatus({ isRunning: false, message: `Game launch failed: ${err.message}` })
    })

    sendGameStatus({ isRunning: true, message: `Game launched: ${exePath.replace(/\\/g, '/')}` })

    return { success: true, status: 'started', exePath }
  } catch (err) {
    const message = (err as Error).message
    sendGameStatus({ isRunning: false, message: `Game launch failed: ${message}` })
    return { success: false, error: message }
  }
}

export function stopGame(): GameStopResult {
  if (!gameProcess) {
    sendGameStatus({ isRunning: false, message: 'Game process is not running.' })
    return { success: true, status: 'not-running' }
  }

  try {
    const processToStop = gameProcess
    gameProcess = null
    processToStop.kill()
    sendGameStatus({ isRunning: false, message: 'Game process stopped.' })

    return { success: true, status: 'stopped' }
  } catch (err) {
    const message = (err as Error).message
    sendGameStatus({ isRunning: false, message: `Game stop failed: ${message}` })
    return { success: false, error: message }
  }
}
