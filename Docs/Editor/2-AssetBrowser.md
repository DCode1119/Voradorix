---
title: "Editor Phase 2: Asset Browser"
project: Voradorix
tags:
  - voradorix
  - editor
  - asset-browser
  - spec
status: implemented
---

# Editor Phase 2: Asset Browser

> **목표**: 프로젝트 에셋을 시각적으로 탐색/미리보기/Import/삭제하는 Asset Browser 구현  
> **의존성**: Editor Phase 1 (Project Setup) 완료, C++ AssetManager (Registry 스키마 확정)  
> **완료 조건**: Assets/ 디렉토리 파일을 트리로 표시하고, 이미지 미리보기 및 Import/삭제가 가능함

---

## 1. UI 구성

```
┌────────────────────────────────────────────────────────────────┐
│  Voradorix Editor                         [▶ Play] [■ Stop]  │
│  ┌──────────┬──────────────────────────────────────────────┐  │
│  │ 🔍 필터  │  Asset Preview                              │  │
│  │          │  ┌────────────────────────────────────────┐  │  │
│  │ 📁 Assets│  │                                        │  │  │
│  │  ├─ Backg│  │         [WhiteRoom.png]                │  │  │
│  │  │  Whit…│  │                                        │  │  │
│  │  │  Whit…│  └────────────────────────────────────────┘  │  │
│  │  ├─ Char…│                                               │  │
│  │  │  Laur…│  Name:      WhiteRoom                         │  │
│  │  │   Nor…│  Type:      Texture                           │  │
│  │  │   Ref…│  GUID:      a1b2c3d4-...                     │  │
│  │  ├─ Fonts│  Alias:     (설정 가능)                       │  │
│  │  │  malg…│  Source:    Assets/Backgrounds/WhiteRoom.png  │  │
│  │  └─ Scri…│                                               │  │
│  │          │  [Import]  [Delete]  [Reveal in Explorer]     │  │
│  │          │                                               │  │
│  │  ┌──────┴──────────────────────────────────────────────┐ │
│  │  │  Log: "WhiteRoom.png imported (GUID: a1b2...)"     │ │
│  │  └─────────────────────────────────────────────────────┘ │
│  └──────────┴──────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
```

### 레이아웃

| 영역 | 위치 | 설명 |
|------|------|------|
| **Asset Tree** | 좌측 패널 | 에셋 타입별/폴더별 계층 트리 |
| **Preview** | 우측 상단 | 선택한 에셋의 시각적 미리보기 |
| **Metadata** | 우측 중앙 | GUID, Alias, SourcePath 등 정보 표시 |
| **Action Bar** | 우측 하단 | Import, Delete, Reveal 버튼 |
| **Log** | 하단 | 작업 로그 메시지 |

---

## 2. Asset Tree

### 2.1 트리 구조

에셋 타입을 최상위 노드로, 파일 시스템 계층을 하위 노드로 표시:

```
📁 프로젝트 Assets
├── 🖼️ Backgrounds
│   ├── WhiteRoom.png
│   └── WhiteRoom_Sunset.png
├── 👤 Characters
│   └── Laura
│       ├── Normal.png
│       └── Reference/ (표시하되 Import 불가)
├── 🔤 Fonts
│   └── malgun.ttf
└── 📜 Scripts
    └── TestScript.txt
```

### 2.2 기능

- 각 노드 우클릭 컨텍스트 메뉴: Delete
- 상단 Import 버튼: 파일/디렉토리 다중 선택
- 디렉토리 Import: 하위 디렉토리까지 재귀 복사
- 더블클릭: 미리보기 갱신
- 드래그 앤 드롭: (추후 고려)
- 필터/검색: 입력 시 트리 필터링

### 2.3 Import 상태 표시

| 아이콘 | 의미 |
|--------|------|
| 🟢 | AssetRegistry에 등록됨 |
| ⚪ | 미등록 (파일만 있음) |

---

## 3. Asset Import

