---
title: Voradorix 프로젝트
tags:
  - voradorix
  - project
  - index
status: active
phase: 7 (Save/Load 완료)
---

# Voradorix

SFML 3.1.0 기반 2D 비주얼 노벨 엔진

## 개요

- **기술**: C++17, SFML 3.1.0, Visual Studio 2022 x64
- **GitHub**: https://github.com/DCode1119/Voradorix
- **상태**: 7단계 Save/Load 완료

## 구현 현황

- [x] **1단계 — Scene 시스템** (완료)
  - Scene 인터페이스 (`CVrdxScene`) — OnEnter/OnExit/HandleEvent/Update/Draw + WantsExit 디커플링
  - SceneManager (`CVrdxSceneManager`) — Push/Pop/Switch + 프레임 이벤트 처리 (shared_ptr 기반)
  - Application (`CVrdxApplication`) — Window + SceneManager + Run 루프
  - TestScene (`CVrdxTestScene`) — 검증용 (중앙 파란 원, ESC 종료)
  - Core (`Common.h`, `Vector.h`) — 타입 alias, TVrdxVector, VrdxMove, 매크로

- [x] **2단계 — NovelScene + DialogueBox + String 기반** (구현 및 검증 완료)
  - [[String.md]] — `FVrdxString` 유니코드 문자열 (UTF-32, UTF-8 입출력, SFML 연동)
  - `Ui/WidgetBase.h` — `CVrdxWidgetBase` 위젯 베이스 클래스
  - `Ui/DialogueBox.h/cpp` — `CVrdxDialogueBox` (타이핑 애니메이션, Speaker/Line 분리)
  - `Scene/NovelScene.h/cpp` — `CVrdxNovelScene` (Background + CharacterManager + DialogueBox + ScriptEngine 통합)
  - `Assets/Fonts/malgun.ttf` — Malgun Gothic 폰트

- [x] **3단계 — 배경 및 캐릭터** (Background + CharacterManager 구현 완료)
  - [[3-BackgroundCharacter.md]] — 배경 전환 + 캐릭터 슬롯/표정 관리 명세
  - `Novel/Background.h/cpp` — `CVrdxBackground` (배경 로드, 페이드 전환, 전체화면 스케일)
  - `Novel/CharacterManager.h/cpp` — `CVrdxCharacterManager` (좌/중/우 슬롯, 페이드 전환, 텍스처 캐싱)
- [x] **4단계 — ScriptEngine** (완료)
  - [[4-ScriptEngine.md]] — 스크립트 로드/파싱/실행 구현
  - `ScriptEngine.h/cpp` — `weak_ptr<NovelScene>` 기반 명령 실행기, 8개 명령어 지원
  - `ScriptLine.h/cpp` — 명령어별 파싱/Construct/Dispatch, 팩토리 테이블 패턴
  - `NovelScene.h/cpp` — `shared_from_this()`로 ScriptEngine 연결, `CanAdvance()`로 흐름 제어
  - `Assets/Scripts/TestScript.txt` — @label/@jump 분기 포함 테스트 스크립트
- [x] **5단계 — 선택지 시스템** (구현 완료)
  - [[5-ChoiceSystem.md]] — 선택지 UI/분기 시스템 구현 문서
  - `ChoiceWidget.h/cpp` — 선택지 패널/버튼/입력 처리/즉시 분기
  - `ScriptLine.h/cpp` — `@choice` 파싱 및 분기용 라인 추가
  - `NovelScene.h/cpp` — ChoiceWidget 소유 및 ScriptEngine 분기 연동
  - `Assets/Scripts/TestScript.txt` — 선택지 검증용 샘플 스크립트
- [ ] **6단계 — UI Foundation** (진행 중)
  - [[6-UIFoundation.md]] — 공통 UI 위젯/컨테이너 계층 명세
  - `Ui/WidgetBase.h/cpp` — 공통 위젯 트리/입력/렌더 계층
  - `Ui/BoxWidget.h/cpp`, `Ui/TextLabel.h/cpp` — 기본 UI 위젯 구현
  - `Ui/Button.h/cpp` — 버튼 상호작용 위젯 (핵심 구현 완료, 폴리싱 예정)
  - `Ui/DialogueBox.h/cpp`, `Ui/ChoiceWidget.h/cpp` — 위젯 기반 UI 정리
- [x] **7단계 — 세이브/로드** (완료)
  - [[7-SaveLoad.md]] — 세이브/로드 기능 명세
  - `Scene/SceneManager.h/cpp` — `Save()` / `Load()` 추가 (const dynamic_cast, NovelScene 상태 직렬화)
  - `Scene/NovelScene.h/cpp` — `Save()` / `Load()` 구현, `FVrdxNovelSceneSaveData` 구조체
  - `Novel/CharacterManager.h/cpp` — `GetSaveData()` / `Reset()` 추가, `FVrdxCharacterSlotSaveData`
  - `Extern/nlohmann/json.hpp` — JSON 직렬화 (nlohmann/json v3.12.0)
  - `Saves/` — 저장 파일 디렉토리 (실행 경로 기준)
- [ ] **8단계 — 메뉴 구성** (예정)
- [ ] **9단계 — 연출 효과** (예정)

## 추가 문서

- [[6-UIFoundation.md]] — 공통 UI 위젯/컨테이너 계층 명세
- [[BACKLOG.md]] — 단계별 보류/폴리싱 항목 관리

## 네이밍 규칙

Unreal Engine 스타일: `C`/`T`/`E`/`I`/`F` 접두어 + `Vrdx` 프로젝트 접두어 + PascalCase. `m_` 접두사 없음.

## 문서

- [[1-SceneSystem.md]] — 1단계 Scene 시스템 명세
- [[2-NovelScene.md]] — 2단계 NovelScene + DialogueBox 명세
- [[String.md]] — FVrdxString 유니코드 문자열 명세
- [[GAME_DESIGN.md]] — 전체 설계
- [[NAMING.md]] — 네이밍 규칙 상세
- [[WORK_LOG.md]] — 작업 기록
- [[BACKLOG.md]] — 백로그
