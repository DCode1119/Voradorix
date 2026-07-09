# Voradorix

SFML 3.1.0 기반 2D 비주얼 노벨 엔진

## 개요

C++17, Visual Studio 2022 x64 환경에서 SFML 3.1.0을 사용하여 구축하는
비주얼 노벨(미연시) 스타일 게임 엔진입니다.

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
│   │   ├── Core/             # Common, Vector, String (유틸리티)
│   │   ├── Novel/            # NovelScene, Background, CharacterManager,
│   │   │                       DialogueBox, ChoiceWidget, ScriptEngine
│   │   └── Ui/               # Application, WidgetBase, BoxWidget, Button,
│   │                           TextLabel (공통 UI)
│   ├── Assets/               # 폰트 및 리소스
│   └── Saves/                # 세이브 파일 저장 디렉토리
├── Extern/                    # SFML 3.1.0, nlohmann/json (v3.12.0)
├── Docs/                     # 설계/작업 기록 문서
│   ├── 1-SceneSystem.md ~ 8-Menu.md
│   ├── BACKLOG.md
│   └── PROJECT_STRUCTURE.md  # opencode/Git 구조 설명
├── graphify-out/             # 그래프 분석 산출물 (git 미관리)
├── .gitignore
└── README.md
```

## 구현 현황

- [x] **1단계 — Scene 시스템**: Scene 인터페이스, SceneManager, Application, 검증용 TestScene 완료
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
- [ ] 9단계 — 연출 효과

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

## 현재 상태

- 1~8단계 + 구조 변경 빌드/테스트 확인 완료
- 8단계(메뉴 구성) TitleWindow / SaveLoadWindow 구현 완료
- 9단계(연출 효과)는 예정
- 작업 기록과 설계 문서는 `Docs/`를 참고

## 라이선스

MIT
