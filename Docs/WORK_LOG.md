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

- `Docs/3-BackgroundCharacter.md` — 배경 전환 및 캐릭터 슬롯 관리 명세 작성
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
  - `CVrdxWidgetBase` (Ui/WidgetBase.h) — UI 위젯 베이스 클래스
  - `CVrdxDialogueBox` (Ui/DialogueBox.h/cpp) — CVrdxWidgetBase 상속, 하단 대사창
  - `CNovelScene` (Novel/NovelScene.h/cpp) — 대사 목록 순회, DialogueBox 소유
  - DialogueBox는 Scene 시스템에 대한 의존성 없음 (순수 위젯)
- `Docs/String.md` — 유니코드 문자열 클래스 명세 작성
- `Core/String.h` + `Core/String.cpp` — `FVrdxString` 구현 완료
  - 내부 UTF-32 (`std::u32string`) 저장, UTF-8 입출력
  - `Left(N)` 타이핑 애니메이션 지원, SFML `sf::String` 변환
  - overlong/surrogate/truncation 검증 포함
- `Assets/Fonts/malgun.ttf` — Malgun Gothic 폰트 설치 (13.5MB)

## 2026-07-01

### 3단계 — CVrdxBackground 구현

- `Src/Novel/Background.h` + `Src/Novel/Background.cpp` — `CVrdxBackground` 구현 완료
  - 생성자: `CVrdxBackground(float TargetWidth, float TargetHeight)` — 출력 해상도 기반, 1×1 투명 텍스처로 초기화
  - `SetBackground(FVrdxString& AssetName, float TransitionSeconds)` — `Assets/Backgrounds/`에서 이미지 로드
    - **중복 요청 무시**: 전환 중 아닐 때 동일 CurrentAssetName, 전환 중일 때 동일 NextAssetName → 로드 생략
    - 첫 배경이거나 TransitionSeconds ≤ 0이면 **즉시 전환** (페이드 없음)
    - 그 외에는 `bIsTransitioning = true` → 페이드 전환 시작
  - `Update(float DeltaTick)` — 전환 시간 누적, `TransitionDuration` 경과 시 Next → Current 교체
  - `Draw(sf::RenderWindow&) const` — 비전환 시 CurrentSprite 단독 출력, 전환 중에는 `TransitionTime / TransitionDuration` 진행도로 현재/다음 스프라이트 알파 블렌딩하여 동시 출력
  - 비공개 헬퍼: `SetupSprite` (텍스처 + 스케일 + 색상 초기화), `ApplyFullScreenTransform` (전체화면 스케일), `SetSpriteAlpha` (알파 클램프), `CreateTransparentTexture` (1×1 투명 텍스처, static)
- `Src/Core/String.h` 의존: `FVrdxString`으로 에셋명 비교

### 3단계 — CVrdxCharacterManager 구현

- `Src/Novel/CharacterManager.h` + `Src/Novel/CharacterManager.cpp` — `CVrdxCharacterManager` 구현 완료
  - 열거형: `EVrdxCharacterPosition { Left, Center, Right }`
  - 구조체: `FVrdxCharacterSlot` — CharacterName, PoseName, Alpha, 페이드 상태(`StartAlpha`/`TargetAlpha`/`FadeTime`/`FadeDuration`), 공유 텍스처, Sprite
  - 생성자: Left/Center/Right 슬롯 3개를 미리 생성, 투명 텍스처로 초기화
  - `ShowCharacter(Name, Pose, Position)` — 지정 슬롯에 텍스처 로드/캐시 후 즉시 표시 (Alpha=1.0)
  - `HideCharacter(Name, FadeSeconds)` — 이름 일치 슬롯 페이드아웃
  - `ClearSlot(Position, FadeSeconds)` — 지정 슬롯 페이드아웃
  - `Update(DeltaTick)` — 페이드 진행률 선형 보간, 완료 시 슬롯 리셋
  - `Draw(Window)` — 보이는 슬롯만 Alpha 적용하여 렌더링
  - 텍스처 캐시: `std::map<FVrdxString, TVrdxSharedPtr<sf::Texture>>` — `"{Name}/{Pose}"` 키, 실패 시 투명 텍스처 반환
  - 슬롯 위치: Left=320, Center=640, Right=960, AnchorY=660, 높이 제한 640px, 중심점 기준 정렬
  - 파일 경로: `Assets/Characters/{Name}/{Pose}.png`

