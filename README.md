# Voradorix

SFML 3.1.0 기반 2D 비주얼 노벨 엔진 + Electron 에디터

## 개요

C++17, Visual Studio 2022 x64 환경에서 SFML 3.1.0을 사용하여 구축하는
비주얼 노벨(미연시) 스타일 게임 엔진이며, Electron 기반 에디터를 함께 개발 중입니다.

> **2026-07-10: 프로젝트 방향 전환** — 게임 개발 → **게임 엔진 + 에디터 개발**
> 자세한 내용은 [[Docs/Design/PROJECT_DIRECTION.md]] 참조.

Widget Tree 기반 아키텍처, 텍스트 출력, 스크립트/분기/세이브로드 구조를 지원합니다.

## 기술 스택

| 항목 | 사양 |
|------|------|
| 언어 | C++17 |
| IDE | Visual Studio 2022 (v143) |
| 플랫폼 | x64 |
| 그래픽 | SFML 3.1.0 (동적 링크) |
| 서브시스템 | Windows |

## 프로젝트 구조

```
Game/                          ← Git 저장소 루트
├── Game.sln                  # Visual Studio 솔루션
├── Game/
│   ├── Game.vcxproj
│   ├── Game.vcxproj.filters
│   ├── Src/
│   │   ├── Main.cpp
│   │   ├── Core/             # Common, Vector, String, AssetManager (유틸리티)
│   │   ├── Novel/            # NovelScene, Background, CharacterManager,
│   │   │                       DialogueBox, ChoiceWidget, ScriptEngine
│   │   └── Ui/               # Application, WidgetBase, BoxWidget, Button,
│   │                           TextLabel (공통 UI)
│   └── Saves/                # 세이브 파일 저장 디렉토리
├── Editor/                   # Electron 에디터 (Phase 2 예정)
├── Extern/                    # SFML 3.1.0, nlohmann/json (v3.12.0)
├── Docs/                     # 설계/작업 기록 문서
│   ├── Engine/               # 엔진 기능명세 (1~9단계, String, StructureReform)
│   ├── Editor/               # 에디터 기능명세 (Phase 1~4, ARCHITECTURE)
│   ├── Design/               # 설계문서 (DIRECTION, STRUCTURE, NAMING, GAME_DESIGN)
│   └── Management/           # BACKLOG, 작업 기록
├── graphify-out/             # 그래프 분석 산출물 (git 미관리)
├── .gitignore
└── README.md
```

## 구현 현황

### 엔진 런타임 (기존, 안정화)

- [x] **1단계 — Scene 시스템**: Scene 인터페이스, SceneManager, Application, 검증용 TestScene 완료 (역사적, Widget Tree로 대체)
- [x] **2단계 — NovelScene + DialogueBox + String 기반**: 대사창/문자열 기반과 노벨 본문 흐름 완료
- [x] **3단계 — 배경 및 캐릭터**: CVrdxBackground (페이드 전환) + CVrdxCharacterManager (슬롯/페이드/텍스처 캐싱) 구현 완료
- [x] **4단계 — ScriptEngine**: 스크립트 로드/파싱/실행, @label/@jump 분기 완료
- [x] **5단계 — 선택지 시스템**: 선택지 UI/입력/분기 구현 완료
- [x] 6단계 — UI Foundation: 핵심 구현 완료, 세부 폴리싱 예정
- [x] **7단계 — 세이브/로드**: NovelScene 상태 직렬화/복원 (JSON, nlohmann)
- [x] **구조 변경**: Scene/SceneManager 폐기, Widget Tree 기반 아키텍처 전환 완료
  - BringToFront(), 모든 컴포넌트 WidgetBase 상속, 파일 구조 정리
- [x] **8단계 — 메뉴 구성**: TitleWindow (New Game / Continue) + SaveLoadWindow (10슬롯, Save/Load 모드) 구현 완료
  - NovelScene::ResetScriptEngine() 추가 (New Game 재시작)
  - Save/Load 경로 버그 수정
