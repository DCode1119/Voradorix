import { useCallback, useEffect, useLayoutEffect, useMemo, useRef, useState, type KeyboardEvent as ReactKeyboardEvent } from 'react'
import { AssetTreeNode } from '../types'

interface ScriptEditorPanelProps {
  node: AssetTreeNode
  onLog: (msg: string, type: 'info' | 'success' | 'error') => void
  onAssetUpdate?: () => void
}

type IssueLevel = 'error' | 'warning'

interface ValidationIssue {
  line: number
  level: IssueLevel
  message: string
}

interface LabelEntry {
  name: string
  line: number
}

interface JumpEntry {
  target: string
  line: number
  resolved: boolean
}

interface ChoiceEntry {
  target: string
  line: number
  text: string
  resolved: boolean
}

interface BlockInfo {
  id: string
  label: string
  startLine: number
  endLine: number
  outgoing: string[]
  hasExplicitTransfer: boolean
}

interface FlowFinding {
  line: number
  level: IssueLevel
  kind: 'unreachable' | 'loop'
  message: string
}

interface AutocompleteItem {
  command: string
  label: string
  detail: string
  snippet: string
  cursorOffset: number
}

const KNOWN_COMMANDS = new Set(['bg', 'show', 'hide', 'pose', 'wait', 'label', 'jump', 'dialogue', 'choice'])

const AUTOCOMPLETE_ITEMS: AutocompleteItem[] = [
  { command: 'bg', label: '@bg', detail: '배경 지정', snippet: '@bg ""', cursorOffset: 5 },
  { command: 'show', label: '@show', detail: '캐릭터 표시', snippet: '@show ""', cursorOffset: 7 },
  { command: 'hide', label: '@hide', detail: '캐릭터 숨김', snippet: '@hide ""', cursorOffset: 7 },
  { command: 'pose', label: '@pose', detail: '포즈 변경', snippet: '@pose "" ""', cursorOffset: 7 },
  { command: 'wait', label: '@wait', detail: '대기(초)', snippet: '@wait ', cursorOffset: 6 },
  { command: 'label', label: '@label', detail: '레이블 정의', snippet: '@label ""', cursorOffset: 8 },
  { command: 'jump', label: '@jump', detail: '레이블 이동', snippet: '@jump ""', cursorOffset: 7 },
  { command: 'dialogue', label: '@dialogue', detail: '대사 출력', snippet: '@dialogue ""', cursorOffset: 11 },
  { command: 'choice', label: '@choice', detail: '선택지 분기', snippet: '@choice "" ""', cursorOffset: 9 },
]

function splitLines(text: string): string[] {
  return text.length > 0 ? text.split(/\r?\n/) : ['']
}

function lineStarts(text: string): number[] {
  const starts = [0]
  for (let i = 0; i < text.length; i++) {
    if (text[i] === '\n') {
      starts.push(i + 1)
    }
  }
  return starts
}

function visibleLineNumbers(lines: string[]): Array<number | null> {
  let count = 0
  return lines.map(line => {
    if (line.trim() === '') {
      return null
    }
    const value = count
    count += 1
    return value
  })
}

function blockColor(index: number): string {
  const palette = [
    '#ff6b6b', '#4dabf7', '#51cf66', '#ffd43b',
    '#9775fa', '#ff922b', '#f06595', '#20c997',
    '#74c0fc', '#c0eb75', '#da77f2', '#ffa8a8'
  ]
  return palette[index % palette.length]
}

function getAutocompleteContext(text: string, caret: number) {
  const before = text.slice(0, caret)
  const match = before.match(/(?:^|[\r\n])\s*@([A-Za-z_]*)$/)
  if (!match) return null

  const prefix = match[1].toLowerCase()
  const start = caret - match[1].length - 1
  return { start, prefix }
}

function getQuotedValues(line: string): string[] {
  return Array.from(line.matchAll(/"([^"]*)"/g), match => match[1])
}

