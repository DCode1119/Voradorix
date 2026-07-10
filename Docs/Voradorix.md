---
title: Voradorix 프로젝트
tags:
  - voradorix
  - project
  - index
status: active
phase: "전환 — 엔진 + 에디터 개발 (AssetManager 우선)"
---

# Voradorix

SFML 3.1.0 기반 2D 비주얼 노벨 엔진 + Electron 에디터

## 개요

- **기술**: C++17, SFML 3.1.0, Visual Studio 2022 x64, Electron
- **GitHub**: https://github.com/DCode1119/Voradorix
- **상태**: 게임 개발 → **엔진 + 에디터 개발**로 방향 전환 ([[Design/PROJECT_DIRECTION.md]])
  - 엔진 런타임: 8단계(메뉴 구성)까지 구현 완료
  - 현재 우선순위: AssetManager 구축 → Electron 에디터

## 구현 현황

- [x] **1단계 — Scene 시스템** (역사적, 8단계에서 Widget Tree로 대체)
  - Core (`Common.h`, `Vector.h`) — 타입 alias, TVrdxVector, VrdxMove, 매크로

- [x] **2단계 — NovelScene + DialogueBox + String 기반** (구현 및 검증 완료)
  - [[Engine/String.md]] — `FVrdxString` 유니코드 문자열 (UTF-32, UTF-8 입출력, SFML 연동)
  - `Ui/WidgetBase.h` — `CVrdxWidgetBase` 위젯 베이스 클래스
  - `Novel/DialogueBox.h/cpp` — `CVrdxDialogueBox` (타이핑 애니메이션, Speaker/Line 분리)
  - `Novel/NovelScene.h/cpp` — `CVrdxNovelScene` (Background + CharacterManager + DialogueBox + ScriptEngine 통합)
  - `Assets/Fonts/malgun.ttf` — Malgun Gothic 폰트

- [x] **3단계 — 배경 및 캐릭터** (Background + CharacterManager 구현 완료)
  - [[Engine/3-BackgroundCharacter.md]] — 배경 전환 + 캐릭터 슬롯/표정 관리 명세
  - `Novel/Background.h/cpp` — `CVrdxBackground` (배경 로드, 페이드 전환, 전체화면 스케일, WidgetBase 상속)
  - `Novel/CharacterManager.h/cpp` — `CVrdxCharacterManager` (좌/중/우 슬롯, 페이드 전환, 텍스처 캐싱, WidgetBase 상속)
- [x] **4단계 — ScriptEngine** (완료)
  - [[Engine/4-ScriptEngine.md]] — 스크립트 로드/파싱/실행 구현
  - `ScriptEngine.h/cpp` — `weak_ptr<NovelScene>` 기반 명령 실행기, 8개 명령어 지원
  - `ScriptLine.h/cpp` — 명령어별 파싱/Construct/Dispatch, 팩토리 테이블 패턴
  - `NovelScene.h/cpp` — `shared_from_this()`로 ScriptEngine 연결, `CanAdvance()`로 흐름 제어
  - `Assets/Scripts/TestScript.txt` — @label/@jump 분기 포함 테스트 스크립트
- [x] **5단계 — 선택지 시스템** (구현 완료)
  - [[Engine/5-ChoiceSystem.md]] — 선택지 UI/분기 시스템 구현 문서
  - `Novel/ChoiceWidget.h/cpp` — 선택지 패널/버튼/입력 처리/즉시 분기 (WidgetBase 상속)
  - `ScriptLine.h/cpp` — `@choice` 파싱 및 분기용 라인 추가
  - `NovelScene.h/cpp` — ChoiceWidget 소유 및 ScriptEngine 분기 연동
  - `Assets/Scripts/TestScript.txt` — 선택지 검증용 샘플 스크립트
- [x] **6단계 — UI Foundation** (핵심 구현 완료, 폴리싱 예정)
  - [[Engine/6-UIFoundation.md]] — 공통 UI 위젯/컨테이너 계층 명세
  - `Ui/WidgetBase.h/cpp` — 공통 위젯 트리/입력/렌더 계층
  - `Ui/BoxWidget.h/cpp`, `Ui/TextLabel.h/cpp` — 기본 UI 위젯 구현
  - `Ui/Button.h/cpp` — 버튼 상호작용 위젯 (핵심 구현 완료, 폴리싱 예정)
- [x] **7단계 — 세이브/로드** (완료)
  - [[Engine/7-SaveLoad.md]] — 세이브/로드 기능 명세
  - `Ui/Application.h/cpp` — `Save()` / `Load()` (Children 순회 + dynamic_pointer_cast)
  - `Novel/NovelScene.h/cpp` — `Save()` / `Load()` 구현, `FVrdxNovelSceneSaveData` 구조체
  - `Novel/CharacterManager.h/cpp` — `GetSaveData()` / `Reset()` 추가, `FVrdxCharacterSlotSaveData`
  - `Extern/nlohmann/json.hpp` — JSON 직렬화 (nlohmann/json v3.12.0)
  - `Saves/` — 저장 파일 디렉토리 (실행 경로 기준)
- [x] **구조 변경** (Widget Tree 기반 아키텍처 전환 완료, [[Engine/StructureReform.md]])
  - Scene/SceneManager/TestScene 전면 삭제
  - 모든 컴포넌트 `CVrdxWidgetBase` 상속으로 통일
  - `BringToFront()` 추가로 Z-Order 관리
  - 파일 구조 정리 (Novel 전용 / Ui 공통 분리)
