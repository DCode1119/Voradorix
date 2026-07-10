---
title: "Editor Phase 1: Project Setup"
project: Voradorix
tags:
  - voradorix
  - editor
  - setup
  - electron
status: setup-complete
---

# Editor Phase 1: Project Setup

> **목표**: Electron + React + Vite 개발 환경 구축 및 빌드/실행 확인  
> **소요 예상**: 기준 없음 (초기 설정)  
> **완료 조건**: Electron 창이 뜨고 "Hello Voradorix Editor" 텍스트가 표시됨

---

## 1. 디렉토리 구조

```
Game/Editor/
├── package.json
├── electron.vite.config.ts        # or vite.config.ts + electron-builder
├── tsconfig.json
├── tsconfig.node.json
├── tsconfig.web.json
├── .gitignore                     # node_modules/, dist/, out/
│
├── src/
│   ├── main/                      # Electron Main Process
│   │   ├── index.ts               # BrowserWindow 생성, 앱 생명주기
│   │   └── gameLauncher.ts        # Voradorix.exe spawn/kill (Phase 3)
│   │
│   ├── preload/                   # Preload Script
│   │   ├── index.ts               # contextBridge API 노출
│   │   └── index.d.ts             # Window API 타입 선언
│   │
│   └── renderer/                  # React UI
│       ├── index.html
│       ├── src/
│       │   ├── main.tsx           # React 진입점
│       │   ├── App.tsx            # 라우트/레이아웃
│       │   ├── assets/            # CSS, 이미지
│       │   ├── components/        # 공통 컴포넌트
│       │   └── pages/             # 페이지별 컴포넌트
│       └── env.d.ts
│
├── resources/                     # 앱 아이콘 등
│
└── build/                         # 빌드 산출물 (gitignore)
    └── Voradorix.exe              # (Phase 3) 엔진 실행 파일
```

---

## 2. package.json 구성

```json
{
  "name": "voradorix-editor",
  "version": "0.1.0",
  "description": "Voradorix Game Editor",
  "main": "./out/main/index.js",
  "scripts": {
    "dev": "electron-vite dev",
    "build": "electron-vite build",
    "preview": "electron-vite preview"
  }
}
```

**의존성:**

| 패키지 | 용도 |
|--------|------|
| `electron` | Electron 런타임 |
| `electron-vite` | Vite 기반 Electron 번들러 |
| `react` + `react-dom` | UI 라이브러리 |
| `@types/react` | 타입 정의 |
| `typescript` | 언어 |
| `tailwindcss` (제안) | CSS 유틸리티 |

---

## 3. Main Process 구조 (`src/main/index.ts`)

```typescript
// 핵심 흐름
app.whenReady().then(() => {
    const win = new BrowserWindow({
        width: 1280,
        height: 800,
        webPreferences: {
            preload: join(__dirname, '../preload/index.js'),
            contextIsolation: true,
            nodeIntegration: false,
        }
    });

    // dev: Vite dev server URL 로드
    // prod: 로컬 HTML 파일 로드
    if (is.dev) {
        win.loadURL('http://localhost:5173');
    } else {
        win.loadFile(join(__dirname, '../renderer/index.html'));
    }
});
```

**중요:** `contextIsolation: true`, `nodeIntegration: false` 유지. Renderer는 preload의 `contextBridge`를 통해서만 Node.js API에 접근.

---

## 4. Preload (`src/preload/index.ts`)

```typescript
// Renderer가 사용할 API 노출
contextBridge.exposeInMainWorld('electronAPI', {
    // 파일 시스템 (Phase 2에서 확장)
    readFile: (path: string) => ipcRenderer.invoke('fs:readFile', path),
    writeFile: (path: string, data: string) => ipcRenderer.invoke('fs:writeFile', path, data),
    readDirectory: (path: string) => ipcRenderer.invoke('fs:readDirectory', path),
    copyFile: (src: string, dest: string) => ipcRenderer.invoke('fs:copyFile', src, dest),

    // 게임 실행 (Phase 3)
    launchGame: () => ipcRenderer.invoke('game:launch'),
    stopGame: () => ipcRenderer.invoke('game:stop'),
});
```

---

## 5. Renderer (`src/renderer/src/App.tsx`)

```tsx
function App() {
    return (
        <div className="flex h-screen">
            {/* Phase 2: AssetBrowser 컴포넌트로 교체 */}
            <h1>Hello Voradorix Editor</h1>
        </div>
    );
}
```

---

## 6. 빌드/실행 확인

```bash
cd Game/Editor
npm install
npm run dev
```

또는 `Game/RunEditor.bat`로 에디터를 바로 실행할 수 있다.

- [ ] Electron 창이 정상적으로 열리는가?
- [ ] 개발자 도구에서 console.log 확인
- [ ] 프로덕션 빌드 (`npm run build`) 정상 동작 확인
- [ ] 디버거 실행 시 작업 디렉토리가 솔루션 루트인지 확인

---

## 7. .gitignore

```
node_modules/
dist/
out/
build/
*.local
```

---

## 8. 다음 Phase (2-AssetBrowser)로 넘어가기 전 확인 사항

- [ ] `npm run dev`로 Electron 창 정상 표시
- [ ] `npm run build`로 프로덕션 빌드 성공
- [ ] Preload → Renderer 간 IPC 통신 정상 (간단한 ping/pong 테스트)
- [ ] TypeScript 컴파일 에러 없음
- [ ] `Game/Editor/`가 `.gitignore`에 등록되어 있는지 확인