function parseScript(text: string) {
  const lines = splitLines(text)
  const labels: LabelEntry[] = []
  const jumps: JumpEntry[] = []
  const choices: ChoiceEntry[] = []
  const issues: ValidationIssue[] = []
  const commands = new Map<string, number>()
  const labelLookup = new Map<string, number>()

  const labelLineIndexes: Array<{ name: string; lineIndex: number }> = []

  lines.forEach((line, index) => {
    const lineNumber = index
    const commandMatch = line.match(/^\s*@([A-Za-z_][\w]*)/)
    if (!commandMatch) return

    const command = commandMatch[1].toLowerCase()
    commands.set(command, (commands.get(command) ?? 0) + 1)

    if (!KNOWN_COMMANDS.has(command)) {
      issues.push({ line: lineNumber, level: 'warning', message: `알 수 없는 명령어 @${command}` })
      return
    }

    if (command === 'label') {
      const labelMatch = line.match(/^\s*@label\s+"([^"]+)"\s*$/)
      if (!labelMatch) {
        issues.push({ line: lineNumber, level: 'error', message: '@label 형식이 올바르지 않습니다.' })
        return
      }

      const name = labelMatch[1]
      labels.push({ name, line: lineNumber })
      labelLineIndexes.push({ name, lineIndex: index })
      if (labelLookup.has(name)) {
        issues.push({ line: lineNumber, level: 'warning', message: `중복 레이블 "${name}"` })
      }
      labelLookup.set(name, lineNumber)
      return
    }

    if (command === 'jump') {
      const jumpMatch = line.match(/^\s*@jump\s+"([^"]+)"\s*$/)
      if (!jumpMatch) {
        issues.push({ line: lineNumber, level: 'error', message: '@jump 형식이 올바르지 않습니다.' })
        return
      }

      jumps.push({ target: jumpMatch[1], line: lineNumber, resolved: false })
      return
    }

    if (command === 'choice') {
      const options = getQuotedValues(line)
      if (options.length < 2 || options.length % 2 !== 0) {
        issues.push({ line: lineNumber, level: 'warning', message: '@choice는 텍스트/레이블 쌍이 필요합니다.' })
      } else {
        for (let i = 1; i < options.length; i += 2) {
          choices.push({ text: options[i - 1], target: options[i], line: lineNumber, resolved: false })
        }
      }
    }
  })

  jumps.forEach(jump => {
    jump.resolved = labelLookup.has(jump.target)
    if (!jump.resolved) {
      issues.push({ line: jump.line, level: 'error', message: `존재하지 않는 레이블 "${jump.target}"` })
    }
  })

  choices.forEach(choice => {
    choice.resolved = labelLookup.has(choice.target)
    if (!choice.resolved) {
      issues.push({ line: choice.line, level: 'error', message: `존재하지 않는 선택지 레이블 "${choice.target}"` })
    }
  })

  const blockStarts: Array<{ label: string; startIndex: number; startLine: number }> = [
    { label: '__entry__', startIndex: 0, startLine: 0 },
    ...labelLineIndexes.map(item => ({ label: item.name, startIndex: item.lineIndex, startLine: item.lineIndex }))
  ]

  const blocks: BlockInfo[] = blockStarts.map((block, index) => {
    const nextStart = blockStarts[index + 1]?.startIndex ?? lines.length
    return {
      id: block.label,
      label: block.label === '__entry__' ? 'Entry' : block.label,
      startLine: block.startLine,
      endLine: nextStart > 0 ? nextStart : lines.length,
      outgoing: [],
      hasExplicitTransfer: false
    }
  })

  blocks.forEach((block, index) => {
    const startIdx = block.id === '__entry__' ? 0 : (labelLineIndexes.find(entry => entry.name === block.id)?.lineIndex ?? 0)
    const nextBlock = blocks[index + 1]
    for (let lineIndex = startIdx; lineIndex < Math.min(block.endLine, lines.length); lineIndex++) {
      const line = lines[lineIndex]
      const jumpMatch = line.match(/^\s*@jump\s+"([^"]+)"\s*$/)
      if (jumpMatch) {
        block.outgoing.push(jumpMatch[1])
        block.hasExplicitTransfer = true
        break
      }

      const choiceValues = getQuotedValues(line)
      if (line.match(/^\s*@choice\b/) && choiceValues.length >= 2 && choiceValues.length % 2 === 0) {
        for (let i = 1; i < choiceValues.length; i += 2) {
          block.outgoing.push(choiceValues[i])
        }
        block.hasExplicitTransfer = true
        break
      }
    }

    if (!block.hasExplicitTransfer && nextBlock) {
      block.outgoing.push(nextBlock.id)
    }
  })

  const adjacency = new Map<string, string[]>()
  blocks.forEach(block => {
    const resolvedTargets = block.outgoing.filter(target => target === '__entry__' || blocks.some(candidate => candidate.id === target))
    adjacency.set(block.id, resolvedTargets)
  })

  const reachable = new Set<string>()
  const visit = (id: string) => {
    if (reachable.has(id)) return
    reachable.add(id)
    for (const next of adjacency.get(id) ?? []) {
      visit(next)
    }
  }
  visit('__entry__')

  const unreachableBlocks = blocks.filter(block => block.id !== '__entry__' && !reachable.has(block.id))
  unreachableBlocks.forEach(block => {
    issues.push({
      line: block.startLine,
      level: 'error',
      message: `도달 불가 블럭: ${block.label} (L${block.startLine}~L${block.endLine})`
    })
  })

  const blockById = new Map(blocks.map(block => [block.id, block]))
  const loopFindings: FlowFinding[] = []

  const cycles = new Map<string, string[]>()
  const visited = new Set<string>()
  const stack: string[] = []
  const onStack = new Set<string>()

  const recordCycle = (start: string, target: string) => {
    const startIndex = stack.lastIndexOf(target)
    if (startIndex < 0) return
    const members = stack.slice(startIndex)
    const key = members.slice().sort().join('|')
    if (!cycles.has(key)) {
      cycles.set(key, members)
    }
  }

  const walk = (id: string) => {
    visited.add(id)
    stack.push(id)
    onStack.add(id)

    for (const next of adjacency.get(id) ?? []) {
      if (!visited.has(next)) {
        walk(next)
      } else if (onStack.has(next)) {
        recordCycle(id, next)
      }
    }

    stack.pop()
    onStack.delete(id)
  }

  if (adjacency.has('__entry__')) {
    walk('__entry__')
  }

  cycles.forEach(component => {
    const isClosed = component.every(id => (adjacency.get(id) ?? []).every(next => component.includes(next)))
    const isReachable = component.some(id => reachable.has(id) && id !== '__entry__')
    if (!isClosed || !isReachable) return

    const first = component.map(id => blockById.get(id)).find(Boolean)
    if (first) {
      loopFindings.push({
          line: first.startLine,
          level: 'warning',
          kind: 'loop',
          message: `잠재적 무한루프: ${first.label}를 포함한 폐쇄 사이클`
      })
    }
  })

  loopFindings.forEach(finding => {
    issues.push({ line: finding.line, level: finding.level, message: finding.message })
  })

  return {
    lines,
    labels,
    jumps,
    choices,
    issues,
    commands,
    lineStarts: lineStarts(text),
    blocks,
    unreachableBlocks,
    loopFindings
  }
}

