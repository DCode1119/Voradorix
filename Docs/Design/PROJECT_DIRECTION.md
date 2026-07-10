---
title: 프로젝트 방향 전환
project: Voradorix
tags:
  - voradorix
  - direction
  - planning
status: active
---

# Voradorix — 프로젝트 방향 전환

> **작성일**: 2026-07-10  
> **이전 방향**: 비주얼 노벨 게임 개발  
> **새 방향**: 게임 엔진 + 에디터 개발  
> **계기**: 콘텐츠 제작이 시작되지 않은 상태에서, 에디터를 먼저 갖추어 제작 생산성을 확보한 후 콘텐츠 작업에 들어가는 것이 효율적이라고 판단

---

## 1. 방향 전환 개요

| 구분 | 이전 | 이후 |
|------|------|------|
| 목표 | 비주얼 노벨 게임 완성 | 게임 엔진(Voradorix Engine) + 에디터(Voradorix Editor) 개발 |
| 콘텐츠 | 시나리오/에셋 제작 병행 | **후순위** (에셋은 프로토타입용, 삭제 가능) |
| 엔진 단계 | 9단계(연출 효과) 구현 예정 | **9단계 보류**, AssetManager 우선 |
| 에디터 | 없음 (텍스트 파일 직접 편집) | Electron 기반 Asset Browser → 확장 예정 |

---

## 2. 전체 아키텍처

```
┌─────────────────────────────────────────────────────────────┐
│  Voradorix Editor (Electron + React)                        │
│  ┌────────┐ ┌────────┐ ┌────────┐    ┌────┐                │
│  │Asset   │ │Script  │ │Widget  │    │[▶] │                │
│  │Browser │ │Editor  │ │Designer│    │[■] │                │
│  │(1차)   │ │(예정)  │ │(예정)  │    │    │                │
│  └────────┘ └────────┘ └────────┘    └─┬──┘                │
│       ┌────────────────────────────┐    │ spawn()           │
│       │ Node.js 파일 시스템 (fs)   │    │                   │
│       │ ├─ Assets/                 │    ▼                   │
│       │ ├─ AssetRegistry.json      │  ┌───────────────┐    │
│       │ └─ Saves/                  │  │ Game.exe      │    │
│       └────────────────────────────┘  │ (C++ SFML)    │    │
│                                        └───────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### 핵심 원칙

- **에디터가 메인 프로세스**, 엔진은 자식 프로세스(`child_process.spawn()`)로 실행
- **데이터 연동:** 파일 기반 (`AssetRegistry.json` 공유)
- **런타임 UI:** 기존 SFML WidgetBase 계층 유지 (게임 화면 렌더링)
- **에디터 UI:** Electron (HTML/CSS/React) — 에셋 관리, 스크립트 편집 등 개발 도구
- IPC 초기 단계에서는 불필요, 필요 시 로컬 HTTP 또는 stdin/stdout으로 확장 가능

---

## 3. 기술 스택

### 엔진 (기존 유지)

| 항목 | 사양 |
|------|------|
| 언어 | C++17 |
| IDE | Visual Studio 2022 (v143) |
| 플랫폼 | x64 |
| 그래픽 | SFML 3.1.0 (동적 링크) |
| 서브시스템 | Windows |
| JSON | nlohmann/json v3.12.0 |

### 에디터 (신규)

| 항목 | 결정 |
|------|------|
| 프레임워크 | Electron |
| UI 라이브러리 | React (권장) / 추후 결정 |
| 번들러 | Vite |
| 위치 | `Game/Editor/` |
| 엔진 실행 | `child_process.spawn("Game.exe")` |
| 게임 경로 전달 | 빌드 시 생성되는 `build/gamePath.json` |
| 파일 접근 | Node.js `fs` 모듈 |

---

## 4. AssetManager (C++ 엔진)

### 4.1 기본 설계

| 항목 | 결정 |
|------|------|
| 생명주기 | 싱글톤, 독립 모듈 (WidgetBase 미상속) |
| 초기화 | `CVrdxApplication::Initialize()` 내에서 `InitializeInstance()` 호출 |
| 종료 | `Shutdown()` — Registry 저장 및 캐시 정리 |
| 내부 식별자 | UUID v4 |
| Alias | 옵셔널 (Name 기반 등록 시 자동 생성, 추후 활용도 살핌) |
| 레지스트리 | `AssetRegistry.json` 단일 파일 |
| Source 경로 | Import 시 프로젝트 내 `Assets/`로 복사 |
| 파일명 규칙 | 원본 파일명 유지 (Import 경로는 에디터에서 편집 가능) |

- `AssetRegistry.json` 파싱 시 `alias` 값이 null이면 내부에서는 빈 문자열로 정규화

### 4.2 에셋 타입 체계

```cpp
enum class EVrdxAssetType : uint8_t
{
    Texture,
    Font,
    Script,
    // Audio, WidgetBlueprint ... (확장 가능)
    Count
};
```

### 4.3 레지스트리 스키마

```json
{
  "version": 1,
  "assets": [
    {
      "guid": "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
      "type": "font",
      "alias": "malgun",
      "sourcePath": "Assets/Fonts/malgun.ttf"
    }
  ],
  "aliases": {
    "malgun": "a1b2c3d4-e5f6-7890-abcd-ef1234567890"
  }
}
```

- `alias`는 null 허용 (옵셔널)
- 최초 빈 상태로 시작, `LoadFont()` 등 호출 시 자동 등록됨
- 현재 UI 폰트는 `DialogueBox` / `TextLabel` / `ChoiceWidget`에서 AssetManager 경유로 공유

### 4.4 LoadFont 동작 흐름

```
LoadFont("malgun")
  → GUID 형식? → No
  → AliasToGuid["malgun"] 존재? → 없음 (최초)
  → "Assets/Fonts/malgun.ttf" 로드 시도 (AssetManager 내부)
  → 성공 시:
      GUID 생성, Registry에 자동 등록
      FontCache[GUID] = 로드된 폰트 (shared_ptr)
      AliasToGuid["malgun"] = GUID
  → FontCache[GUID] 반환

