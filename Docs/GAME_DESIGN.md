---
title: 게임 설계 문서
project: Voradorix
tags:
  - voradorix
  - design
  - spec
---
# 비주얼 노벨 게임 — 기획 및 설계 문서

> **프로젝트**: Voradorix  
> **엔진**: SFML 3.1.0 (C++17, Visual Studio 2022 x64)  
> **작성일**: 2026-06-30

---

## 1. 개요

SFML 3.1.0을 기반으로 하는 2D 비주얼 노벨(미연시) 스타일 게임.
텍스트 중심의 스토리텔링에 배경 전환, 캐릭터 스프라이트 표시, 선택지를 통한 분기,
BGM/효과음, 연출 효과를 지원하는 자체 엔진을 구축한다.

### 1.1 게임 플로우

```
타이틀 화면 → [새 게임 / 로드 / 설정 / 종료]
    ↓ 새 게임
시나리오 재생 (텍스트 출력, 배경/캐릭터 전환)
    ↓ 선택지 등장
선택 → 분기 → 계속 진행
    ↓
엔딩 → 타이틀 복귀
```

---

## 2. 시스템 아키텍처

### 2.1 씬 기반 구조 (Scene Graph)

```
Scene (추상 인터페이스)
├── TitleScene       — 타이틀 화면
├── NovelScene       — 본문 재생 (핵심)
├── ConfigScene      — 설정 (볼륨, 해상도 등)
├── SaveLoadScene    — 세이브/로드 화면
└── (확장 가능)
```

**Scene 인터페이스 메서드**:
| 메서드 | 설명 |
|--------|------|
| `OnEnter()` | 씬 진입 시 1회 호출 |
| `OnExit()` | 씬 이탈 시 1회 호출 |
| `HandleEvent(sf::Event)` | 입력 처리 |
| `Update(float dt)` | 프레임 갱신 |
| `Draw(sf::RenderWindow&)` | 렌더링 |

**SceneManager**: Scene 포인터를 스택으로 관리.
- `Push(Scene*)` — 현재 씬 위에 새 씬 푸시 (일시 정지)
- `Pop()` — 현재 씬 제거, 이전 씬 복귀
- `Switch(Scene*)` — 현재 씬을 완전히 교체

### 2.2 어셋 관리 (AssetManager)

- 텍스처, 폰트, 사운드버퍼, 음악 파일을 `std::unordered_map<std::string, T>`에 캐싱
- 중복 로딩 방지, 느리게 로딩(Lazy load)
- 키(파일명) 기반 참조 → NovelScene 등에서 문자열로 어셋 지정

### 2.3 스크립트 엔진 (ScriptEngine)

텍스트 파일 기반 시나리오를 파싱하여 명령 단위로 실행.

**지원 명령어 (초안)**:

```
@bg filename.jpg        — 배경 전환 (페이드 옵션)
@bgm filename.mp3       — BGM 재생
@stopbgm                — BGM 정지
@se filename.wav        — 효과음 재생
@show char_name pose pos — 캐릭터 표시 (pos: left/center/right)
@hide char_name          — 캐릭터 숨김
@fadeout duration        — 화면 페이드아웃
@fadein duration         — 화면 페이드인
@shake intensity duration — 화면 흔들림
@choice "text" → label   — 선택지 (분기)
@jump label              — 레이블로 이동
@if flag = value → label — 조건 분기
@set flag = value        — 플래그 설정
char_name "대사"         — 대사 출력 (캐릭터명 자동 표시)
"대사"                   — 내레이션 (캐릭터명 없음)
```

### 2.4 세이브/로드 (SaveManager)

- JSON 기반 (SFML + nlohmann/json 또는 직접 파싱)
- 저장 항목: 현재 씬 상태, 플래그 값, 히스토리, 표시중인 캐릭터/배경

---

## 3. 디렉토리 구조

```
Game/Game/
├── Src/
│   ├── Main.cpp              # 진입점
│   ├── Core/
│   │   ├── Application.h/cpp # SFML 창+루프 관리
│   │   ├── AssetManager.h/cpp
│   │   └── Settings.h/cpp
│   ├── Scene/
│   │   ├── Scene.h           # 인터페이스
│   │   ├── SceneManager.h/cpp
│   │   ├── TitleScene.h/cpp
│   │   ├── NovelScene.h/cpp
│   │   └── ConfigScene.h/cpp
│   ├── Novel/
│   │   ├── NovelScene.h/cpp
│   │   ├── ScriptEngine.h/cpp
│   │   ├── CharacterManager.h/cpp
│   │   ├── Background.h/cpp
│   │   ├── ChoiceManager.h/cpp
│   │   └── EffectManager.h/cpp
│   ├── Ui/
│   │   ├── BaseWidget.h
│   │   ├── DialogueBox.h/cpp
│   │   ├── Button.h/cpp
│   │   ├── TextBox.h/cpp
│   │   └── Menu.h/cpp
│   └── Save/
│       └── SaveManager.h/cpp
├── Assets/
│   ├── Scripts/         # .txt 시나리오
│   ├── Backgrounds/     # 배경 이미지 (.png)
│   ├── Characters/      # 캐릭터 스프라이트 (.png)
│   ├── Bgm/             # 배경 음악 (.ogg/.mp3)
│   ├── Se/              # 효과음 (.wav/.ogg)
│   ├── Fonts/           # 폰트 파일 (.ttf)
│   └── Ui/              # UI 프레임, 버튼 그래픽
└── Docs/
    └── GameDesign.md    # (이 파일)
```

