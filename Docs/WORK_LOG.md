---
title: 작업 기록
project: Voradorix
tags:
  - voradorix
  - log
---
# 작업 기록

## 2026-07-10

### 에디터 Import/실행 환경 정리

- `Editor/src/main/ipcHandlers.ts` — Import를 단일 흐름으로 통합, 다중 선택 및 디렉토리 재귀 복사 지원
- `Editor/src/preload/index.ts` / `index.d.ts` — 새 Import IPC 바인딩 반영
- `Editor/src/renderer/src/components/AssetTree.tsx` — Import 버튼 단일화, 다중 소스 선택 모달, 디렉토리 포함 Import UI 반영
- `Editor/src/renderer/src/App.css` — Import 모달/컨텍스트 메뉴 스타일 추가
- `Game/Game.vcxproj` — 디버거 실행 시 작업 디렉토리를 솔루션 루트로 고정
- `RunEditor.bat` — 에디터를 바로 실행하는 배치 파일 추가
- `README.md` / `Docs/Editor/*` — 에디터 실행/Import 동작 설명 갱신

### 에셋/배포 정리

- `Assets/` — 로컬 에셋 작업 디렉토리로 전환, 소스관리 제외
- `Assets/` — 로컬 작업용 에셋 디렉토리로 전환, 소스관리 제외
- `Extern/SFML-3.1.0/` — 복사해둔 SFML 3.1.0 vendor 트리 반영

### 런타임 자원 로딩 정리

- `Src/Core/AssetManager.cpp` — 레지스트리 로딩 시 `alias` null을 빈 문자열로 처리하도록 수정
- `Src/Ui/TextLabel.cpp` — 폰트 직접 로드 제거, AssetManager 기반 `sf::Text` 보관으로 변경
- `Src/Novel/DialogueBox.cpp` — 대사창 텍스트도 AssetManager 폰트를 사용하도록 변경
- `Src/Novel/ChoiceWidget.cpp` — 선택지 텍스트도 AssetManager 폰트를 사용하도록 변경

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

### 2026-07-10 — AssetManager 텍스처/스크립트 전환

- `Src/Core/AssetManager.cpp` — `ScriptPathCache` 엔트리도 유효 에셋으로 등록하도록 수정
- `Src/Novel/Background.h/cpp` — 배경 로딩을 `AssetManager::GetTexture()` 경유로 전환, `Clear()` 추가
- `Src/Novel/CharacterManager.h/cpp` — 캐릭터 텍스처 로딩을 `AssetManager::GetTexture()` 경유로 전환, 내부 `TextureCache` 제거
- `Src/Novel/DialogueBox.h/cpp` — `Clear()` 추가, 리셋 시 표시 문자열/발화자 문자열 초기화
- `Src/Novel/ChoiceWidget.h/cpp` — `Reset()` 추가, 리셋 시 선택지/버튼/가시성 초기화
- `Src/Novel/NovelScene.h/cpp` — `Reset()` 추가, `ScriptEngine.Reset()`에서 배경/캐릭터/대사창/선택지 상태 동기 초기화
- `Src/Novel/ScriptEngine.cpp` — `Reset()` 시 NovelScene 상태까지 함께 초기화
- `Src/Main.cpp` — New Game/Continue/Load 전환 시 창 가시성 일원화
- `Docs/Engine/AssetManager.md`, `Docs/Engine/3-BackgroundCharacter.md`, `Docs/Management/BACKLOG.md` — 구현 상태와 로딩 경로 반영 완료

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
- `Src/Ui/BoxWidget.h/cpp` — 이미지/텍스처 출력 가능한 위젯 구현, Shape 기반 geometry 동기화
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

## 2026-07-06

### 7단계 — Save/Load 구현 완료

- `Scene/SceneManager.h/cpp` — `Save()` / `Load()` 멤버 함수 추가
  - `Save()`: const 메서드, dynamic_cast로 NovelScene 확인 후 JSON 파일 작성
  - `Load()`: 파일 읽기 → JSON 파싱 → NovelScene.Load() 호출
  - `std::filesystem::create_directories`로 `Saves/` 디렉토리 자동 생성
  - 사용 라이브러리: `<fstream>`, `<sstream>`, `<filesystem>`, `nlohmann/json`

- `Scene/NovelScene.h/cpp` — SaveData 구조체 및 직렬화 구현
  - `FVrdxNovelSceneSaveData` — ScriptPath, CurrentLine, BackgroundName, CharacterSlots
  - `Save()` — 상태 수집 (ScriptEngine, Background, CharacterManager)
  - `Load()` — 상태 복원 (ScriptEngine 재로드, Background/Character 재설정, JumpToLine)
  - `ToJson()` / `FromJson()` — nlohmann/json 기반 직렬화

