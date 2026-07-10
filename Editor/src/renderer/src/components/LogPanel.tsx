import { useEffect, useRef } from 'react'
import { LogMessage } from '../types'

interface LogPanelProps {
  messages: LogMessage[]
}

export default function LogPanel({ messages }: LogPanelProps) {
  const bottomRef = useRef<HTMLDivElement>(null)

  useEffect(() => {
    bottomRef.current?.scrollIntoView({ behavior: 'smooth' })
  }, [messages.length])

  return (
    <div className="log-panel">
      <div className="panel-header">Log</div>
      <div className="log-scroll">
        {messages.length === 0 ? (
          <div className="log-empty">No log entries.</div>
        ) : (
          messages.map(m => (
            <div key={m.id} className={`log-entry log-${m.type}`}>
              {m.text}
            </div>
          ))
        )}
        <div ref={bottomRef} />
      </div>
    </div>
  )
}