- [ ] **9단계 — EffectManager** (연출 효과, **보류** — 프로젝트 방향 전환으로 후순위)

### 신규 — 엔진 + 에디터 개발 방향 (2026-07-10 전환)

- [ ] **Phase 1 — AssetManager** (C++, Core/AssetManager.h/cpp)
  - 싱글톤 독립 모듈, UUID v4 기반 에셋 식별
  - `AssetRegistry.json` 중앙 레지스트리
  - 1차 목표: `LoadFont()` 구현, 폰트 중복 로딩 제거
  - 2차 목표: `LoadTexture()`, `ImportAsset()` 구현
- [ ] **Phase 2 — Electron Editor** (Game/Editor/)
  - Asset Browser (파일 트리, 미리보기, Import)
  - Play/Stop 버튼 (Voradorix.exe spawn)
  - React + Vite

## 스크립트 문법

스토리 스크립트는 `Assets/Scripts/` 디렉토리에 `.txt` 파일로 작성하며,
`@` 명령어 기반으로 동작합니다. 명령어 인자는 큰따옴표(`"`)로 감쌉니다.

| 명령어 | 설명 | 예시 |
|--------|------|------|
| `@bg` | 배경 이미지 전환 | `@bg "WhiteRoom"` |
| `@show` | 캐릭터 표시 (위치: Left/Center/Right) | `@show "Laura" "Center" "Normal"` |
| `@hide` | 캐릭터 숨김 | `@hide "Laura"` |
| `@pose` | 캐릭터 표정 변경 | `@pose "Laura" "Angry"` |
| `@wait` | 지정 시간(초) 동안 진행 정지 | `@wait "0.3"` |
| `@label` | 분기점 레이블 정의 | `@label "Main"` |
| `@jump` | 지정 레이블로 즉시 이동 | `@jump "Main"` |
| `@dialogue` | 대사 출력 (화자 + 내용) | `@dialogue "Laura" "안녕하세요."` |
| `@choice` | 선택지 표시 (옵션/레이블 쌍) | `@choice "Exit" "Finish" "Repeat" "Main"` |

### 예시

```
@label "Main"
@bg "WhiteRoom"
@show "Laura" "Center" "Normal"
@dialogue "Laura" "안녕하세요?"
@hide "Laura"
@choice "계속" "Next" "종료" "Finish"

@label "Finish"
@dialogue "Laura" "수고했어요."
```

## 네이밍 규칙

Unreal Engine 스타일 접두어 + `Vrdx` 프로젝트 접두어 + PascalCase를 사용합니다.

| 접두어 | 대상 | 예시 |
|--------|------|------|
| `C` | 클래스 | `CVrdxScene` |
| `T` | 템플릿 | `TVrdxVector<T>` |
| `E` | 열거형 | `EVrdxSceneResult` |
| `I` | 인터페이스 | `IVrdxSerializable` |
| `F` | 구조체 | `FVrdxSaveSlot` |
| `b` | bool 변수 | `bIsRunning` |

## 빌드 방법

1. Visual Studio 2022로 `Game/Game.sln` 열기
2. x64 Debug (또는 Release) 선택
3. 빌드 → PostBuild에서 SFML DLL 자동 복사
4. 실행

에디터는 `Game/RunEditor.bat`로 바로 실행할 수 있으며, 디버거 실행 시 작업 디렉토리는 솔루션 루트로 고정되어 있습니다.

## 현재 상태

- 1~8단계 + 구조 변경 빌드/테스트 확인 완료
- 8단계(메뉴 구성) TitleWindow / SaveLoadWindow 구현 완료
- **2026-07-10: 프로젝트 방향 전환** — 게임 개발 → 게임 엔진 + 에디터 개발
  - 9단계(연출 효과)는 보류
  - Phase 1: AssetManager (C++) 우선 구현
  - Phase 2: Electron 에디터 (Asset Browser) 예정
- 작업 기록과 설계 문서는 `Docs/`를 참고
- 방향 전환 상세: `Docs/Design/PROJECT_DIRECTION.md`

## 라이선스

MIT