- `Novel/CharacterManager.h/cpp` — Save/Load 지원
  - `GetSaveData()` — 슬롯 상태를 `TVrdxVector<FVrdxCharacterSlotSaveData>`로 수집
  - `Reset()` — 전체 슬롯 초기화

- `Extern/nlohmann/json.hpp` — nlohmann/json v3.12.0 헤더 설치
- `Game.vcxproj` — `$(SolutionDir)Extern;` include 경로 추가
- `Saves/` — 저장 파일 디렉토리 (실행 경로 기준)

### const correctness 개선

- `Novel/CharacterManager.h/cpp` — `FindSlotByPosition` / `FindSlotByCharacter` const 오버로드 분리
  - const 버전: `const FVrdxCharacterSlot*` 반환
  - non-const 버전: const_cast를 통한 위임 패턴

### 8단계 — Scene 시스템 → Widget Tree 구조 변경

**파일 이동:**
- `Ui/DialogueBox.h/cpp` → `Novel/`
- `Ui/ChoiceWidget.h/cpp` → `Novel/`
- `Core/Application.h/cpp` → `Ui/`
- `Scene/NovelScene.h/cpp` → `Novel/`

**삭제:**
- `Scene/Scene.h/cpp`, `Scene/SceneManager.h/cpp`, `Scene/TestScene.h/cpp` — 전체 삭제

**WidgetBase 상속 전환:**
- `CVrdxNovelScene` — `CVrdxScene` 대신 `CVrdxWidgetBase` 상속
- `CVrdxBackground` — `CVrdxWidgetBase` 상속 (이름 유지)
- `CVrdxCharacterManager` — `CVrdxWidgetBase` 상속 (이름 유지)
- `CVrdxApplication` — `CVrdxWidgetBase` 상속, SceneManager 제거, Save/Load 직접 처리 (Children 순회)

**WidgetBase 보강:**
- `BringToFront()` 추가 — 자신을 형제 중 최상단 Z-Order로 이동

**Application 구조 변경:**
- `Run()` 루프: `Update()` → `Draw()` (WidgetBase 트리 자동 전파)
- `Save()/Load()`: Children 순회 + `dynamic_pointer_cast<CVrdxNovelScene>`으로 탐색 후 직렬화

**Main.cpp:**
- `CreateWidget<CVrdxApplication>(nullptr, Panel)` 방식으로 전환

**프로젝트 파일:**
- `Game.vcxproj` — 모든 경로 Novel/Ui로 갱신
- `Game.vcxproj.filters` — 경로/필터 갱신, Scene 필터 및 잔여 참조 제거

**신규 문서:**
- `Docs/StructureReform.md` — 구조 변경 명세 및 진행 상태
- `Docs/PROJECT_STRUCTURE.md` — opencode/Git 구조 설명

### 8단계 후속 — 이벤트 시스템 개선 및 Application 리팩터

**이벤트 훅 `void` → `bool` 반환 변경:**
- `WidgetBase.h/cpp` — `OnMouseMove`, `OnMouseLeftButtonPressed/Released`, `OnMouseRightButtonPressed/Released`, `OnKeyboardPressed/Released` 모두 `bool` 반환으로 변경
- `HandleEvent()`가 각 훅의 반환값을 그대로 전파 (true=소비, false=통과)
- 마우스 이벤트에서 버튼 미매칭 시 false 반환 (이전에는 무조건 true)
- `ChoiceWidget`, `DialogueBox`, `Button` — 각자의 이벤트 훅 `bool` 반환으로 일괄 변경

**Application 재구성:**
- `OnPostCreate()` 제거 → `Initialize(VRDX_Initializer&&)` 콜백 패턴 도입
- `Save()/Load()` 메서드 제거 → NovelScene으로 이관
- `Draw()` override 제거 (WidgetBase 기본 Draw 사용, clear/display는 Run 내부)
- 편의 생성자 `CVrdxApplication(const sf::Vector2f& Size)` 추가
- 생성자에 try/catch (Window 생성 실패 시 bIsRunning = false)
- 중복 초기화 방지 `bInitialized` 플래그 추가
- 이전 `#if 0` 처리된 Save/Load/Keyboard 핸들링 정리

**NovelScene — Save/Load 직접 처리:**
- `OnKeyboardPressed` override — Ctrl+S/Ctrl+L 감지
- `Save(const FVrdxString& Filename)` — 파일 I/O 버전 (기존 `Save()` 호출 후 파일 쓰기)
- `Load(const FVrdxString& Filename)` — 파일 I/O 버전 (파일 읽기 후 기존 `Load(SaveData)` 호출)

