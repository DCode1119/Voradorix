# 프로젝트 구조

## 개요

```
D:\Projects\SFML_Project\          ← opencode 프로젝트 루트
├─ .opencode/                       ← opencode 설정 (git 미관리)
├─ opencode.json                    ← opencode 에이전트 설정
├─ AGENTS.md                        ← 에이전트 동작 수칙
└─ Game/                            ← Git 저장소 루트
    ├─ .git/
    ├─ README.md
    ├─ Game.sln
    ├─ Game/
    │   ├─ Game.vcxproj
    │   ├─ Game.vcxproj.filters
    │   ├─ Src/
    │   │   ├─ Core/              ← 유틸리티 (Common.h, String, Vector, AssetManager)
    │   │   ├─ Novel/             ← 비주얼 노벨 전용 컴포넌트 (NovelScene, Background,
    │   │   │                       CharacterManager, DialogueBox, ChoiceWidget, ScriptEngine 등)
    │   │   ├─ Ui/                ← 공통 UI 컴포넌트 (WidgetBase, Application, BoxWidget, Button, TextLabel)
    │   │   └─ Main.cpp
    │   ├─ Editor/                       ← Electron 에디터 프로젝트
    │   ├─ Saves/
    │   └─ Docs/
    ├─ Extern/
    ├─ Assets/                           ← 로컬 작업용 에셋 디렉토리 (소스관리 제외)
    └─ .gitignore
```

## 설계 의도

- **opencode 프로젝트 루트**와 **Git 저장소 루트**를 분리하여, 에이전트 설정(`.opencode/`, `opencode.json`, `AGENTS.md` 등)이
  Git에 노출되지 않도록 보호.
- `Game/` 디렉토리만 독립적인 Git 저장소로 관리.
- README.md, .gitignore 등 저장소 메타파일은 `Game/` 내에 위치.
- 문서 파일(`Docs/`)도 `Game/` 내에 위치하여 Git 추적됨.
