---
title: "Editor Architecture"
project: Voradorix
tags:
  - voradorix
  - editor
  - architecture
status: planning
---

# Voradorix Editor — Architecture

> Electron 기반 에디터의 프로세스 구조, 통신 방식, 보안 모델을 정의합니다.

---

## 1. 프로세스 모델

```
┌─────────────────────────────────────────────────────────────────┐
│  Electron Application                                          │
│                                                                │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Main Process (Node.js)                                 │   │
│  │  ├── BrowserWindow 관리 (생성/종료)                      │   │
│  │  ├── ipcMain 핸들러 (파일 I/O, Voradorix.exe spawn)     │   │
│  │  ├── Voradorix.exe 생명주기 관리                         │   │
│  │  └── 메뉴/단축키 관리                                   │   │
│  └────────────────┬────────────────────────────────────────┘   │
│                   │ contextBridge (preload)                     │
│  ┌────────────────▼─────────────────────────────────────────┐   │
│  │  Renderer Process (Chromium, React)                      │   │
│  │  ├── Asset Browser UI                                    │   │
│  │  ├── Script Editor UI (Phase 3-1)                        │   │
│  │  └── Widget Designer UI (Phase 3-2)                      │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Child Process (Voradorix.exe)                          │   │
│  │  ├── C++ / SFML 게임 엔진                               │   │
│  │  ├── spawn()으로 실행, kill()으로 종료                    │   │
│  │  └── 별도 OS 창에서 실행                                 │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. 보안 모델

### 2.1 Context Isolation

```typescript
// main/index.ts
const win = new BrowserWindow({
    webPreferences: {
        contextIsolation: true,   // Renderer와 Main 분리
        nodeIntegration: false,    // Renderer에서 Node.js 직접 접근 금지
        preload: join(__dirname, '../preload/index.js'),
    }
});
```

### 2.2 Preload Bridge

Renderer는 오직 `contextBridge.exposeInMainWorld`로 노출된 API만 사용 가능합니다.

```typescript
// preload/index.ts
contextBridge.exposeInMainWorld('editorAPI', {
    // 에셋 관련
    readAssetRegistry: () => ipcRenderer.invoke('registry:read'),
    writeAssetRegistry: (data) => ipcRenderer.invoke('registry:write', data),
    importAsset: (src, type) => ipcRenderer.invoke('asset:import', src, type),
    deleteAsset: (guid) => ipcRenderer.invoke('asset:delete', guid),

    // 파일 시스템
    readDirectory: (path) => ipcRenderer.invoke('fs:readDirectory', path),
    readFile: (path) => ipcRenderer.invoke('fs:readFile', path),

    // 게임 실행
    launchGame: () => ipcRenderer.invoke('game:launch'),
    stopGame: () => ipcRenderer.invoke('game:stop'),
});
```

---

## 3. 데이터 흐름

### 3.1 Editor → Engine (파일 기반)

```
Editor에서 AssetRegistry.json 수정
  → Editor가 JSON 파일 직접 저장
  → Engine 시작 시 (spawn) 또는 ReloadRegistry() 호출 시 변경사항 반영
```

### 3.2 File System 접근 경로

```typescript
// 프로젝트 루트 경로 (Voradorix.exe 위치 기준 2단계 상위 = Game/)
const PROJECT_ROOT = path.resolve(app.getAppPath(), '..', '..', '..');
// 실제로는 Editor/build/Voradorix.exe 고정 경로 사용

const ASSETS_DIR = path.join(PROJECT_ROOT, 'Assets');
const REGISTRY_PATH = path.join(ASSETS_DIR, 'AssetRegistry.json');
const GAME_EXE_PATH = path.join(PROJECT_ROOT, 'Editor', 'build', 'Voradorix.exe');
```

---

## 4. Voradorix.exe 실행 상세

### 4.1 실행 절차

```typescript
async function launchGame(): Promise<void> {
    // 1. 고정 실행 파일 경로 사용
    const editorRoot = path.resolve(__dirname, '..', '..');
    const projectRoot = path.resolve(editorRoot, '..');
    const exePath = path.join(editorRoot, 'build', 'Voradorix.exe');

    // 3. spawn
    gameProcess = spawn(exePath, [], {
        cwd: projectRoot,    // 작업 디렉토리 = Game/
        detached: false,
        stdio: 'pipe',       // stdout/stderr 캡처 가능
    });

    // 4. 종료 감지
    gameProcess.on('exit', (code, signal) => {
        gameProcess = null;
        // Renderer에 상태 전파
    });
}
```

### 4.2 작업 디렉토리 중요성

Voradorix.exe는 `Assets/`, `Saves/`, `AssetRegistry.json`을 현재 작업 디렉토리 기준으로 찾습니다.
따라서 `cwd: projectRoot`는 솔루션 루트(`Game/`)를 가리켜야 하며, 실행 시점에는 디버거와 Editor가 동일한 기준 경로를 사용해야 합니다.

---

## 5. 빌드 구성

### 5.1 개발 환경

```
Editor/                           ← Electron 프로젝트
Game/                             ← Git 저장소 루트
  Game.sln                        ← Visual Studio 솔루션
  Game/                           ← vcxproj
    Editor/build/Voradorix.exe    ← 빌드 결과
  Assets/                         ← 에셋, AssetRegistry.json
```

### 5.2 Voradorix.exe 자동 복사 (Post-Build)

Game.vcxproj의 Post-Build 이벤트:

```xml
<PostBuildEvent>
  <Command>
    if not exist "$(SolutionDir)Editor\build" mkdir "$(SolutionDir)Editor\build"
    copy /Y "$(TargetPath)" "$(SolutionDir)Editor\build\Voradorix.exe"
    xcopy /Y /D "$(SolutionDir)Extern\SFML-3.1.0\bin\sfml-*-3.dll" "$(SolutionDir)Editor\build\"
  </Command>
</PostBuildEvent>
```

---

## 6. 향후 IPC 확장 (선택적)

초기에는 파일 기반 연동만으로 충분하지만, 향후 라이브 프리뷰 등이 필요할 때:

| 방식 | 설명 | 도입 시점 |
|------|------|-----------|
| **로컬 HTTP 서버** | 엔진이 내장 HTTP 서버를 띄워 REST API 제공 | Phase 3 이후 |
| **stdin/stdout JSON** | spawn 시 pipe로 연결, JSON 라인 단위 통신 | Phase 3 이후 |
| **WebSocket** | 엔진이 WebSocket 서버 실행, 에디터가 접속 | Phase 3 이후 |
| **공유 메모리** | 성능이 중요한 경우 | Phase 3 이후 |

초기 개발에서는 **고정 실행 파일 복사 방식**으로 시작하고, 필요에 따라 **stdin/stdout**을 가장 먼저 검토할 수 있습니다.
