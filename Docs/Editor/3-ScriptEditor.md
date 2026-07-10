---
title: "Editor Phase 3-1: Script Editor"
project: Voradorix
tags:
  - voradorix
  - editor
  - script-editor
  - spec
status: implemented
---

# Editor Phase 3-1: Script Editor

> **목표**: @label/@jump 기반 스크립트를 편집할 수 있는 전용 에디터 구현  
> **의존성**: Phase 2 (Asset Browser) 완료  
> **상태**: 구현 완료

---

## 1. 개요

Script Editor는 `Assets/Scripts/` 디렉토리의 `.txt` 스크립트 파일을 편집하는 도구입니다.
텍스트 에디터의 기본 기능에 더해, Voradorix 스크립트 문법에 특화된 기능을 제공합니다.

---

## 2. 핵심 기능

### 2.1 기본 편집 기능
- 구문 강조 (Syntax Highlighting) — `@` 명령어, `"문자열"`, `@label` 태그
- 라인 번호 표시
- 다중 커서/Undo/Redo

### 2.2 Voradorix 특화 기능
- **@label 네비게이션** — 문서 내 @label 목록을 사이드바에 표시, 클릭 시 해당 라인으로 이동
- **@jump 미리보기** — @jump 대상 레이블이 존재하는지 검증, 존재하지 않으면 경고 표시
- **선택지 연결 보기** — @choice 분기 구조를 시각적으로 표시

### 2.3 유효성 검증
- 존재하지 않는 레이블로의 @jump 감지
- 형식이 잘못된 명령어 강조
- 파일 저장 전 기본 검증

### 2.4 현재 구현된 동작
- `.txt` 파일 선택 시 전용 Script Editor 렌더링
- Alias 편집 지원
- 저장/자동저장 지원
- `@label`, `@jump`, `@choice` 검증
- 도달 불가 블럭 및 잠재적 무한루프 탐지
- 블럭 색상띠 및 줄번호 0-based 표시
- `@` 입력 자동완성 및 스니펫 삽입
- 커서 인근 팝업, 스크롤 추적, 위/아래 방향 배치

---

## 3. 고려사항

- 현재는 커스텀 textarea 기반 구현
- 향후 다중 커서, 더 정교한 구문 강조가 필요하면 CodeMirror/Monaco 재검토 가능

---

## 4. 참고 자료

- 스크립트 문법: [[Engine/4-ScriptEngine.md]]
- 현재 테스트 스크립트: `Assets/Scripts/TestScript.txt`