---

## 4. 구현 로드맵

### 1단계 — Scene 시스템 도입
- `Scene.h` — 추상 인터페이스 정의
- `SceneManager.h/cpp` — 스택 기반 씬 관리
- `Application.h/cpp` — 메인 루프 리팩터 (SceneManager 구동)
- `main.cpp` — 최소화 (Application 생성/실행)
- **결과물**: 기존 원 그리기 코드가 Scene 위에서 동작

### 2단계 — NovelScene + DialogueBox (명세 완료, 코드 진행 중)
- `Core/String.h/cpp` — `FVrdxString` 유니코드 문자열 클래스 (UTF-32 기반)
  - UTF-8 입출력, SFML `sf::String` 변환, 코드 포인트 단위 `Left(N)`/`Substr`
  - DialogueBox 타이핑 애니메이션의 기반 타입
- `Ui/BaseWidget.h` — `CVrdxBaseWidget` (UI 위젯 공통 인터페이스)
- `Ui/DialogueBox.h/cpp` — 하단 대사창 (CVrdxBaseWidget 상속)
  - 타이핑 출력 + 클릭 시 전체 표시 / 다음 대사
  - 캐릭터명 표시 영역
  - Scene 시스템 의존성 없음 (순수 위젯)
- `Novel/NovelScene.h/cpp` — 기본 노벨 화면
  - `FDialogueLine` 목록 순회, DialogueBox 소유
  - 확인 입력 시 `ShowNextLine()` 호출
- 폰트: `Assets/Fonts/malgun.ttf` (Malgun Gothic)

### 3단계 — 배경 및 캐릭터
- `Background.h/cpp` — 배경 이미지 전환 (페이드)
- `CharacterManager.h/cpp` — 캐릭터 스프라이트 표시/위치/표정 전환
  - left/center/right 3슬롯
  - 표정 변경 시 페이드
  - 세부 명세: `Docs/4-BackgroundCharacter.md`

### 4단계 — ScriptEngine
- `ScriptEngine.h/cpp` — 스크립트 파일 로드, 파싱, 명령 실행
- NovelScene이 ScriptEngine에서 명령을 꺼내 실행하는 루프
- `@bg`, `@show`, `@hide`, `say` 등 기본 명령어 처리

### 5단계 — 선택지 시스템
- `ChoiceManager.h/cpp` — 선택지 버튼 표시, 선택 대기
- ScriptEngine의 `@choice` 명령과 연동하여 분기

### 6단계 — 세이브/로드
- `SaveManager.h/cpp` — JSON 저장/불러오기
- SaveLoadScene에서 UI로 표시

### 7단계 — 메뉴 구성
- `TitleScene.h/cpp` — 타이틀 화면 (새 게임/로드/설정/종료)
- `ConfigScene.h/cpp` — 볼륨 조절, 전역 설정
- `UI/Button.h/cpp`, `UI/Menu.h/cpp` — 재사용 UI 컴포넌트

### 8단계 — 연출 효과
- `EffectManager.h/cpp` — 페이드인/아웃, 화면 셰이크, 컬러 오버레이
- 타이머 기반 연출 큐

---

## 5. 기술적 고려사항

### 네이밍 규칙
- 자세한 규칙은 `Docs/NAMING.md` 참조
- 요약: Unreal Engine 스타일 접두어 (`C`, `T`, `F`, `E`, `I`, `b`) + `Vrdx` 프로젝트 접두어 + PascalCase
- **`m_` 접두사 사용하지 않음** — 멤버 변수도 PascalCase 유지

### SFML 3.x 사용 시 주의점
- `sf::RenderWindow::draw()`는 `const` 참조를 받음
- `sf::Text`는 `sf::Font`를 참조로 유지해야 함 (복사 주의)
- `sf::SoundBuffer`는 공유 가능, `sf::Sound`는 개별 인스턴스
- `sf::Music`는 스트리밍 재생 (한 번에 하나)
- `sf::Texture`는 `sf::Sprite`보다 수명이 길어야 함

### 폰트
- 한글 렌더링을 위해 한글 TTF 필요
- 현재: `Assets/Fonts/malgun.ttf` (Malgun Gothic, Windows 기본)
- 추후 변경 가능: Noto Sans CJK KR, Pretendard

### 프레임 속도
- `sf::Clock` 기반 fixed timestep 또는 delta time
- 텍스트 타이핑 속도: 초당 40~60자 기준

---

## 6. 에셋 준비 계획

| 종류 | 포맷 | 비고 |
|------|------|------|
| 배경 | 1280×720 PNG | JPG도 가능 |
| 캐릭터 | PNG (투명) | 표정별 파일 분리 |
| UI | PNG | 9-slice 가능 고려 |
| BGM | OGG Vorbis | 루프 재생 고려 |
| SE | WAV/OGG | 짧은 효과음 |
| 폰트 | TTF | 한글 지원 |
| 스크립트 | UTF-8 TXT | BOM 없음, `@` 명령어 체계 |
