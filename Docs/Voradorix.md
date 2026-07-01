---
title: Voradorix 프로젝트
tags:
  - voradorix
  - project
  - index
status: active
phase: 3 (Background + CharacterManager 완료)
---

# Voradorix

SFML 3.1.0 기반 2D 비주얼 노벨 엔진

## 개요

- **기술**: C++17, SFML 3.1.0, Visual Studio 2022 x64
- **GitHub**: https://github.com/DCode1119/Voradorix
- **상태**: 3단계 Background + CharacterManager 구현 완료

## 구현 현황

- [x] **1단계 — Scene 시스템** (완료)
  - Scene 인터페이스 (`CVrdxScene`) — OnEnter/OnExit/HandleEvent/Update/Draw + WantsExit 디커플링
  - SceneManager (`CVrdxSceneManager`) — Push/Pop/Switch + 프레임 이벤트 처리
  - Application (`CVrdxApplication`) — Window + SceneManager + Run 루프
  - TestScene (`CVrdxTestScene`) — 검증용 (중앙 파란 원, ESC 종료)
  - Core (`Common.h`, `Vector.h`) — 타입 alias, TVrdxVector, VrdxMove, 매크로

- [x] **2단계 — NovelScene + DialogueBox + String 기반** (구현 및 검증 완료)
  - [[3-String.md]] — `FVrdxString` 유니코드 문자열 (UTF-32, UTF-8 입출력, SFML 연동)
  - `Ui/BaseWidget.h` — `CVrdxBaseWidget` 위젯 베이스 클래스
  - `Ui/DialogueBox.h/cpp` — `CDialogueBox`
  - `Novel/NovelScene.h/cpp` — `CVrdxNovelScene`
  - `Assets/Fonts/malgun.ttf` — Malgun Gothic 폰트

- [x] **3단계 — 배경 및 캐릭터** (Background + CharacterManager 구현 완료)
  - [[4-BackgroundCharacter.md]] — 배경 전환 + 캐릭터 슬롯/표정 관리 명세
  - `Novel/Background.h/cpp` — `CVrdxBackground` (배경 로드, 페이드 전환, 전체화면 스케일)
  - `Novel/CharacterManager.h/cpp` — `CVrdxCharacterManager` (좌/중/우 슬롯, 페이드 전환, 텍스처 캐싱)
- [ ] **4단계 — ScriptEngine** (예정)
- [ ] **5단계 — 선택지 시스템** (예정)
- [ ] **6단계 — 세이브/로드** (예정)
- [ ] **7단계 — 메뉴 구성** (예정)
- [ ] **8단계 — 연출 효과** (예정)

## 네이밍 규칙

Unreal Engine 스타일: `C`/`T`/`E`/`I`/`F` 접두어 + `Vrdx` 프로젝트 접두어 + PascalCase. `m_` 접두사 없음.

## 문서

- [[1-SceneSystem.md]] — 1단계 Scene 시스템 명세
- [[2-NovelScene.md]] — 2단계 NovelScene + DialogueBox 명세
- [[3-String.md]] — FVrdxString 유니코드 문자열 명세
- [[GAME_DESIGN.md]] — 전체 설계
- [[NAMING.md]] — 네이밍 규칙 상세
- [[WORK_LOG.md]] — 작업 기록
