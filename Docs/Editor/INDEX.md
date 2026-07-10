---
title: Voradorix Editor
project: Voradorix
tags:
  - voradorix
  - editor
  - index
status: planning
phase: 1 (Project Setup)
---

# Voradorix Editor

> **기술**: Electron + React + Vite  
> **위치**: `Game/Editor/`  
> **상태**: Phase 1 (Project Setup) ✅ 완료

---

## 개요

Voradorix Editor는 Electron 기반의 게임 개발 도구입니다.
C++/SFML 엔진과는 독립 프로세스로 동작하며, `AssetRegistry.json`을 통해 데이터를 공유합니다.

### 핵심 원칙

1. **에디터가 메인 프로세스** — 엔진(Voradorix.exe)은 에디터의 `child_process.spawn()`으로 실행
2. **파일 기반 데이터 연동** — `AssetRegistry.json`, 루트 `Assets/` 디렉토리를 통해 에디터와 엔진이 데이터 공유
3. **점진적 확장** — Asset Browser → Script Editor → Widget Designer 순서로 기능 추가
4. **런타임 UI는 SFML 유지** — 에디터는 개발 도구, 게임 화면 자체는 엔진이 담당

---

## 로드맵

| Phase | 기능 | 상태 |
|-------|------|------|
| **1** | **Project Setup** — Electron + React + Vite 프로젝트 생성, 빌드/실행 확인 | ✅ 완료 |
| **2** | **Asset Browser** — 파일 트리, 미리보기, Import, Registry 읽기/쓰기 | ✅ 구현 |
| **3** | **Play/Stop** — `Voradorix.exe` spawn/kill | ✅ 구현 |
| **4** | **Script Editor** (Phase 3-1) — 문법 하이라이팅, @label 네비게이션 | 예정 |
| **5** | **Widget Designer** (Phase 3-2) — 위젯 트리 JSON 편집 | 예정 |

---

## 프로세스 아키텍처

```
┌─────────────────────────────────────────────────────────────┐
│  Electron Main Process                                      │
│  ├─ BrowserWindow 생성                                       │
│  ├─ gameLauncher.ts — spawn/kill Voradorix.exe              │
│  └─ ipcMain — Renderer와의 통신 채널                         │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Renderer Process (React)                            │   │
│  │  ├─ AssetBrowser — 파일 트리, 미리보기, Import         │   │
│  │  ├─ PlayButton — ▶ 실행 / ■ 중지                      │   │
│  │  ├─ ScriptEditor — (Phase 4)                         │   │
│  │  └─ WidgetDesigner — (Phase 5)                       │   │
│  └──────────────────────────────────────────────────────┘   │
│                                                              │
│  Node.js File System (fs)                                    │
│  ├─ Assets/ ←→ AssetRegistry.json                           │
│  └─ build/Voradorix.exe                                    │
└─────────────────────────────────────────────────────────────┘
         │ spawn()
         ▼
┌─────────────────────────────────────────────────────────────┐
│  Voradorix.exe (C++ / SFML)                                 │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  CVrdxApplication                                    │   │
│  │  ├─ CVrdxAssetManager (싱글톤)                        │   │
│  │  ├─ Widget Tree (NovelScene, DialogueBox, ...)        │   │
│  │  └─ AssetRegistry.json (읽기 전용 or 충돌 회피)       │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## 데이터 연동

| 데이터 | 위치 |读写 주체 | 비고 |
|--------|------|----------|------|
| `AssetRegistry.json` | `Assets/` | Editor: 读写, Engine: 讀 | 엔진은 시작 시 읽고, Editor가 수정하면 재시작或 ReloadRegistry() |
| 에셋 파일 | `Assets/` | Editor: Import/삭제, Engine: Load | 엔진 시작 시점의 파일 상태 기준 |
| `Voradorix.exe` | `Game/Editor/build/` | Post-Build 생성 | Debug/Release 동일 파일명 |
| 세이브 파일 | `Game/Saves/` | Engine: 读写 | 에디터와 무관 |

---

## 기술 스택

| 항목 | 선택 | 비고 |
|------|------|------|
| 프레임워크 | Electron | Chromium + Node.js |
| UI 라이브러리 | React | 미정 (Vue/Svelte 검토 가능) |
| 언어 | TypeScript | Electron/React 모두 |
| 번들러 | Vite | `vite-electron-builder` 또는 `electron-vite` |
| CSS | Tailwind CSS (제안) | UI 빠른 프로토타이핑 |
| 파일 시스템 | Node.js `fs` | Asset I/O |

---

## 구현 순서 상세

각 Phase의 상세 명세는 별도 문서 참조:

- [[Editor/1-ProjectSetup.md]] — Electron 프로젝트 생성, Vite + React 설정
- [[Editor/2-AssetBrowser.md]] — Asset Browser 기능 명세
- [[Editor/3-ScriptEditor.md]] — Script Editor 기능 명세 (Phase 4)
- [[Editor/4-WidgetDesigner.md]] — Widget Designer 기능 명세 (Phase 5)
- [[Editor/ARCHITECTURE.md]] — Electron 프로세스 구조, IPC, spawn 상세