두 번째 호출:
  LoadFont("malgun")
  → AliasToGuid["malgun"] 존재 → GUID 획득
  → FontCache[GUID] 존재 → 캐시 반환 (중복 로딩 없음)
```

### 4.5 점진적 전환 순서

| 순서 | 내용 | 영향 파일 |
|------|------|-----------|
| 1 | LoadFont() 구현 + 폰트 3곳 리팩터 | DialogueBox, TextLabel, ChoiceWidget |
| 2 | LoadTexture() + ImportAsset() | Background, CharacterManager |
| 3 | Background 텍스처 캐싱 전환 | Background.cpp |
| 4 | CharacterManager 자체 캐시 → AssetManager 이관 | CharacterManager.cpp |
| 5 | ScriptEngine 파일 로딩 캐싱 | ScriptEngine.cpp |
| 6 | UI 위젯 텍스처 (TitleWindow 등) | TitleWindow, BoxWidget |

---

## 5. Electron 에디터

### 5.1 디렉토리 구조 (초안)

```
Game/Editor/
├── package.json
├── vite.config.ts
├── tsconfig.json
├── .gitignore                 (node_modules/ 포함)
├── src/
│   ├── main/                  # Electron main process
│   │   ├── index.ts           # BrowserWindow 생성, 메뉴
│   │   └── gameLauncher.ts    # spawn/kill Game.exe
│   ├── renderer/              # React UI
│   │   ├── App.tsx
│   │   ├── AssetBrowser/      # 에셋 브라우저
│   │   │   ├── AssetTree.tsx
│   │   │   ├── AssetPreview.tsx
│   │   │   └── ImportDialog.tsx
│   │   ├── PlayButton.tsx     # 실행/중지 버튼
│   │   └── common/            # 공통 컴포넌트
│   └── preload/               # contextBridge
│       └── index.ts
└── build/
    └── gamePath.json          # (자동 생성) 엔진 실행 파일 경로
```

### 5.2 첫 기능 — Asset Browser

- `Assets/` 디렉토리 파일 트리 표시
- 이미지/폰트 미리보기
- Import 버튼 → GUID 생성 + 파일 복사 + Registry 등록
- 등록 에셋 목록 조회 및 삭제
- `AssetRegistry.json` 읽기/쓰기 (Node.js fs)

### 5.3 게임 실행 경로

- 엔진 빌드 시 Post-Build 이벤트로 `build/gamePath.json` 자동 생성
- 내용: `{ "path": "Bin/x64/Debug/Game.exe" }`
- Debug/Release에 따라 경로 분기 가능

---

## 6. 단계별 로드맵

### Phase 1 — AssetManager 코어 (C++)

| Step | 내용 | 비고 |
|------|------|------|
| 1.0 | `AssetManager.h/cpp` 생성, 싱글톤 골격, Registry I/O | Core/ 신규 파일 |
| 1.1 | `LoadFont()` 구현, 폰트 3곳 리팩터 | DialogueBox, TextLabel, ChoiceWidget |
| 1.2 | `LoadTexture()`, `ImportAsset()` 구현 | Background, CharacterManager |
| 1.3 | Background/CharacterManager 점진적 전환 | 기존 로직 유지하며 전환 |

### Phase 2 — Electron 에디터

| Step | 내용 |
|------|------|
| 2.0 | Electron 프로젝트 생성 (Vite + React) |
| 2.1 | Asset Browser UI (트리 뷰, 미리보기, Import) |
| 2.2 | AssetRegistry.json 읽기/쓰기 연동 |
| 2.3 | Play/Stop 버튼 → spawn()/kill() |

### Phase 3 — 확장

| Step | 내용 |
|------|------|
| 3.0 | Script Editor (문법 하이라이팅, @label 네비게이션) |
| 3.1 | Widget Designer (위젯 트리 JSON 편집) |
| 3.2 | 엔진 ↔ 에디터 라이브 연동 (선택적) |

---

## 7. 변경 영향을 받는 기존 문서

| 문서 | 변경 사항 |
|------|-----------|
| `README.md` | 프로젝트 설명을 엔진+에디터 방향으로 갱신 |
| `Docs/Voradorix.md` | phase 변경, 9단계 보류 표시, 신규 항목 추가 |
| `Docs/BACKLOG.md` | EffectManager를 Later로 이동, AssetManager/Editor 항목 추가 |
| `Docs/WORK_LOG.md` | 오늘 논의 내용 기록 |
| `Docs/PROJECT_STRUCTURE.md` | Editor/ 경로 추가 |
| `Docs/GAME_DESIGN.md` | 방향 전환 노트 추가 (런타임 설계로 범위 한정) |
| `Docs/9-EffectManager.md` | 상태를 "보류"로 변경 |
| 기존 단계별 명세(1~8) | 변경 없음 (런타임 엔진 설계로 유효) |

---

## 8. 미확정 및 추후 결정 사항

| 사항 | 상태 |
|------|------|
| Electron UI 라이브러리 (React / Vue / Svelte) | 미정 |
| Import 시 타입별 하위 폴더 구조 규칙 | 에디터 구현 시 결정 |
| 엔진 ↔ 에디터 간 라이브 IPC 방식 | Phase 3에서 결정 |
| CVrdxApplication → CVrdxEngine 개명 시점 | 추후 |