### 3.1 Import 트리거

1. **Import 버튼** — 파일/디렉토리를 한 번에 선택해 복사
2. **외부 파일 드래그 앤 드롭** — OS 파일 탐색기에서 Assets/ 폴더로 드롭
3. **폴더 감시** (선택) — `Assets/` 내 신규 파일 자동 감지

### 3.2 Import 절차

```
사용자 동작:
  ① 파일/디렉토리 선택 (또는 드래그)
  ② Import 버튼 클릭

내부 처리:
  ① UUID v4 생성 (Node.js crypto.randomUUID())
  ② 소스 파일/디렉토리를 Assets/ 아래로 복사
  ③ 하위 디렉토리 재귀 복사
  ④ AssetRegistry.json에 엔트리 추가
  ⑤ AssetRegistry.json 저장
  ⑥ 완료 로그 출력, 트리 갱신
```

### 3.3 Import 규칙

| 항목 | 규칙 |
|------|------|
| 파일명 | 원본 파일명 유지 (중복 시 `_1`, `_2` 접미사) |
| 타입 추론 | 확장자 기반 (`.png`→Texture, `.ttf`→Font, `.txt`→Script) |
| Alias | Import 시 미설정 (빈 문자열), 사용자가 수동 지정 가능 |
| 복사 경로 | 선택한 대상 경로 아래로 복사 |

### 3.4 지원 파일 형식 (초기)

| 확장자 | 에셋 타입 | 미리보기 |
|--------|-----------|----------|
| `.png` | Texture | 이미지 표시 |
| `.jpg`, `.jpeg` | Texture | 이미지 표시 |
| `.ttf` | Font | 폰트 샘플 텍스트 렌더링 |
| `.txt` | Script | 텍스트 내용 표시 |

---

## 4. Asset 미리보기

### 4.1 타입별 미리보기

| 에셋 타입 | 미리보기 내용 |
|-----------|---------------|
| **Texture** | 이미지를 Preview 영역에 맞게 리사이즈하여 표시. 좌측 하단에 해상도(너비×높이) 표시 |
| **Font** | "Aa 가나다 123" 샘플 텍스트를 해당 폰트로 렌더링하여 표시 |
| **Script** | 텍스트 내용을 읽어 코드 블록 형태로 표시 (Phase 4 이전까지는 plain text) |

### 4.2 메타데이터 표시

| 필드 | 설명 |
|------|------|
| GUID | UUID v4 (읽기 전용) |
| Alias | 사용자 지정 가능한 텍스트 필드 (옵셔널) |
| Type | 에셋 타입 (읽기 전용) |
| Source Path | 프로젝트 내 상대 경로 (읽기 전용) |
| Display Name | 메타데이터 표시명 (편집 가능, 추후 확장) |

---

## 5. AssetRegistry.json 읽기/쓰기

### 5.1 IPC 인터페이스

Renderer → Main Process → Node.js fs:

```typescript
// preload에서 노출
window.electronAPI.readAssetRegistry() : Promise<AssetRegistryData>
window.electronAPI.writeAssetRegistry(data: AssetRegistryData) : Promise<void>
window.electronAPI.pickImportSources() : Promise<{ canceled: boolean; sources: ... }>
window.electronAPI.previewImport(sources, targetDir) : Promise<ImportPreviewData | { error: string }>
window.electronAPI.executeImport(sources, targetDir, overwriteTargets) : Promise<ImportExecuteResult | { error: string }>
window.electronAPI.deleteAsset(guid: string) : Promise<void>
window.electronAPI.readDirectory(relativePath: string) : Promise<FileEntry[]>
```

### 5.2 데이터 타입

```typescript
interface AssetRegistryData {
    version: number;
    assets: AssetEntry[];
    aliases: Record<string, string>;
}

interface AssetEntry {
    guid: string;
    type: 'texture' | 'font' | 'script';
    alias: string | null;
    sourcePath: string;
    meta?: Record<string, unknown>;
}

interface FileEntry {
    name: string;
    path: string;
    isDirectory: boolean;
    extension: string;
    size: number;
    isRegistered: boolean;  // Registry에 등록되었는지 여부
}
```

