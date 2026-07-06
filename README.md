# Voradorix

SFML 3.1.0 기반 2D 비주얼 노벨 엔진

## 개요

C++17, Visual Studio 2022 x64 환경에서 SFML 3.1.0을 사용하여 구축하는
비주얼 노벨(미연시) 스타일 게임 엔진입니다.

씬 기반 아키텍처, 텍스트 출력, 스크립트/분기 확장 구조를 지원합니다.

현재 1~6단계 핵심 구현이 완료되었고, 7~9단계는 예정 상태입니다.

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
├── Game.sln                  # Visual Studio 솔루션
├── Game/
│   ├── Game.vcxproj
│   ├── Game.vcxproj.filters
│   ├── Extern/SFML-3.1.0/
│   └── Src/
│       ├── Main.cpp
│       ├── Core/             # Application, Common, Vector, String
│       ├── Scene/            # Scene, SceneManager, TestScene, NovelScene
│       ├── Novel/            # 노벨 재생 로직
│       └── Ui/               # WidgetBase, DialogueBox, ChoiceWidget, Button
├── Extern/                    # nlohmann/json (v3.12.0)
├── Docs/                     # 설계/작업 기록 문서
│   └── BACKLOG.md            # 보류/폴리싱 항목
├── graphify-out/             # 그래프 분석 산출물
├── Assets/                   # 폰트 및 리소스
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
- [ ] 8단계 — 메뉴 구성
- [ ] 9단계 — 연출 효과

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

- 1~5단계 빌드/테스트 확인 완료
- 6단계 핵심 구현 완료, 7단계(Save/Load) 구현 완료
- 8~9단계는 예정
- 작업 기록과 프로젝트 인덱스 문서는 `Docs/`를 참고

## 라이선스

MIT