**Main.cpp:**
- `CreateWidget<CVrdxApplication>` → `MakeVrdxShared<CVrdxApplication>(sf::Vector2f(1280,720))` + `Initialize(Initializer)` 패턴
- `NovelScene` 생성 람다를 `VRDX_Initializer` 콜백으로 전달

## 2026-07-10

### 8단계 — 메뉴 구성 (TitleWindow / SaveLoadWindow) 구현 완료

**신규 파일:**
- `Novel/TitleWindow.h/cpp` — `CVrdxTitleWindow` (`CVrdxBoxWidget` 상속)
  - `OnPostCreate`: TitleLabel, NewGameButton, LoadGameButton 생성
  - `OnNewGame()` / `OnContinueGame()` → `Broadcast()` 델리게이트 호출
  - `RequestNewGame`, `RequestContinueGame` 델리게이트
- `Novel/SaveLoadWindow.h/cpp` — `CVrdxSaveLoadWindow` (`CVrdxBoxWidget` 상속)
  - `OnPostCreate`: TitleLabel, BackToMain 버튼, 10개 슬롯 버튼 생성
  - `ShowSaveLoadWindow(bool bSave)` — 저장/불러오기 모드 전환, 슬롯 텍스트 변경
  - `RequestSaveGame`, `RequestLoadGame`, `RequestBackToMain` 델리게이트
  - `IsSaveMode()` 쿼리 메서드

**수정 파일:**
- `Novel/NovelScene.cpp` — `ResetScriptEngine()` 추가 (ScriptEngine 재로드, New Game 재시작 지원)
- `Novel/NovelScene.cpp` — Save/Load 경로 버그 수정: `"Saves" + Filename` → `"Saves/" + Filename`
  - Load 함수에서 하드코딩 `"Saves/Save0.dat"` → `"Saves/" + Filename.ToUtf8()` 동적 경로로 변경
- `Ui/TextLabel.cpp` — `OnResized()` 위치 기준을 생성자와 통일 (`{0,0}`)
- `Main.cpp` — TitleWindow/SaveLoadWindow 생성 및 델리게이트 연결
  - New Game → `NovelWindow->BringToFront()`
  - Continue → `SaveLoadWindow->ShowSaveLoadWindow(false)`
  - Load 슬롯 선택 → `std::filesystem::exists()` 체크 후 `NovelWindow->Load()` + `BringToFront()`
  - BackToMain → `TitleWindow->BringToFront()`

**프로젝트 설정:**
- `Game.vcxproj` / `.filters` — SaveLoadWindow.cpp/h, TitleWindow.cpp/h 등록

**문서:**
- `Docs/8-Menu.md` — 명세서를 실제 구현에 맞게 전면 갱신

## 2026-07-10 (Discussion)

### 프로젝트 방향 전환 — 게임 개발 → 엔진 + 에디터 개발

**배경:** 8단계(메뉴 구성)까지 구현 완료된 상태에서, 콘텐츠 제작이 아직 시작되지 않았으므로
에디터를 먼저 갖추어 제작 생산성을 확보한 후 콘텐츠 작업에 들어가는 것이 효율적이라고 판단.

**결정 사항:**
- 9단계(EffectManager, 연출 효과)는 **후순위 보류**
- 프로젝트 방향: 비주얼 노벨 게임 개발 → **게임 엔진 + 에디터 개발**로 전환
- 엔진은 C++/SFML 유지, 에디터는 **Electron**(웹 기술)으로 제작
- 에디터가 메인 프로세스, 엔진은 자식 프로세스로 실행 (Unreal Editor 방식)
- 데이터 연동은 파일 기반 (`AssetRegistry.json`)

**AssetManager 설계 (Phase 1):**
- 싱글톤, 독립 모듈 (WidgetBase 미상속)
- 내부 식별자: UUID v4
- Alias: 옵셔널 (Name 기반 등록 시 자동 생성)
- 레지스트리: `AssetRegistry.json` 단일 파일
- Import 시 원본 파일명 유지하여 프로젝트 내 `Assets/`로 복사
- 첫 구현 목표: `GetFont()` → 폰트 3곳(DialogueBox, TextLabel, ChoiceWidget) 중복 로딩 제거
- `InitializeInstance()`에서 Registry 로드 후 폰트/텍스처/스크립트 캐시를 일괄 구축

**Electron 에디터 설계 (Phase 2):**
- 위치: `Game/Editor/`
- 첫 기능: Asset Browser (파일 트리, 미리보기, Import)
- 게임 실행: `child_process.spawn()`, 경로는 `Editor/build/Voradorix.exe` 고정

**향후 확장 (Phase 3):**
- Script Editor (문법 하이라이팅, @label 네비게이션)
- Widget Designer (위젯 트리 JSON 편집)