---

## 6. Main Process 핸들러

```typescript
// src/main/ipcHandlers.ts

// Registry 읽기
ipcMain.handle('registry:read', async () => {
    const raw = fs.readFileSync(registryPath, 'utf-8');
    return JSON.parse(raw);
});

// Registry 쓰기
ipcMain.handle('registry:write', async (_event, data) => {
    fs.writeFileSync(registryPath, JSON.stringify(data, null, 2), 'utf-8');
});

// Import
ipcMain.handle('asset:import', async (_event, sourcePath, type) => {
    const guid = crypto.randomUUID();
    const destPath = determineDestPath(sourcePath, type);
    fs.copyFileSync(sourcePath, destPath);
    // Registry에 추가...
    return { guid, sourcePath: destPath };
});

// 디렉토리 읽기
ipcMain.handle('fs:readDirectory', async (_event, relativePath) => {
    const fullPath = join(projectRoot, relativePath);
    const entries = fs.readdirSync(fullPath, { withFileTypes: true });
    return entries.map(e => ({
        name: e.name,
        path: join(relativePath, e.name),
        isDirectory: e.isDirectory(),
        extension: extname(e.name),
        size: e.isFile() ? fs.statSync(join(fullPath, e.name)).size : 0,
        isRegistered: registry.assets.some(a => a.sourcePath === join(relativePath, e.name)),
    }));
});
```

---

## 7. 게임 실행 (Phase 2.5 — Play/Stop 버튼)

### 7.1 Voradorix.exe 실행 경로

```text
// Game/Editor/build/Voradorix.exe (Post-Build 자동 생성)
```

### 7.2 spawn/kill

```typescript
import { spawn, ChildProcess } from 'child_process';

let gameProcess: ChildProcess | null = null;

const editorRoot = path.resolve(__dirname, '..', '..');
const projectRoot = path.resolve(editorRoot, '..');

ipcMain.handle('game:launch', async () => {
    const exePath = join(editorRoot, 'build', 'Voradorix.exe');

    gameProcess = spawn(exePath, [], {
        cwd: projectRoot,
        detached: false,
    });

    gameProcess.on('exit', (code) => {
        gameProcess = null;
        // Renderer에 알림
    });
});

ipcMain.handle('game:stop', async () => {
    if (gameProcess) {
        gameProcess.kill();
        gameProcess = null;
    }
});
```

---

## 8. 에러 처리

| 상황 | 처리 |
|------|------|
| Registry JSON 파싱 실패 | 기본 구조 `{version:1, assets:[], aliases:{}}`로 초기화, 로그 출력 |
| 파일 복사 실패 | 에러 메시지를 Log 영역에 출력, Import 중단 |
| 지원하지 않는 파일 형식 Import 시도 | "Unsupported file type" 경고 |
| 중복 파일명 | 자동 접미사 추가 후 복사 |
| Voradorix.exe 없음 | "Voradorix.exe not found. Build the engine first." 경고 |

---

## 9. 완료 조건

- [x] Asset Tree에 `Assets/` 디렉토리 구조가 표시되는가?
- [x] 등록된 에셋과 미등록 에셋이 시각적으로 구분되는가?
- [x] PNG/JPEG 이미지 미리보기가 표시되는가?
- [x] TTF 폰트 미리보기가 표시되는가?
- [x] TXT 스크립트 내용이 표시되는가?
- [x] Import 버튼으로 파일/디렉토리 다중 선택 등록이 가능한가?
- [x] Import 후 Registry.json에 엔트리가 추가되는가?
- [x] Delete 버튼으로 등록 해제가 가능한가?
- [ ] Play 버튼으로 Voradorix.exe가 실행되는가?
- [ ] Stop 버튼으로 Voradorix.exe가 종료되는가?
