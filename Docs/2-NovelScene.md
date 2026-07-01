---
title: "2단계: NovelScene + DialogueBox"
project: Voradorix
phase: 2
status: 완료
tags:
  - voradorix
  - novel-scene
  - dialogue-box
  - spec
  - cpp
---
# Voradorix — 2단계: NovelScene + DialogueBox 구현 명세

> **대상**: CVrdxNovelScene (본문 재생 씬) + CVrdxBaseWidget (UI 위젯 베이스) + CDialogueBox (하단 대사창 위젯)  
> **프로젝트**: Voradorix  
> **관련 파일**: `Src/Novel/NovelScene.h/cpp`, `Src/Ui/BaseWidget.h`, `Src/Ui/DialogueBox.h/cpp`, `Src/Novel/Background.h/cpp`, `Src/Novel/CharacterManager.h/cpp`  
> **의존성**: SFML 3.1.0 (`sf::Text`, `sf::Font`, `sf::RectangleShape`, `sf::Clock`), `Core/String.h`  
> **완료 조건**: NovelScene이 대사 목록을 순회하며 DialogueBox에 한 줄씩 표시, 배경 전환 및 캐릭터 표시 연동

---

## 1. 파일 구성

| # | 파일 | 설명 | 상태 |
|---|------|------|------|
| 1 | `Src/Ui/BaseWidget.h` | CVrdxBaseWidget 선언 (위젯 베이스 클래스) | ✅ 작성됨 |
| 2 | `Src/Ui/DialogueBox.h` | CDialogueBox 선언 | ✅ 작성됨 |
| 3 | `Src/Ui/DialogueBox.cpp` | CDialogueBox 구현 | ✅ 작성됨 |
| 4 | `Src/Novel/NovelScene.h` | CVrdxNovelScene 선언 | ✅ 작성됨 |
| 5 | `Src/Novel/NovelScene.cpp` | CVrdxNovelScene 구현 | ✅ 작성됨 |
| 6 | `Game.vcxproj` | ClCompile/ClInclude 항목 등록 | ✅ 등록 완료 |
| 7 | `Game.vcxproj.filters` | Ui/, Novel/ 필터 구성 | ✅ 구성됨 |

---

## 2. 네이밍

| 구분 | 규칙 | 예시 |
|------|------|------|
| 클래스 | `C` + `Vrdx` 접두어 | `CVrdxNovelScene`, `CDialogueBox`, `CVrdxBaseWidget` |
| 파일명 | PascalCase, 디렉토리명과 일치 | `NovelScene.h`, `DialogueBox.cpp`, `BaseWidget.h` |
| 구조체 | `F` + `Vrdx` | `FDialogueLine` |
| 폴더 | `Novel/`, `Ui/` | (Src 아래) |

---

## 3. FDialogueLine — 대사 데이터 구조체

**위치**: `NovelScene.h` 내부 또는 별도 헤더

```cpp
struct FVrdxDialogueLine
{
    FVrdxString Speaker;    // 발화자 이름 (빈 문자열이면 나레이션)
    FVrdxString Text;       // 대사 내용
};
```

- `FVrdxString` = UTF-32 기반 유니코드 문자열 (`Core/String.h` 참조)
- `FString` alias로도 접근 가능 (`using FString = FVrdxString`)
- Speaker가 비어있으면 나레이션으로 간주 (캐릭터명 영역 미표시)

---

## 4. CVrdxBaseWidget — UI 위젯 베이스 클래스

**파일**: `Src/Ui/BaseWidget.h` (header-only, .cpp 불필요)

### 4.1 역할

모든 UI 위젯이 상속받는 추상 베이스 클래스.
Scene과 동일한 생명주기 메서드(`HandleEvent`, `Update`, `Draw`)를 정의하여
일관된 인터페이스를 제공한다.

### 4.2 클래스 설계

```cpp
class CVrdxBaseWidget
{
public:
    virtual ~CVrdxBaseWidget() VRDX_DEFAULT;

    virtual void HandleEvent(const sf::Event& Event) VRDX_PURE_VIRTUAL;
    virtual void Update(const float DeltaTick) VRDX_PURE_VIRTUAL;
    virtual void Draw(sf::RenderWindow& Window) const VRDX_PURE_VIRTUAL;
};
```