### 문서 갱신

- `Docs/3-BackgroundCharacter.md` — 설계서를 실제 구현과 정합하도록 갱신
  - 1차: 클래스명 `CBackground` → `CVrdxBackground`, 생성자 시그니처, 헬퍼 메서드, 중복 무시 조건, 전환 메커니즘 상세 보강
  - 2차: `CCharacterManager` → `CVrdxCharacterManager`, `EVrdxCharacterSlot` → `EVrdxCharacterPosition`, `FCharacterSlot` → `FVrdxCharacterSlot`, 페이드 시스템 상세, 텍스처 캐싱 명세, 슬롯 좌표값 반영
  - 파일 상태표 항목들 ✅ 완료로 변경
- `Docs/Voradorix.md` — phase 갱신 (`3 (Background + CharacterManager 완료)`), 3단계 세부 항목에 CharacterManager 추가
- `Docs/WORK_LOG.md` — 본 항목 (작업 기록 추가)

### 4단계 명세서 작성

- `Docs/4-ScriptEngine.md` — ScriptEngine 로드/파싱/실행 명세 작성
- `Docs/Voradorix.md` — 4단계 항목에 ScriptEngine 명세 링크 추가
- `Docs/GAME_DESIGN.md` — ScriptEngine 세부 명세 링크 추가

## 2026-07-02

### 4단계 — ScriptEngine 구현 완료

- `Src/Scene/SceneManager.h/cpp` — Scene 소유권 `unique_ptr` → `shared_ptr`로 전환
- `Src/Scene/NovelScene.h/cpp` — `enable_shared_from_this` 상속, `OnEnter()`에서 `ScriptEngine.SetNovelScene(shared_from_this())`
- `Src/Novel/ScriptEngine.h/cpp` — `weak_ptr<NovelScene>` 저장, `Update()` 내 while 루프로 연속 실행, `JumpToLabel()`로 분기
- `Src/Novel/ScriptLine.h/cpp` — 명령어별 파생 struct + 팩토리 테이블 + Construct/Dispatch 패턴, 8개 명령어 지원
- `Assets/Scripts/TestScript.txt` — @label/@jump 분기 포함 25라인 테스트 스크립트
- `Application.cpp` — `MakeVrdxShared<CVrdxNovelScene>()`로 시작

### 문서 갱신

- `Docs/Voradorix.md` — phase를 4로 갱신, 4단계 완료 항목 추가
- `Docs/GAME_DESIGN.md` — shared_ptr 소유권, ScriptEngine 명령어 리스트, 디렉토리 구조 갱신
- `Docs/1-SceneSystem.md` — SceneManager `TVrdxSharedPtr` 전환 반영, 소유권 정책 갱신
- `Docs/2-NovelScene.md` — ScriptEngine 연동 구조로 전면 재작성 (shared_from_this, CanAdvance, 갱신/렌더 순서, 메서드 상세)
- `Docs/4-ScriptEngine.md` — 상태를 "구현 완료"로 변경, 실제 인터페이스/실행 규칙/테스트 스크립트 반영

### 5단계 명세서 작성

- `Docs/5-ChoiceSystem.md` — 선택지 UI/분기 시스템 명세 작성
- `Docs/Voradorix.md` — 5단계 항목을 명세 작성 완료 상태로 갱신
- `Docs/GAME_DESIGN.md` — `@choice` 파라미터 규칙과 ChoiceWidget 흐름 반영
- `Docs/4-ScriptEngine.md` — `@choice` 후속 명세 링크 추가