export default function ScriptEditorPanel({ node, onLog, onAssetUpdate }: ScriptEditorPanelProps) {
  const textareaRef = useRef<HTMLTextAreaElement | null>(null)
  const codepaneRef = useRef<HTMLDivElement | null>(null)
  const autocompleteRef = useRef<HTMLDivElement | null>(null)
  const gutterRef = useRef<HTMLDivElement | null>(null)
  const saveTimerRef = useRef<number | null>(null)
  const pendingSelectionRef = useRef<{ start: number; end: number } | null>(null)
  const latestTextRef = useRef('')
  const [text, setText] = useState('')
  const [savedText, setSavedText] = useState('')
  const [isLoading, setIsLoading] = useState(true)
  const [saveState, setSaveState] = useState<'idle' | 'dirty' | 'saving' | 'saved' | 'error'>('idle')
  const [errorText, setErrorText] = useState<string | null>(null)
  const [autocomplete, setAutocomplete] = useState<{ open: boolean; start: number; prefix: string; index: number }>({
    open: false,
    start: 0,
    prefix: '',
    index: 0,
  })
  const [autocompletePosition, setAutocompletePosition] = useState({ left: 0, top: 0, maxHeight: 240, placement: 'bottom' as 'bottom' | 'top' })

  useEffect(() => {
    let alive = true
    setIsLoading(true)
    setErrorText(null)
    setSaveState('idle')
    setText('')
    setSavedText('')

    window.electronAPI.readFileText(node.path).then(content => {
      if (!alive) return
      if (content === null) {
        setErrorText('스크립트를 불러오지 못했습니다.')
        setIsLoading(false)
        setSaveState('error')
        return
      }

      setText(content)
      setSavedText(content)
      setIsLoading(false)
      setSaveState('saved')
    })

    return () => {
      alive = false
      if (saveTimerRef.current !== null) {
        window.clearTimeout(saveTimerRef.current)
        saveTimerRef.current = null
      }
    }
  }, [node.path])

  const script = useMemo(() => parseScript(text), [text])
  const displayLineNumbers = useMemo(() => visibleLineNumbers(script.lines), [script.lines])
  const blockBands = useMemo(() => {
    return script.blocks
      .map((block, index) => ({
        ...block,
        color: blockColor(index),
      }))
      .filter(block => block.endLine > block.startLine)
  }, [script.blocks])
  const isDirty = text !== savedText

  useEffect(() => {
    latestTextRef.current = text
  }, [text])

  useEffect(() => {
    const pending = pendingSelectionRef.current
    if (!pending || !textareaRef.current) return
    textareaRef.current.setSelectionRange(pending.start, pending.end)
    pendingSelectionRef.current = null
  }, [text])

  const filteredAutocomplete = useMemo(() => {
    if (!autocomplete.open) return []
    const prefix = autocomplete.prefix
    return AUTOCOMPLETE_ITEMS.filter(item => item.command.startsWith(prefix))
  }, [autocomplete.open, autocomplete.prefix])

  useEffect(() => {
    if (!autocomplete.open) return
    if (filteredAutocomplete.length === 0) {
      setAutocomplete(prev => ({ ...prev, open: false, index: 0 }))
      return
    }
    if (autocomplete.index >= filteredAutocomplete.length) {
      setAutocomplete(prev => ({ ...prev, index: 0 }))
    }
  }, [autocomplete.index, autocomplete.open, filteredAutocomplete.length])

  const syncGutterScroll = useCallback(() => {
    if (!textareaRef.current || !gutterRef.current) return
    gutterRef.current.scrollTop = textareaRef.current.scrollTop
  }, [])

  const updateAutocompletePosition = useCallback(() => {
    const textarea = textareaRef.current
    const codepane = codepaneRef.current
    const popup = autocompleteRef.current
    if (!textarea || !codepane || !popup || !autocomplete.open || filteredAutocomplete.length === 0) return

    const caret = textarea.selectionStart ?? 0
    const textBeforeCaret = text.slice(0, caret)
    const linesBeforeCaret = textBeforeCaret.split(/\r?\n/)
    const caretLine = linesBeforeCaret.length - 1
    const caretColumn = linesBeforeCaret[linesBeforeCaret.length - 1]?.length ?? 0

    const mirror = document.createElement('div')
    const style = window.getComputedStyle(textarea)
    mirror.style.position = 'fixed'
    mirror.style.left = '0'
    mirror.style.top = '0'
    mirror.style.visibility = 'hidden'
    mirror.style.whiteSpace = 'pre'
    mirror.style.font = style.font
    mirror.style.fontSize = style.fontSize
    mirror.style.fontFamily = style.fontFamily
    mirror.style.fontWeight = style.fontWeight
    mirror.style.fontStyle = style.fontStyle
    mirror.style.lineHeight = style.lineHeight
    mirror.style.letterSpacing = style.letterSpacing
    mirror.style.tabSize = style.tabSize
    mirror.style.padding = style.padding
    mirror.style.border = style.border
    mirror.style.boxSizing = style.boxSizing
    mirror.style.width = `${textarea.clientWidth}px`
    mirror.style.height = 'auto'
    mirror.style.overflow = 'hidden'
    mirror.textContent = textBeforeCaret

    const marker = document.createElement('span')
    marker.textContent = '\u200b'
    mirror.appendChild(marker)
    document.body.appendChild(mirror)

    const markerRect = marker.getBoundingClientRect()
    const mirrorRect = mirror.getBoundingClientRect()
    document.body.removeChild(mirror)

    const caretX = markerRect.left - mirrorRect.left
    const caretY = markerRect.top - mirrorRect.top
    const textareaRect = textarea.getBoundingClientRect()
    const codepaneRect = codepane.getBoundingClientRect()
    const lineHeight = parseFloat(style.lineHeight) || 20
    const popupWidth = popup.offsetWidth || 260
    const popupHeight = popup.offsetHeight || 240

    let left = textareaRect.left - codepaneRect.left + caretX - textarea.scrollLeft
    let top = textareaRect.top - codepaneRect.top + caretY - textarea.scrollTop + lineHeight + 6
    let placement: 'bottom' | 'top' = 'bottom'

    if (top + popupHeight > codepane.clientHeight - 8) {
      placement = 'top'
      top = textareaRect.top - codepaneRect.top + caretY - textarea.scrollTop - popupHeight - 8
    }

    left = Math.max(8, Math.min(left, codepane.clientWidth - popupWidth - 8))
    top = Math.max(8, Math.min(top, codepane.clientHeight - popupHeight - 8))

    setAutocompletePosition({ left, top, maxHeight: Math.max(120, codepane.clientHeight - top - 12), placement })
  }, [autocomplete.open, filteredAutocomplete.length, text])

  useLayoutEffect(() => {
    if (!autocomplete.open || filteredAutocomplete.length === 0) return
    updateAutocompletePosition()
  }, [autocomplete.open, autocomplete.index, filteredAutocomplete.length, text, updateAutocompletePosition])

  useEffect(() => {
    if (!autocomplete.open) return

    const handleResize = () => updateAutocompletePosition()
    window.addEventListener('resize', handleResize)
    return () => window.removeEventListener('resize', handleResize)
  }, [autocomplete.open, updateAutocompletePosition])

  const updateAutocomplete = useCallback((value: string, caret: number) => {
    const ctx = getAutocompleteContext(value, caret)
    if (!ctx) {
      setAutocomplete(prev => prev.open ? { ...prev, open: false, prefix: '' } : prev)
      return
    }

    setAutocomplete(prev => {
      const nextItems = AUTOCOMPLETE_ITEMS.filter(item => item.command.startsWith(ctx.prefix))
      const nextIndex = nextItems.length === 0 ? 0 : Math.min(prev.index, nextItems.length - 1)
      return {
        open: nextItems.length > 0,
        start: ctx.start,
        prefix: ctx.prefix,
        index: nextIndex,
      }
    })
  }, [])

  const closeAutocomplete = useCallback(() => {
    setAutocomplete(prev => prev.open ? { ...prev, open: false, prefix: '', index: 0 } : prev)
  }, [])

  const insertAutocomplete = useCallback((item: AutocompleteItem) => {
    const textarea = textareaRef.current
    if (!textarea) return

    const start = autocomplete.start
    const end = textarea.selectionStart ?? start
    const nextText = `${text.slice(0, start)}${item.snippet}${text.slice(end)}`
    const cursor = start + item.cursorOffset

    pendingSelectionRef.current = { start: cursor, end: cursor }
    setText(nextText)
    closeAutocomplete()
  }, [autocomplete.start, closeAutocomplete, text])

  const focusLine = useCallback((line: number) => {
    const textarea = textareaRef.current
    if (!textarea) return

    const lineIndex = Math.max(0, Math.min(line, script.lines.length - 1))
    const start = script.lineStarts[lineIndex] ?? 0
    const end = lineIndex + 1 < script.lineStarts.length ? script.lineStarts[lineIndex + 1] - 1 : text.length

    textarea.focus()
    textarea.setSelectionRange(start, end)
    textarea.scrollTop = Math.max(0, textarea.scrollHeight * (lineIndex / Math.max(1, script.lines.length)) - textarea.clientHeight / 2)
    syncGutterScroll()
  }, [script.lines.length, script.lineStarts, syncGutterScroll, text.length])

  const saveScript = useCallback(async (snapshot: string, reason: 'manual' | 'autosave') => {
    if (saveTimerRef.current !== null) {
      window.clearTimeout(saveTimerRef.current)
      saveTimerRef.current = null
    }

    setSaveState('saving')
    const result = await window.electronAPI.writeFileText(node.path, snapshot)
    if (!result.success) {
      setSaveState('error')
      setErrorText(result.error ?? '저장에 실패했습니다.')
      if (reason === 'manual') {
        onLog(`Script save failed: ${result.error ?? 'unknown error'}`, 'error')
      }
      return false
    }

    if (latestTextRef.current === snapshot) {
      setSavedText(snapshot)
      setSaveState('saved')
    } else {
      setSaveState('dirty')
    }
    setErrorText(null)
    if (reason === 'manual') {
      onLog(`Saved script: ${node.name}`, 'success')
      onAssetUpdate?.()
    }
    return true
  }, [node.name, node.path, onLog])

  useEffect(() => {
    if (isLoading || !isDirty) return

    setSaveState('dirty')
    saveTimerRef.current = window.setTimeout(() => {
      void saveScript(text, 'autosave')
    }, 800)

    return () => {
      if (saveTimerRef.current !== null) {
        window.clearTimeout(saveTimerRef.current)
        saveTimerRef.current = null
      }
    }
  }, [isDirty, isLoading, saveScript, text])

  const handleManualSave = useCallback(async () => {
    await saveScript(text, 'manual')
  }, [saveScript, text])

  const handleKeyDown = useCallback((event: ReactKeyboardEvent<HTMLTextAreaElement>) => {
    if (autocomplete.open && filteredAutocomplete.length > 0) {
      if (event.key === 'ArrowDown') {
        event.preventDefault()
        setAutocomplete(prev => ({ ...prev, index: (prev.index + 1) % filteredAutocomplete.length }))
        return
      }

      if (event.key === 'ArrowUp') {
        event.preventDefault()
        setAutocomplete(prev => ({ ...prev, index: (prev.index - 1 + filteredAutocomplete.length) % filteredAutocomplete.length }))
        return
      }

      if (event.key === 'Enter' || event.key === 'Tab') {
        event.preventDefault()
        insertAutocomplete(filteredAutocomplete[autocomplete.index] ?? filteredAutocomplete[0])
        return
      }
    }

    if (event.key === 'Escape' && autocomplete.open) {
      event.preventDefault()
      closeAutocomplete()
      return
    }

    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 's') {
      event.preventDefault()
      void handleManualSave()
    }
  }, [autocomplete.index, autocomplete.open, closeAutocomplete, filteredAutocomplete, handleManualSave, insertAutocomplete])

  const handleEditorChange = useCallback((value: string, caret: number) => {
    setText(value)
    updateAutocomplete(value, caret)
  }, [updateAutocomplete])

  const handleEditorClick = useCallback(() => {
    const textarea = textareaRef.current
    if (!textarea) return
    updateAutocomplete(textarea.value, textarea.selectionStart ?? 0)
  }, [updateAutocomplete])

  const handleSuggestionClick = useCallback((item: AutocompleteItem) => {
    insertAutocomplete(item)
  }, [insertAutocomplete])

  const handleEditorScroll = useCallback(() => {
    syncGutterScroll()
    if (autocomplete.open) {
      updateAutocompletePosition()
    }
  }, [autocomplete.open, syncGutterScroll, updateAutocompletePosition])

  const totalCommandCount = Array.from(script.commands.values()).reduce((sum, count) => sum + count, 0)

  return (
    <div className="preview-panel script-editor-panel">
      <div className="script-editor-header">
        <div>
          <div className="script-editor-title">Script Editor</div>
          <div className="script-editor-path mono small">{node.path}</div>
        </div>
        <div className="script-editor-actions">
          <span className={`script-editor-status state-${saveState}`}>
            {isLoading ? 'Loading...' : saveState === 'saving' ? 'Saving...' : isDirty ? 'Unsaved' : 'Saved'}
          </span>
          <button type="button" className="btn btn-sm btn-primary" onClick={handleManualSave} disabled={isLoading || !isDirty}>
            Save
          </button>
        </div>
      </div>

      {errorText && <div className="script-editor-banner error">{errorText}</div>}

      <div className="script-editor-workspace">
        <div ref={codepaneRef} className="script-editor-codepane">
          <div ref={gutterRef} className="script-editor-gutter" aria-hidden="true">
            <div className="script-editor-block-bands">
              {blockBands.map(block => (
                <div
                  key={block.id}
                  className="script-editor-block-band"
                  style={{
                    ['--block-accent' as string]: block.color,
                    top: `calc(12px + ${block.startLine} * 1.55em)`,
                    height: `calc(${Math.max(1, block.endLine - block.startLine)} * 1.55em)`,
                  }}
                  title={block.label}
                />
              ))}
            </div>
            {script.lines.map((line, index) => (
              <div key={index} className={`script-editor-line-number ${line.trim() === '' ? 'empty' : ''}`}>
                <span className="script-editor-line-label">
                  {line.trim() === '' ? '↩' : displayLineNumbers[index]}
                </span>
                </div>
              ))}
            </div>

          <textarea
            ref={textareaRef}
            className="script-editor-textarea"
            value={text}
            spellCheck={false}
            wrap="off"
            onChange={e => handleEditorChange(e.target.value, e.currentTarget.selectionStart ?? e.target.value.length)}
            onClick={handleEditorClick}
            onKeyUp={handleEditorClick}
            onBlur={closeAutocomplete}
            onScroll={handleEditorScroll}
            onKeyDown={handleKeyDown}
            placeholder="스크립트를 입력하세요."
          />

          {autocomplete.open && filteredAutocomplete.length > 0 && (
            <div
              ref={autocompleteRef}
              className="script-editor-autocomplete"
              role="listbox"
              aria-label="Command autocomplete"
              style={{ left: `${autocompletePosition.left}px`, top: `${autocompletePosition.top}px`, maxHeight: `${autocompletePosition.maxHeight}px` }}
            >
              {filteredAutocomplete.map((item, index) => (
                <button
                  type="button"
                  key={item.command}
                  className={`script-editor-autocomplete-item ${index === autocomplete.index ? 'active' : ''}`}
                  onMouseDown={e => e.preventDefault()}
                  onClick={() => handleSuggestionClick(item)}
                >
                  <span className="script-editor-autocomplete-label">{item.label}</span>
                  <span className="script-editor-autocomplete-detail">{item.detail}</span>
                </button>
              ))}
            </div>
          )}
        </div>

        <aside className="script-editor-sidebar">
          <section className="script-editor-card">
            <div className="script-editor-card-title">Labels</div>
            <div className="script-editor-chip-row">
              {script.labels.length === 0 && <span className="script-editor-empty">레이블 없음</span>}
              {script.labels.map(label => (
                <button type="button" key={`${label.name}-${label.line}`} className="script-editor-chip" onClick={() => focusLine(label.line)}>
                  <span>{label.name}</span>
                  <span className="script-editor-chip-line">L{label.line}</span>
                </button>
              ))}
            </div>
          </section>

          <section className="script-editor-card">
            <div className="script-editor-card-title">Validation</div>
            <div className="script-editor-summary">
              <div>Commands: {totalCommandCount}</div>
              <div>Labels: {script.labels.length}</div>
              <div>Jumps: {script.jumps.length}</div>
              <div className="script-editor-warning-count">Issues: {script.issues.length}</div>
            </div>
            <div className="script-editor-issues">
              {script.issues.length === 0 && <div className="script-editor-empty">문제 없음</div>}
              {script.issues.map((issue, index) => (
                <button type="button" key={`${issue.line}-${index}`} className={`script-editor-issue ${issue.level}`} onClick={() => focusLine(issue.line)}>
                  <span className="script-editor-issue-line">L{issue.line}</span>
                  <span className="script-editor-issue-message">{issue.message}</span>
                </button>
              ))}
            </div>
          </section>

          <section className="script-editor-card">
            <div className="script-editor-card-title">Jump preview</div>
            <div className="script-editor-summary">
              {script.jumps.length === 0 && <div className="script-editor-empty">@jump 없음</div>}
              {script.jumps.map((jump, index) => (
                <button type="button" key={`${jump.target}-${jump.line}-${index}`} className={`script-editor-jump ${jump.resolved ? 'ok' : 'missing'}`} onClick={() => focusLine(jump.line)}>
                  <span>@jump "{jump.target}"</span>
                  <span>{jump.resolved ? 'OK' : 'Missing'}</span>
                </button>
              ))}
            </div>
          </section>

          <section className="script-editor-card">
            <div className="script-editor-card-title">Flow analysis</div>
            <div className="script-editor-summary">
              <div>Blocks: {script.blocks.length}</div>
              <div>Unreachable: {script.unreachableBlocks.length}</div>
              <div>Loop risks: {script.loopFindings.length}</div>
            </div>
            <div className="script-editor-issues">
              {script.unreachableBlocks.length === 0 && script.loopFindings.length === 0 && (
                <div className="script-editor-empty">흐름 문제 없음</div>
              )}
              {script.unreachableBlocks.map(block => (
                <button type="button" key={`unreachable-${block.id}`} className="script-editor-issue error" onClick={() => focusLine(block.startLine)}>
                  <span className="script-editor-issue-line">L{block.startLine}</span>
                  <span className="script-editor-issue-message">도달 불가 블럭: {block.label}</span>
                </button>
              ))}
              {script.loopFindings.map((finding, index) => (
                <button type="button" key={`loop-${index}`} className="script-editor-issue warning" onClick={() => focusLine(finding.line)}>
                  <span className="script-editor-issue-line">L{finding.line}</span>
                  <span className="script-editor-issue-message">{finding.message}</span>
                </button>
              ))}
            </div>
          </section>
        </aside>
      </div>

      <div className="metadata-panel script-editor-footer">
        <div className="metadata-row">
          <span className="metadata-label">File:</span>
          <span className="metadata-value mono small">{node.name}</span>
        </div>
        <div className="metadata-row">
          <span className="metadata-label">Lines:</span>
          <span className="metadata-value">{script.lines.length}</span>
        </div>
        <div className="metadata-row">
          <span className="metadata-label">State:</span>
          <span className="metadata-value">{isDirty ? 'Modified' : 'Clean'}</span>
        </div>
      </div>
    </div>
  )
}