### 4.3 메서드

| 메서드 | 설명 |
|--------|------|
| `HandleEvent(const sf::Event&)` | 입력 이벤트 처리 (키보드/마우스) |
| `Update(const float DeltaTick)` | 매 프레임 상태 갱신 |
| `Draw(sf::RenderWindow&)` | 렌더링 |

---

## 5. CDialogueBox — 대사창 위젯

**파일**: `Src/Ui/DialogueBox.h` / `Src/Ui/DialogueBox.cpp`

### 5.1 역할

화면 하단에 위치한 대사 출력 UI. `CVrdxBaseWidget`을 상속받아
Scene으로부터 독립적인 위젯으로 동작하며, 자체적으로 입력 처리,
타이핑 애니메이션, 렌더링을 수행한다.

### 4.1 역할

화면 하단에 위치한 대사 출력 UI. NovelScene으로부터 독립적인 위젯으로,
자체적으로 입력 처리, 타이핑 애니메이션, 렌더링을 수행한다.

### 4.2 위치 및 크기 (1280×720 기준)

| 요소 | 위치/크기 |
|------|-----------|
| 박스 | 화면 하단, (40, 500) ~ (1240, 700) — 1200×200 |
| 이름 영역 | 박스 상단 좌측, (60, 505) ~ (260, 545) — 200×40 |
| 대사 영역 | (60, 550) ~ (1220, 680) — 1160×130 |

이후 실제 폰트 크기와 레이아웃에 따라 조정 가능.

### 5.3 클래스 설계

```cpp
class CDialogueBox : public CVrdxBaseWidget
{
public:
    CDialogueBox();
    virtual ~CDialogueBox() VRDX_DEFAULT;

    // CVrdxBaseWidget 인터페이스
    virtual void HandleEvent(const sf::Event& Event) VRDX_OVERRIDE;
    virtual void Update(const float DeltaTick) VRDX_OVERRIDE;
    virtual void Draw(sf::RenderWindow& Window) const VRDX_OVERRIDE;

    // 대사 설정
    void SetSpeaker(const FVrdxString& Name);
    void SetLine(const FVrdxString& Text);

    // 상태
    bool IsTyping() const;      // 타이핑 진행 중?
    bool IsFinished() const;    // 현재 대사 출력 완료?

private:
    void StartTyping();
    void FinishTyping();        // 즉시 전체 표시

    // 위젯
    sf::RectangleShape Panel;       // 대사창 배경
    sf::Text           SpeakerText; // 발화자명
    sf::Text           LineText;    // 대사 내용

    // 타이핑 상태
    FVrdxString        CurrentText;  // 원본 문자열
    size_t             VisibleCount; // 현재까지 표시된 글자 수
    float              TypeTimer;    // 타이핑 누적 시간
    float              TypeInterval; // 글자 간 시간 간격 (초)

    // 데이터
    FVrdxString        SpeakerName;  // 현재 발화자
};
```

### 5.4 메서드 상세

#### 생성자

1. 폰트 로드: `Assets/Fonts/` 경로에서 기본 폰트 로드
   - 추후 AssetManager로 이관
   - 로드 실패 시 기본 `sf::Font::getDefaultFont()` 사용 (fallback)
2. Panel 설정:
   - `sf::Color(0, 0, 0, 180)` — 반투명 검정
3. SpeakerText 설정:
   - 글자 크기: 22px
   - 색상: 흰색
4. LineText 설정:
   - 글자 크기: 20px
   - 색상: 흰색
   - 줄바꿈: `sf::Text::setString()`에 `\n` 포함

#### `SetSpeaker(const FVrdxString& Name)`

- `SpeakerName = Name`
- `SpeakerText.setString(Name)` 업데이트

#### `SetLine(const FVrdxString& Text)`

- `CurrentText = Text`
- `VisibleCount = 0`
- `TypeTimer = 0.0f`
- 타이핑 애니메이션 시작

#### `Update(const float DeltaTick)`