## 2026-07-04

### UI Foundation 진행 중

- `Docs/6-UIFoundation.md` — 공통 위젯 계층을 진행 중 상태로 전환
- `Docs/Voradorix.md` — 현재 진행 단계로 UI Foundation 반영
- `Docs/GAME_DESIGN.md` — UI Foundation 섹션을 진행 중 상태로 갱신

### 5단계 — ChoiceSystem 구현 완료

- `Src/Ui/ChoiceWidget.h/cpp` — 선택지 UI, hover, keyboard, click, 즉시 분기 처리 구현
- `Src/Novel/ScriptLine.h/cpp` — `FVrdxChoiceScriptLine` 및 `@choice` 파싱/실행 연결
- `Src/Scene/NovelScene.h/cpp` — ChoiceWidget 소유 및 입력/렌더 연동, `CanAdvance()` 연동
- `Assets/Scripts/TestScript.txt` — 선택지 분기 검증용 샘플 스크립트 추가

### 문서 갱신

- `Docs/5-ChoiceSystem.md` — 구현 완료 상태로 갱신
- `Docs/Voradorix.md` — 5단계 완료 상태로 갱신
- `Docs/GAME_DESIGN.md` — 5단계/`@choice` 구현 완료 반영
- `Docs/4-ScriptEngine.md` — `@choice` 후속 항목을 구현 완료로 정리
- `Docs/7-SaveLoad.md` — 7단계 Save/Load 기능명세 작성
- `Docs/GAME_DESIGN.md` — 7단계 세부 문서 링크 추가
- `Docs/Voradorix.md` — 7단계 항목에 Save/Load 명세 링크 추가

---

## 비주얼 노벨 엔진 — 구현 계획 (9단계)

### 디렉토리 구조

```
Game/Game/Src/
├── Core/          # Application, Common, Vector, String
├── Scene/         # Scene 인터페이스, SceneManager, 각 씬
├── Novel/         # NovelScene, ScriptEngine, CharacterManager
├── Ui/            # WidgetBase, DialogueBox, ChoiceWidget, Button, TextBox, Menu
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
| **2단계** | NovelScene + WidgetBase + DialogueBox + FVrdxString | `NovelScene.h/cpp`, `WidgetBase.h`, `DialogueBox.h/cpp`, `String.h/cpp` |
| **3단계** | Background 전환 + CharacterManager (좌/중/우 슬롯, 페이드, 텍스처 캐싱) | `Background.h/cpp`, `CharacterManager.h/cpp` |
| **4단계** | ScriptEngine — 텍스트 스크립트 파서/실행기 | `ScriptEngine.h/cpp` |
| **5단계** | ChoiceWidget — 선택지 UI 및 분기 처리 | `ChoiceWidget.h/cpp` |
| **6단계** | UI Foundation — 공통 위젯 계층 | `WidgetBase.h/cpp`, `DialogueBox.h/cpp`, `ChoiceWidget.h/cpp` |
| **7단계** | SaveManager — 세이브/로드 | `SaveManager.h/cpp` |
| **8단계** | TitleScene, ConfigScene — 메뉴 구성 | `TitleScene.h/cpp`, `ConfigScene.h/cpp` |
| **9단계** | EffectManager — 페이드, 셰이크 등 연출 효과 | `EffectManager.h/cpp` |

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

### UI 컴포넌트 작업 메모

- **현재 작업 단계**: `UI Foundation` 진행 중
- **핵심 목표**: `WidgetBase` 확장 + `ImageWidget` / `TextLabel` / `Button` / `WidgetContainer` 공통 계층화
- **적용 후보**: `DialogueBox`, `ChoiceWidget`, 이후 `TitleScene` / `ConfigScene` / `SaveLoadScene`
- **목표**: 입력 처리, Hover/Leave/Click, Draw, 계층 배치 공통화를 통해 UI와 로직 분리
- **명세 문서**: `Docs/6-UIFoundation.md`
