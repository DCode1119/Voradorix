# Voradorix

SFML 3.1.0 기반 2D 비주얼 노벨 엔진

## 개요

C++17, Visual Studio 2022 x64 환경에서 SFML 3.1.0을 사용하여 구축하는
비주얼 노벨(미연시) 스타일 게임 엔진입니다.

씬 기반 아키텍처, 스크립트 엔진, 텍스트 중심 스토리텔링을 지원합니다.

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
Game/
├── Game/                     # 솔루션 루트
│   ├── Game.sln              # Visual Studio 솔루션
│   ├── Game/                 # 프로젝트 디렉토리
│   │   ├── Game.vcxproj
│   │   ├── Extern/SFML-3.1.0/
│   │   └── Src/
│   │       ├── Main.cpp
│   │       ├── Core/         # Application, Common, Vector
│   │       ├── Scene/        # Scene, SceneManager, TestScene
│   │       ├── Novel/        # (예정) ScriptEngine, DialogueBox
│   │       ├── Ui/           # (예정) UI 컴포넌트
│   │       └── Save/         # (예정) SaveManager
│   └── README.md
├── Docs/                     # 설계 문서 및 작업 기록 (git 제외, 별도 관리)
│   ├── NAMING.md             # 네이밍 규칙
│   ├── 1-SceneSystem.md      # 1단계 명세
│   ├── GAME_DESIGN.md        # 전체 설계
│   └── WORK_LOG.md           # 작업 기록
└── opencode.json             # MCP 에이전트 설정
```

## 구현 현황

- [x] **1단계 — Scene 시스템**: Scene 인터페이스, SceneManager, Application, 검증용 TestScene 완료
- [ ] 2단계 — NovelScene + DialogueBox
- [ ] 3단계 — 배경 및 캐릭터
- [ ] 4단계 — ScriptEngine
- [ ] 5단계 — 선택지 시스템
- [ ] 6단계 — 세이브/로드
- [ ] 7단계 — 메뉴 구성
- [ ] 8단계 — 연출 효과

## 네이밍 규칙

Unreal Engine 스타일 접두어 + `Vrdx` 프로젝트 접두어 + PascalCase를 사용합니다.
자세한 내용은 프로젝트 로컬 `Docs/NAMING.md` 참조 (git 제외, 별도 관리).

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

## 라이선스

MIT