- 타이핑 중이면:
  - `TypeTimer += DeltaTick`
  - `TypeTimer >= TypeInterval`마다 `VisibleCount++`
  - 전체 글자가 표시되면 타이핑 완료 상태로 전환
- 기본 `TypeInterval`: 0.025초 (초당 40자)

#### `HandleEvent(const sf::Event& Event)`

- 클릭 또는 확인 키(Enter/Space):
  - 타이핑 중 → `FinishTyping()` (전체 표시)
  - 이미 완료 → (NovelScene이 인지하고 다음 줄로 넘김)

#### `FinishTyping()`

- `VisibleCount = CurrentText.length()` (전체 표시)
- 타이핑 상태 → 완료 상태

#### `SetLineVisibleCount()`

- `LineText.setString(CurrentText.substr(0, VisibleCount))`로 표시 문자열 갱신
- Update 내에서 VisibleCount 변경 시 호출

#### `Draw(sf::RenderWindow& Window)`

1. `Window.draw(Panel)`
2. `Window.draw(SpeakerText)`
3. `Window.draw(LineText)`

---

## 6. CVrdxNovelScene — 노벨 본문 재생 씬

**파일**: `Src/Novel/NovelScene.h` / `Src/Novel/NovelScene.cpp`

### 6.1 역할

`CDialogueBox`를 소유하고 대사 목록을 순회하며 표시하는 Scene.
3단계부터 `CVrdxBackground`와 `CVrdxCharacterManager`를 소유하여
배경 전환 및 캐릭터 표시를 함께 처리한다.

렌더 순서: **배경 → 캐릭터 → 대사창** (3계층)

### 6.2 클래스 설계

```cpp
class CVrdxNovelScene : public CVrdxScene
{
public:
    CVrdxNovelScene();
    ~CVrdxNovelScene() VRDX_DEFAULT;

    void OnEnter() VRDX_OVERRIDE;
    void OnExit() VRDX_OVERRIDE;
    void HandleEvent(const sf::Event& Event) VRDX_OVERRIDE;
    void Update(const float DeltaTick) VRDX_OVERRIDE;
    void Draw(sf::RenderWindow& Window) VRDX_OVERRIDE;

    void SetBackground(const FVrdxString& BackgroundName);
    void ShowCharacter(const FVrdxString& Character, EVrdxCharacterPosition Position);
    void HideCharacter(const FVrdxString& Character);
    void SetCharacterPose(const FVrdxString& Character, const FVrdxString& Pose);

private:
    void ShowNextLine();
    void EndScenario();
    void SwitchBackground(int32_t BackgroundIndex);

    TVrdxVector<FVrdxDialogueLine> Script;
    int32_t BackgroundIndex;
    int32_t CurrentIndex;
    CVrdxBackground Background;
    CVrdxCharacterManager CharacterManager;
    CVrdxDialogueBox DialogeBox;
};
```

### 6.3 메서드 상세

#### 생성자

1. 테스트용 대사 데이터를 `Script`에 하드코딩 (향후 ScriptEngine에서 로드)
   - 예: `{"???", "Welcome to the Chamber of Creation."}`
2. `CurrentIndex = 0`, `BackgroundIndex = -1` 초기화
3. `SwitchBackground(0)` 호출 — 첫 배경(WhiteRoom) 로드
4. `ShowCharacter("Laura", EVrdxCharacterPosition::Center)` 호출 — 테스트 캐릭터 표시

#### `OnEnter()`

- `BackgroundIndex = 0` 재설정
- `ShowNextLine()` 호출 (첫 번째 대사 표시)

#### `HandleEvent(const sf::Event& Event)`

- 클릭/확인 입력 발생 시:
  - 타이핑 중(`DialogueBox.IsTyping()`) → `FinishTyping()` (전체 표시)
  - 타이핑 완료 → 배경 전환 토글(`SwitchBackground`) + 대사 첫 줄로 리셋
    (현재는 배경 전환 테스트를 위해 대사 진행 대신 배경 토글)

#### `Update(const float DeltaTick)`

1. `Background.Update(DeltaTick)` — 배경 전환 타이머
2. `CharacterManager.Update(DeltaTick)` — 캐릭터 페이드
3. `DialogueBox.Update(DeltaTick)` — 대사 타이핑

