---
title: 작업 기록
project: Voradorix
tags:
  - voradorix
  - log
---
# 작업 기록

## 2026-06-30 (Later)

### 3단계 명세서 작성

- `Docs/4-BackgroundCharacter.md` — 배경 전환 및 캐릭터 슬롯 관리 명세 작성
- `Docs/Voradorix.md` — 3단계 항목에 명세서 링크 추가

### 빌드/테스트 및 문서 갱신

- 2단계 기반 타입/씬 흐름 구현 확인
- 빌드/테스트 완료 후 문서 상태 갱신
- `README.md` — 현재 구현 단계와 프로젝트 구조 갱신
- `Docs/Voradorix.md` — 인덱스 문서 상태를 구현 완료 기준으로 갱신

### Visual Studio 솔루션 구성

- `Game/Game.sln` — Visual Studio 2022 솔루션 (x64, v143)
- `Game/Game/Game.vcxproj` — 프로젝트 설정
  - 플랫폼: x64, 툴셋: v143, 언어: C++17
  - SFML 동적 링크 (`SFML_DYNAMIC`)
  - 서브시스템: Windows (콘솔 창 없음)
  - Debug 링크: `sfml-*-d.lib` + `sfml-main-d.lib`
  - Release 링크: `sfml-*.lib` + `sfml-main.lib`
  - Post-Build: SFML DLL 자동 복사
- `Game/extern/SFML-3.1.0/` — SFML 라이브러리 복제본

### 소스 코드

- `Game/Game/Src/Main.cpp`
  - 800x600 창, 초록색 원 회전 애니메이션
  - FPS 출력, ESC/닫기 이벤트 처리
  - 인코딩: UTF-8 BOM
  - 네이밍: PascalCase

### `.gitignore`

- `bin/`, `obj/`, `.vs/` 등 빌드 산물 제외

### Scene 시스템 1단계 완료

- 파일명 변경: `VrdxScene.h/cpp` → `Scene.h/cpp`, `VrdxSceneManager.h/cpp` → `SceneManager.h/cpp`
- `Common.h`: `<type_traits>`/`<memory>` 추가, `MakeVrdxUnique` 추가
- `Vector.h`: `Container.Begin()` → `Container.begin()` 버그 수정
- `Scene.h`: `WantsExit()`/`RequestExit()`/`bWantExit` 추가 (Scene-SceneManager 디커플링)
- `SceneManager.cpp`: Push 재구성 (OnEnter → Add 순서), HandleEvent에 WantsExit 감지 + Pop 처리
- `Application.h/cpp`: `CVrdxApplication` 생성 (Window, SceneManager, Run 루프)
- `TestScene.h/cpp`: `CVrdxTestScene` — ESC → RequestExit, 정중앙 파란 원
- `Main.cpp`: Application 생성/실행으로 리팩터 완료
- `Game.vcxproj`: 모든 파일 등록 완료 (Core/ + Scene/ 필터)
- 프로젝트명 **Voradorix** 명명
- `README.md` 작성

### 2단계 준비 — 명세 및 기반 타입

- `README.md`: Docs/ 및 opencode.json 참조 제거 (git-tracked 파일만 표시)
- `Game.vcxproj`: `$(ProjectDir)Src;` include path 추가 → `#include "Core/Common.h"` 형태 사용 가능
- 소스 파일 include 경로 단축: `../Core/Common.h` → `Core/Common.h` 적용 완료
- `Docs/2-NovelScene.md` — 2단계 명세 작성
  - `CVrdxBaseWidget` (Ui/BaseWidget.h) — UI 위젯 베이스 클래스
  - `CDialogueBox` (Ui/DialogueBox.h/cpp) — CVrdxBaseWidget 상속, 하단 대사창
  - `CNovelScene` (Novel/NovelScene.h/cpp) — 대사 목록 순회, DialogueBox 소유
  - DialogueBox는 Scene 시스템에 대한 의존성 없음 (순수 위젯)
- `Docs/3-String.md` — 유니코드 문자열 클래스 명세 작성
- `Core/String.h` + `Core/String.cpp` — `FVrdxString` 구현 완료
  - 내부 UTF-32 (`std::u32string`) 저장, UTF-8 입출력
  - `Left(N)` 타이핑 애니메이션 지원, SFML `sf::String` 변환
  - overlong/surrogate/truncation 검증 포함
- `Assets/Fonts/malgun.ttf` — Malgun Gothic 폰트 설치 (13.5MB)

---

## 비주얼 노벨 엔진 — 구현 계획 (8단계)

### 디렉토리 구조

```
Game/Game/Src/
├── Core/          # Application, Common, Vector, String
├── Scene/         # Scene 인터페이스, SceneManager, 각 씬
├── Novel/         # NovelScene, ScriptEngine, CharacterManager
├── Ui/            # BaseWidget, DialogueBox, Button, TextBox, Menu
└── Save/          # SaveManager (JSON 기반 세이브/로드)

Game/Game/Assets/
├── Scripts/       # 시나리오 텍스트 파일
├── Backgrounds/   # 배경 이미지
├── Characters/    # 캐릭터 스프라이트
├── Bgm/           # 배경 음악
├── Se/            # 효과음
├── Fonts/         # malgun.ttf (Malgun Gothic)
└── Ui/            # 버튼, 텍스트박스 프레임 등 UI 리소스
```

### 단계별 구현 로드맵

| 단계 | 내용 | 핵심 파일 |
|------|------|-----------|
| **1단계** | Scene 인터페이스 + SceneManager 도입, main.cpp 리팩터 | `Scene.h`, `SceneManager.h/cpp`, `Application.h/cpp` |
| **2단계** | NovelScene + BaseWidget + DialogueBox + FVrdxString (명세 완료, 코드 진행 중) | `NovelScene.h/cpp`, `BaseWidget.h`, `DialogueBox.h/cpp`, `String.h/cpp` |
| **3단계** | Background 전환, CharacterManager (스프라이트 표시/전환) | `Background.h/cpp`, `CharacterManager.h/cpp` |
| **4단계** | ScriptEngine — 텍스트 스크립트 파서/실행기 | `ScriptEngine.h/cpp` |
| **5단계** | ChoiceManager — 선택지 UI 및 분기 처리 | `ChoiceManager.h/cpp` |
| **6단계** | SaveManager — 세이브/로드 | `SaveManager.h/cpp` |
| **7단계** | TitleScene, ConfigScene — 메뉴 구성 | `TitleScene.h/cpp`, `ConfigScene.h/cpp` |
| **8단계** | EffectManager — 페이드, 셰이크 등 연출 효과 | `EffectManager.h/cpp` |

### 스크립트 포맷 (제안)

```
bg classroom.jpg
bgm happy_theme.mp3 loop
show yuki smile center
yuki "어서 와! 기다리고 있었어."
hide yuki
show yuki angry center
yuki "…근데 왜 늦었어?"
fadeout 1.0
choice "미안해" → chapter2 "변명하지마" → chapter3
```

### 에이전트 규칙 (AGENTS.md)

- 에이전트는 직접 코드를 작성/수정하지 않으며, 코드 리뷰/설계 검토/조언 역할로 한정.
- 파일 생성 및 설정, 작업 기록 관리 등 코드 외 업무는 수행 가능.
