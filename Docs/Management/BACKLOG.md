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

> **2026-07-10: 프로젝트 방향 전환** — 게임 개발 → 게임 엔진 + 에디터 개발
> EffectManager 등 게임 콘텐츠 관련 항목은 후순위로 조정됨.
> 새 로드맵은 [[Design/PROJECT_DIRECTION.md]] 참조.

## 분류 기준

- **Now**: 현재 단계에서 바로 진행 중인 항목
- **Later**: 다음 큰 기능 단계에서 처리할 항목
- **Polish**: 핵심 동작은 끝났고 세부 개선만 남은 항목
- **On Hold**: 프로젝트 방향 전환으로 인해 보류된 항목

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
- `ConfigScene` 적용

## 7단계 — Save/Load

- [x] `NovelScene` 상태 직렬화/복원
- [x] `SaveManager` — SceneManager.Save()/Load()에 통합

## 8단계 — Menu

- [x] `TitleWindow` — New Game / Continue
- [x] `SaveLoadWindow` — 오버레이, 저장/불러오기 모드
- [ ] `ConfigScene` — (추후)

## 9단계 — Effect (On Hold)

프로젝트 방향 전환으로 인해 보류. 엔진+에디터 기반이 안정화된 후 재개.

- `EffectManager`
- 페이드 / 셰이크 등 연출 효과

---

## Phase 1 — AssetManager (C++ Engine)

AssetManager 코어 구축. 싱글톤 독립 모듈, GUID 기반 에셋 식별.

- [ ] `Core/AssetManager.h/cpp` — 클래스 골격 + 싱글톤
- [ ] `LoadRegistry()` / `SaveRegistry()` — AssetRegistry.json I/O
- [x] `LoadFont()` — 폰트 로딩 + 캐싱 (Phase 1 목표)
- [ ] `LoadTexture()` — 텍스처 로딩 + 캐싱
- [ ] `ImportAsset()` — 파일 복사 + GUID 생성 + Registry 등록
- [x] 기존 폰트 로딩 코드 3곳 리팩터 (DialogueBox, TextLabel, ChoiceWidget)
- [ ] Background 텍스처 → AssetManager 경유
- [ ] CharacterManager TextureCache → AssetManager 이관

## Phase 2 — Electron Editor

Electron 기반 에디터. Asset Browser 우선 개발.

- [ ] Electron 프로젝트 생성 (Vite + React)
- [ ] Asset Browser — 파일 트리
- [ ] Asset Browser — 미리보기
- [ ] Asset Browser — Import (파일 복사 + Registry 등록)
- [ ] AssetRegistry.json 읽기/쓰기
- [ ] Play/Stop 버튼 → Game.exe spawn/kill
- [ ] build/gamePath.json 생성 (Post-Build)

## Phase 3 — Editor 확장

- [ ] Script Editor (문법 하이라이팅, @label 네비게이션)
- [ ] Widget Designer (위젯 트리 JSON 편집)

## 관리 규칙

- 핵심 구현이 끝나면 해당 항목을 `Polish`로 이동
- 다음 단계로 넘길 항목은 `Later`로 이동
- 로그나 자원 관리처럼 프로젝트 전반에 영향을 주는 항목은 별도 섹션으로 유지