- [x] **8단계 — 메뉴 구성** (TitleWindow / SaveLoadWindow 구현 완료)
  - `Novel/TitleWindow.h/cpp` — `CVrdxTitleWindow` (New Game / Continue Game 버튼, 델리게이트)
  - `Novel/SaveLoadWindow.h/cpp` — `CVrdxSaveLoadWindow` (10개 슬롯, Save/Load 모드 전환)
  - `Novel/NovelScene.h/cpp` — `ResetScriptEngine()` 추가 (New Game 재시작 지원)
  - `Main.cpp` — 메뉴 → NovelScene 전환 흐름 구성
  - Save/Load 경로 버그 수정 (`"Saves/" + Filename`)
- [ ] **9단계 — EffectManager** (연출 효과, [[Engine/9-EffectManager.md]] 명세 작성 완료, **보류**)
  - 프로젝트 방향 전환으로 인해 후순위로 미뤄짐 ([[Design/PROJECT_DIRECTION.md]])
  - 페이드인/페이드아웃, 셰이크, 컬러 오버레이
  - 효과 큐 시스템
  - `@fadein`, `@fadeout`, `@shake`, `@coloroverlay` 스크립트 명령어

## 방향 전환 — 엔진 + 에디터 개발

프로젝트의 방향을 **게임 개발**에서 **게임 엔진 + 에디터 개발**로 전환하였습니다.
자세한 사항은 [[Design/PROJECT_DIRECTION.md]] 참조.

### 신규 계획

- [ ] **Phase 1 — AssetManager** (C++, 싱글톤 독립 모듈)
  - GUID 기반 에셋 식별, `AssetRegistry.json` 레지스트리
  - `LoadFont()` → 폰트 중복 로딩 제거 (1차 목표) ✅
  - `LoadTexture()`, `ImportAsset()` (2차 목표)
  - `GetScriptPath()` 및 배경/캐릭터 텍스처 경로 이관 ✅
- [ ] **Phase 2 — Electron Editor** (Electron + React, [[Editor/INDEX.md]])
  - Asset Browser (파일 트리, 미리보기, Import)
  - Play/Stop 버튼 (Voradorix.exe spawn)
  - 위치: `Game/Editor/`

## 문서 구조

```
Docs/
├── Voradorix.md              (인덱스)
├── WORK_LOG.md               (작업 기록)
├── Engine/                   (엔진 기능명세)
│   ├── 1-SceneSystem.md ~ 8-Menu.md
│   ├── 9-EffectManager.md   (보류)
│   ├── String.md
│   └── StructureReform.md
├── Editor/                   (에디터 기능명세 - 신규)
│   ├── INDEX.md              (개요 및 로드맵)
│   ├── 1-ProjectSetup.md
│   ├── 2-AssetBrowser.md
│   ├── 3-ScriptEditor.md
│   ├── 4-WidgetDesigner.md
│   └── ARCHITECTURE.md
├── Design/                   (설계문서)
│   ├── GAME_DESIGN.md
│   ├── PROJECT_DIRECTION.md
│   ├── PROJECT_STRUCTURE.md
│   ├── NAMING.md
│   └── INCLUDE_CONVENTION.md
└── Management/               (아이디어/백로그)
    └── BACKLOG.md
```

### 엔진 기능명세 ([[Engine/]])


- [[Engine/6-UIFoundation.md]] — 공통 UI 위젯/컨테이너 계층 명세
- [[Engine/StructureReform.md]] — Scene → Widget Tree 구조 변경 명세
- [[Design/PROJECT_STRUCTURE.md]] — opencode/Git 저장소 분리 구조
- [[Design/INCLUDE_CONVENTION.md]] — include 전처리문 규칙
- [[Management/BACKLOG.md]] — 단계별 보류/폴리싱 항목 관리
- [[Design/PROJECT_DIRECTION.md]] — 엔진 + 에디터 방향 전환 명세

## 네이밍 규칙

Unreal Engine 스타일: `C`/`T`/`E`/`I`/`F` 접두어 + `Vrdx` 프로젝트 접두어 + PascalCase. `m_` 접두사 없음.

## 문서

- [[Engine/1-SceneSystem.md]] — 1단계 Scene 시스템 명세
- [[Engine/2-NovelScene.md]] — 2단계 NovelScene + DialogueBox 명세
- [[Engine/3-BackgroundCharacter.md]] — 3단계 배경/캐릭터 명세
- [[Engine/4-ScriptEngine.md]] — 4단계 ScriptEngine 명세
- [[Engine/5-ChoiceSystem.md]] — 5단계 선택지 시스템 명세
- [[Engine/6-UIFoundation.md]] — 6단계 UI Foundation 명세
- [[Engine/7-SaveLoad.md]] — 7단계 Save/Load 명세
- [[Engine/8-Menu.md]] — 8단계 메뉴 구성 명세
- [[Engine/StructureReform.md]] — 구조 변경 명세
- [[Engine/String.md]] — FVrdxString 유니코드 문자열 명세
- [[Design/GAME_DESIGN.md]] — 전체 설계
- [[Design/NAMING.md]] — 네이밍 규칙 상세
- [[Design/PROJECT_STRUCTURE.md]] — opencode/Git 구조
- [[Design/INCLUDE_CONVENTION.md]] — include 규칙
- [[WORK_LOG.md]] — 작업 기록
- [[Management/BACKLOG.md]] — 백로그
- [[Design/PROJECT_DIRECTION.md]] — 방향 전환 명세