**관련 문서:**
- `Docs/PROJECT_DIRECTION.md` — 방향 전환 상세 명세 (신규)
- `Docs/Voradorix.md` — phase 및 구현 현황 갱신
- `Docs/BACKLOG.md` — 백로그 갱신
- `README.md` — 프로젝트 설명 갱신

---

## 2026-07-10 (Later)

### 9단계 명세서 작성

- `Docs/9-EffectManager.md` — EffectManager 연출 효과 시스템 명세 작성
  - 페이드인/페이드아웃, 셰이크, 컬러 오버레이 효과 정의
  - 효과 큐(Queue) 시스템 설계
  - ScriptEngine `@fadein`/`@fadeout`/`@shake`/`@coloroverlay` 명령어 연동 명세
  - NovelScene과의 협력 관계 (Draw 시 View offset 적용)
  - 우선순위: FadeIn/Out → Shake → ColorOverlay → 큐 → ScriptLine 순으로 구현
- `Docs/Voradorix.md` — phase를 9로 갱신, 9단계 항목에 명세서 링크 추가

---

## 비주얼 노벨 엔진 — 구현 계획 (10단계)

---

## 2026-07-11 (Editor)

### Script Editor 확장

- `Editor/src/renderer/src/components/ScriptEditorPanel.tsx`
  - `.txt` 스크립트 전용 편집 UI 추가
  - `@label`, `@jump`, `@choice` 검증 및 점프 대상 존재 여부 검사
  - 블럭 단위 색상띠 표시
  - 도달 불가 블럭 및 잠재적 무한루프 경고 표시
  - `@` 입력 시 명령어 자동완성 드롭다운 및 스니펫 삽입
  - 커서 인근 표시, 스크롤 추적, 위/아래 방향 펼침 대응
  - 줄번호 0-based 표시, 빈 줄은 번호 건너뜀
- `Editor/src/main/ipcHandlers.ts`
  - 스크립트 저장용 텍스트 쓰기 IPC 추가
- `Editor/src/preload/index.ts`, `index.d.ts`
  - Renderer에서 스크립트 저장 API 노출
- `Editor/src/renderer/src/components/AssetTree.tsx`
  - Alias 없는 리소스 노란 상태 표시 추가


### 디렉토리 구조

```
Game/Game/Src/
├── Core/          # Common, Vector, String (유틸리티)
├── Novel/         # NovelScene, Background, CharacterManager, DialogueBox,
│                  # ChoiceWidget, ScriptEngine, ScriptLine, DialogueLine
├── Ui/            # Application, WidgetBase, BoxWidget, Button, TextLabel
└── Main.cpp

Assets/
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
| **7단계** | Save/Load — NovelScene 직렬화/복원 | `Application.h/cpp`, `NovelScene.h/cpp`, `CharacterManager.h/cpp` |
| **8단계** | 구조 변경 — Widget Tree 기반 아키텍처 전환 | `WidgetBase.h/cpp`, `Application.h/cpp`, `NovelScene.h/cpp` |
| **9단계** | 메뉴 구성 — TitleWindow, SaveLoadWindow | `TitleWindow.h/cpp`, `SaveLoadWindow.h/cpp` |
| **10단계** | EffectManager — 페이드, 셰이크 등 연출 효과 | 예정 |

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

- **현재 작업 단계**: `8단계 — 메뉴 구성` 완료
- **핵심 목표**: `WidgetBase`(위젯 트리 컨테이너 포함) 확장 + `BoxWidget` / `TextLabel` / `Button` 공통 계층화
- **정리 사항**: 위젯 트리 컨테이너 역할은 `WidgetBase` 기능에 포함하며, 별도 컨테이너 클래스는 두지 않음
- **정리 사항**: 완전 가림 상태는 `bCanBeDrawn`으로 판정해 불필요한 Draw call을 줄임
- **적용 완료**: `DialogueBox`, `ChoiceWidget`, `TitleWindow`, `SaveLoadWindow`
- **목표**: 입력 처리, Hover/Leave/Click, Draw, 계층 배치 공통화를 통해 UI와 로직 분리
- **명세 문서**: `Docs/6-UIFoundation.md`

### TextLabel 작업 메모

- `CVrdxTextLabel` 추가 완료
- `CVrdxBoxWidget` 기반으로 `sf::Text`를 함께 렌더링
- 폰트는 현재 `AssetManager`의 `malgun` 엔트리를 통해 공유됨

### Button 작업 메모

- `CVrdxButton` 핵심 구현 완료
- `Normal` / `Pressed` / `Text` 자식 위젯으로 상태를 분리
- `SetIgnoreEvent()`로 시각 전용 자식의 이벤트 소비를 차단
- `SetCapture()`로 누름 상태와 마우스 릴리스 판정을 처리
- 추후 폴리싱: Hover / Focus / Keyboard Activate / 텍스트 서식