#### `Draw(sf::RenderWindow& Window)`

1. `Window.clear(sf::Color::Black)` — 배경 초기화
2. `Background.Draw(Window)` — 배경 레이어
3. `CharacterManager.Draw(Window)` — 캐릭터 레이어
4. `DialogueBox.Draw(Window)` — UI 최상단

#### `ShowNextLine()`

1. `CurrentIndex < Script.Num()`:
   - `DialogueBox.SetSpeaker(Script[CurrentIndex].Speaker)`
   - `DialogueBox.SetLine(Script[CurrentIndex].Text)`
   - `CurrentIndex++`
2. 모든 대사 소진 시:
   - `EndScenario()` 호출

#### `EndScenario()`

- `RequestExit()` 호출
- SceneManager가 Pop 처리 → 이전 씬(또는 종료)

#### `SwitchBackground(int32_t InBackgroundIndex)`

- 현재 `BackgroundIndex`와 동일하면 무시 (중복 방지)
- 인덱스에 따라 `SetBackground()` 호출:
  - 0 → `"WhiteRoom"`
  - 1 → `"WhiteRoom_SunSet"`
- `BackgroundIndex` 갱신
- (테스트용 고정 매핑, 추후 ScriptEngine에서 동적 지정)

#### NovelScene의 퍼블릭 래퍼

| 메서드 | 내부 호출 |
|--------|-----------|
| `SetBackground(Name)` | `Background.SetBackground(Name)` |
| `ShowCharacter(Char, Pos)` | `CharacterManager.ShowCharacter(Char, Pos)` |
| `HideCharacter(Char)` | `CharacterManager.HideCharacter(Char)` |
| `SetCharacterPose(Char, Pose)` | `CharacterManager.SetCharacterPose(Char, Pose)` |

---

## 7. 폰트

| 항목 | 내용 |
|------|------|
| 권장 폰트 | Pretendard Variable, Noto Sans CJK KR, 나눔고딕 |
| 경로 | `Assets/Fonts/` |
| 폴백 | `sf::Font::getDefaultFont()` (한글 깨짐 가능) |
| 1차 목표 | 시스템 폰트 또는 프로젝트 내 TTF 포함 |

1단계에서는 프로젝트 내부에 한글 TTF를 포함하고,
CDialogueBox 생성자에서 이를 로드한다.
추후 AssetManager로 이관.

---

## 8. 의존성 그래프

```
Background.h
├── Core/Common.h
├── Core/String.h
├── SFML/Graphics/RenderWindow.hpp
├── SFML/Graphics/Image.hpp
├── SFML/Graphics/Texture.hpp
└── SFML/Graphics/Sprite.hpp

CharacterManager.h
├── Core/Common.h
├── Core/String.h
├── Core/Vector.h
├── SFML/Graphics/Image.hpp
├── SFML/Graphics/RenderWindow.hpp
├── SFML/Graphics/Sprite.hpp
├── SFML/Graphics/Texture.hpp
└── <map>

NovelScene.h
├── Scene/Scene.h
│     └── Core/Common.h
├── Ui/DialogueBox.h
│     └── Ui/BaseWidget.h
├── Novel/Background.h
└── Novel/CharacterManager.h

NovelScene.cpp
├── NovelScene.h
└── SFML/Window/Event.hpp
```

- 순환 의존성 없음
- DialogueBox는 Scene 시스템에 대해 전혀 모름 (완전히 독립적인 위젯)

---

## 9. DialogueBox 독립성 규칙

`CDialogueBox`는 `CVrdxScene`을 상속하지 않으며,
Scene 계층에 대한 어떤 참조도 가지지 않는다.

`CDialogueBox`는 `CVrdxBaseWidget`을 상속받지만, Scene 계층에 대한 참조는 가지지 않는다.

| 포함 금지 | 이유 |
|-----------|------|
| `Scene.h` include | Scene 의존성 생기면 재사용 불가 |
| `NovelScene.h` include | 상위 개념 참조 → 순환 위험 |
| SceneManager 참조 | 위젯은 SceneManager를 몰라야 함 |

