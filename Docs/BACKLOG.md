---
title: 백로그
project: Voradorix
tags:
  - voradorix
  - backlog
  - todo
status: active
---

# Voradorix — 백로그

현재 단계에서 미루거나, 다음 단계로 넘기거나, 폴리싱으로 분리한 항목을 모아둔 문서.

## 분류 기준

- **Now**: 현재 단계에서 바로 진행 중인 항목
- **Later**: 다음 큰 기능 단계에서 처리할 항목
- **Polish**: 핵심 동작은 끝났고 세부 개선만 남은 항목

## 2단계 이후 보류 항목

- `NovelScene` / `DialogueBox` 자원 관리를 `AssetManager`로 분리
- 폰트/이미지/기타 리소스 로드 실패에 대한 공통 로그 처리

## 4단계 — ScriptEngine 후속 항목

- 일반 텍스트 대사 지원 (`speaker "text"`, `"text"`)
- `@bgm` / `@se`
- `@if` / `@set`
- 스크립트 실패 시 로그 출력 체계

## 5단계 — ChoiceSystem 후속 항목

- 공통 로그 계층 연동
- 선택지 실패/예외 경로의 진단 메시지 정리

## 6단계 — UI Foundation 폴리싱

- `TextLabel` 폰트 자원 관리 분리
- `Button` Hover 상태
- `Button` Focus 상태
- `Button` Keyboard Activate 상태
- `Button` 텍스트 정렬/서식
- `TitleScene` / `ConfigScene` / `SaveLoadScene` 적용

## 7단계 — Save/Load

- [x] `NovelScene` 상태 직렬화/복원
- [x] `SaveManager` — SceneManager.Save()/Load()에 통합
- [ ] `SaveLoadScene` — 저장/불러오기 UI 씬 (별도 작업 예정)

## 8단계 — Menu

- `TitleScene`
- `ConfigScene`

## 9단계 — Effect

- `EffectManager`
- 페이드 / 셰이크 등 연출 효과

## 관리 규칙

- 핵심 구현이 끝나면 해당 항목을 `Polish`로 이동
- 다음 단계로 넘길 항목은 `Later`로 이동
- 로그나 자원 관리처럼 프로젝트 전반에 영향을 주는 항목은 별도 섹션으로 유지