위젯은 순수 입력 → 상태 변경 → 렌더링의 단방향 흐름으로 설계한다.

---

## 10. .vcxproj 변경 사항

### ClCompile (신규 등록)

| 파일 | 설명 |
|------|------|
| `Src\Ui\DialogueBox.cpp` | 대사창 구현 |
| `Src\Novel\NovelScene.cpp` | 노벨 씬 구현 |
| `Src\Novel\Background.cpp` | 배경 전환 구현 |
| `Src\Novel\CharacterManager.cpp` | 캐릭터 관리 구현 |

### ClInclude (신규 등록)

| 파일 | 설명 |
|------|------|
| `Src\Ui\BaseWidget.h` | 위젯 베이스 클래스 |
| `Src\Ui\DialogueBox.h` | 대사창 선언 |
| `Src\Novel\NovelScene.h` | 노벨 씬 선언 |
| `Src\Novel\Background.h` | 배경 전환 선언 |
| `Src\Novel\CharacterManager.h` | 캐릭터 관리 선언 |

---

## 11. 컴파일 확인 사항

- `sf::Text`, `sf::Font`, `sf::RectangleShape` 사용 → `sfml-graphics-d.lib` 링크 필요
- `sf::Event::KeyPressed`, `sf::Keyboard::Enter/Space` 사용
- `#include "Core/Common.h"` — `FString`, `VRDX_OVERRIDE` 등 사용
- include 경로: `$(ProjectDir)Src;` 추가 완료 → `#include "Core/Common.h"` 형태 사용 가능

---

## 12. 검증 방법

1. **빌드**: Visual Studio 2022 x64 Debug, Clean Build 성공
2. **실행**: 1280×720 창 열림 (타이틀 "Voradorix"), 배경 → 캐릭터 → 대사창 순서로 표시
3. **대사 진행**:
   - 첫 번째 대사가 타이핑 효과와 함께 출력됨
   - Enter/Space/클릭 → 타이핑 중이면 전체 표시, 완료면 배경 전환 토글 + 첫 대사로 리셋 (테스트 모드)
4. **배경 전환**:
   - 클릭 시 WhiteRoom ↔ WhiteRoom_SunSet 전환, 페이드 효과 동작 확인
5. **메모리**: 종료 시 unique_ptr 정리, leak 없음

---

## 13. 구현 순서

| 단계 | 작업 | 상태 |
|------|------|:----:|
| 1 | `Ui/BaseWidget.h` — CVrdxBaseWidget 선언 | ✅ 완료 |
| 2 | `Ui/DialogueBox.h` — 클래스 선언 (CVrdxBaseWidget 상속) | ✅ 완료 |
| 3 | `Ui/DialogueBox.cpp` — 생성자 (폰트 로드, 패널/텍스트 초기화) | ✅ 완료 |
| 4 | `Ui/DialogueBox.cpp` — SetSpeaker, SetLine, StartTyping, FinishTyping | ✅ 완료 |
| 5 | `Ui/DialogueBox.cpp` — Update (타이핑 타이머) | ✅ 완료 |
| 6 | `Ui/DialogueBox.cpp` — HandleEvent (클릭/키 → FinishTyping) | ✅ 완료 |
| 7 | `Ui/DialogueBox.cpp` — Draw (패널 + 이름 + 대사) | ✅ 완료 |
| 8 | `Novel/NovelScene.h` — 클래스 선언 (Script, DialogueBox, Background, CharacterManager 소유) | ✅ 완료 |
| 9 | `Novel/NovelScene.cpp` — 생성자 (테스트 대사 + 배경 + 캐릭터 초기화) | ✅ 완료 |
| 10 | `Novel/NovelScene.cpp` — ShowNextLine, EndScenario, SwitchBackground | ✅ 완료 |
| 11 | `Novel/NovelScene.cpp` — OnEnter/HandleEvent/Update/Draw (3계층 렌더링) | ✅ 완료 |
| 12 | `Game.vcxproj` + `.filters` 파일 등록 (Background, CharacterManager 포함) | ✅ 완료 |
| 13 | 빌드 확인 및 실행 테스트 | ✅ 완료 |
