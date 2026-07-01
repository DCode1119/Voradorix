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
> **관련 파일**: `Src/Scene/NovelScene.h/cpp`, `Src/Ui/BaseWidget.h`, `Src/Ui/DialogueBox.h/cpp`, `Src/Novel/Background.h/cpp`, `Src/Novel/CharacterManager.h/cpp`, `Src/Novel/ScriptEngine.h/cpp`, `Src/Novel/ScriptLine.h/cpp`  
> **의존성**: SFML 3.1.0 (`sf::Text`, `sf::Font`, `sf::RectangleShape`, `sf::Clock`), `Core/String.h`  
> **완료 조건**: NovelScene이 ScriptEngine을 통해 스크립트 파일을 읽어 배경/캐릭터/대사 명령을 순차 실행, 3계층(BG→캐릭터→UI) 렌더링

---

## 1. 파일 구성

| # | 파일 | 설명 | 상태 |
|---|------|------|------|
| 1 | `Src/Ui/BaseWidget.h` | CVrdxBaseWidget 선언 (위젯 베이스 클래스) | ✅ 작성됨 |
| 2 | `Src/Ui/DialogueBox.h` | CDialogueBox 선언 | ✅ 작성됨 |
| 3 | `Src/Ui/DialogueBox.cpp` | CDialogueBox 구현 | ✅ 작성됨 |
| 4 | `Src/Scene/NovelScene.h` | CVrdxNovelScene 선언 | ✅ 작성됨 |
| 5 | `Src/Scene/NovelScene.cpp` | CVrdxNovelScene 구현 | ✅ 작성됨 |
| 6 | `Src/Novel/ScriptEngine.h/cpp` | ScriptEngine (4단계 추가) | ✅ 작성됨 |
| 7 | `Src/Novel/ScriptLine.h/cpp` | ScriptLine 명령어별 파생 struct (4단계 추가) | ✅ 작성됨 |
| 8 | `Game.vcxproj` | ClCompile/ClInclude 항목 등록 | ✅ 등록 완료 |
| 9 | `Game.vcxproj.filters` | Scene/, Novel/, Ui/ 필터 구성 | ✅ 구성됨 |

---

## 2. 네이밍

| 구분 | 규칙 | 예시 |
|------|------|------|
| 클래스 | `C` + `Vrdx` 접두어 | `CVrdxNovelScene`, `CDialogueBox`, `CVrdxBaseWidget` |
| 파일명 | PascalCase, 디렉토리명과 일치 | `NovelScene.h`, `DialogueBox.cpp`, `BaseWidget.h` |
| 구조체 | `F` + `Vrdx` | `FDialogueLine` |
| 폴더 | `Novel/`, `Ui/` | (Src 아래) |

---

## 3. FVrdxDialogueLine — 대사 데이터 구조체

**위치**: `Src/Novel/DialogueLine.h`

```cpp
struct FVrdxDialogueLine
{
    FVrdxString Speaker;    // 발화자 이름 (빈 문자열이면 나레이션)
    FVrdxString Text;       // 대사 내용
};
```

- `FVrdxString` = UTF-32 기반 유니코드 문자열 (`Core/String.h` 참조)
- Speaker가 비어있으면 나레이션으로 간주 (캐릭터명 영역 미표시)
- ScriptEngine의 `@dialogue` 명령이 Dispatch 시 `FVrdxDialogueLine`를 구성하여 `NovelScene::SetDialogue()` 호출

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

**파일**: `Src/Scene/NovelScene.h` / `Src/Scene/NovelScene.cpp`

### 6.1 역할

`CDialogueBox`, `CVrdxBackground`, `CVrdxCharacterManager`, `CVrdxScriptEngine`를 통합 소유하는 Scene.
ScriptEngine이 스크립트 파일을 해석하여 배경/캐릭터/대사 명령을 순차 실행하고,
NovelScene은 입력 전달과 갱신/렌더 순서를 제어한다.

렌더 순서: **배경 → 캐릭터 → 대사창** (3계층)
갱신 순서: **ScriptEngine → 배경 → 캐릭터 → 대사창**

### 6.2 소유권 구조

```
CVrdxNovelScene
 ├── CVrdxScene (상속)
 ├── std::enable_shared_from_this<CVrdxNovelScene> (상속)
 ├── CVrdxScriptEngine (멤버, 값 소유)
 │    └── TVrdxWeakPtr<CVrdxNovelScene> (→ shared_from_this())
 ├── CVrdxBackground (멤버)
 ├── CVrdxCharacterManager (멤버)
 └── CDialogueBox (멤버)
```

- `enable_shared_from_this` 상속: ScriptEngine이 `weak_ptr`로 NovelScene을 참조할 수 있도록 함
- `OnEnter()`에서 `ScriptEngine.SetNovelScene(shared_from_this())` 호출
- SceneManager는 `TVrdxSharedPtr<CVrdxNovelScene>`로 소유

### 6.3 클래스 설계

```cpp
class CVrdxNovelScene
    : public CVrdxScene
    , public std::enable_shared_from_this<CVrdxNovelScene>
{
public:
    CVrdxNovelScene();
    ~CVrdxNovelScene() VRDX_DEFAULT;

    void OnEnter() VRDX_OVERRIDE;
    void OnExit() VRDX_OVERRIDE;
    void HandleEvent(const sf::Event&) VRDX_OVERRIDE;
    void Update(const float DeltaTick) VRDX_OVERRIDE;
    void Draw(sf::RenderWindow&) VRDX_OVERRIDE;

    // ScriptEngine → NovelScene 호출 (ScriptLine Dispatch 통해 간접 호출)
    bool CanAdvance() const;
    void SetBackground(const FVrdxString& BackgroundName);
    void ShowCharacter(const FVrdxString& Character, EVrdxCharacterPosition Position);
    void HideCharacter(const FVrdxString& Character);
    void SetCharacterPose(const FVrdxString& Character, const FVrdxString& Pose);
    void SetDialogue(const FVrdxDialogueLine& DialogueLine);
    void JumpToLabel(const FVrdxString& TargetLabelName);
    void WaitForSeconds(const float Seconds);

private:
    void EndScenario();

    bool bWaitingInput = false;

    CVrdxBackground Background;
    CVrdxCharacterManager CharacterManager;
    CVrdxDialogueBox DialogueBox;
    CVrdxScriptEngine ScriptEngine;

    float RemainingWaitSeconds = 0.f;
};
```

### 6.4 생성자

- 비어 있음. 초기화는 모두 `OnEnter()`에서 수행.

### 6.5 `OnEnter()`

```cpp
void CVrdxNovelScene::OnEnter()
{
    ScriptEngine.SetNovelScene(shared_from_this());
    if (!ScriptEngine.LoadScript("Assets/Scripts/TestScript.txt"))
    {
        return;
    }
}
```

1. `ScriptEngine.SetNovelScene(shared_from_this())` — ScriptEngine에 `weak_ptr` 전달
2. `ScriptEngine.LoadScript("Assets/Scripts/TestScript.txt")` — 스크립트 파일 로드

> **주의**: `shared_from_this()`는 생성자에서 호출 불가. 반드시 `OnEnter()`(SceneManager가 Push 후 호출)에서 호출.

### 6.6 `HandleEvent(const sf::Event&)`

```cpp
void CVrdxNovelScene::HandleEvent(const sf::Event& Event)
{
    // Enter/Space/클릭 확인
    if (확인 입력)
    {
        if (DialogueBox.IsTyping())
        {
            DialogueBox.FinishTyping();  // 타이핑 중 → 전체 표시
        }
        else
        {
            bWaitingInput = false;       // 입력 완료 → ScriptEngine 진행 허용
        }
    }
}
```

- 타이핑 중 → `FinishTyping()`으로 즉시 전체 표시
- 타이핑 완료 → `bWaitingInput = false` 설정 (ScriptEngine의 `CanAdvance()` 조건 해제)

### 6.7 `Update(const float DeltaTick)`

```
1. ScriptEngine.Update(DeltaTick)
   └─ 내부 while 루프: CanAdvance() && ParseLine() 반복
      └─ ParseLine() → ScriptLine::Dispatch() → NovelScene 메서드 호출
2. Background.Update(DeltaTick)
3. CharacterManager.Update(DeltaTick)
4. DialogueBox.Update(DeltaTick)
5. RemainingWaitSeconds 감소 (0 이하로는 안 내려감)
6. bWaitingInput = DialogueBox.IsFinished()
```

**갱신 순서가 중요한 이유**:
- ScriptEngine이 먼저 실행되어 @bg/@show/@hide/@dialogue 등을 Dispatch
- Background/CharacterManager가 페이드 전환 상태 갱신
- DialogueBox가 타이핑 갱신
- 마지막에 `bWaitingInput`을 DialogueBox 상태에 따라 설정

#### `CanAdvance()` 조건

```cpp
bool CVrdxNovelScene::CanAdvance() const
{
    return !DialogueBox.IsTyping()
        && !(RemainingWaitSeconds > 0)
        && !bWaitingInput;
}
```

- DialogueBox가 타이핑 중이면 대기
- `@wait` 시간이 남아있으면 대기
- 사용자 입력(bWaitingInput)을 기다리면 대기
- 셋 다 해제되어야 ScriptEngine이 다음 줄로 진행

### 6.8 `Draw(sf::RenderWindow& Window)`

1. `Window.clear(sf::Color::Black)` — 배경 초기화
2. `Background.Draw(Window)` — 배경 레이어
3. `CharacterManager.Draw(Window)` — 캐릭터 레이어
4. `DialogueBox.Draw(Window)` — UI 최상단

### 6.9 ScriptEngine 연동 메서드

다음 메서드들은 ScriptLine의 `Dispatch()`를 통해 ScriptEngine이 간접 호출한다.

| NovelScene 메서드 | 호출 명령어 | 내부 동작 |
|---|---|---|
| `SetBackground(Name)` | `@bg "Name"` | `Background.SetBackground(Name)` |
| `ShowCharacter(Char, Pos)` | `@show "Char" "Pos"` | `CharacterManager.ShowCharacter(Char, Pos)` |
| `HideCharacter(Char)` | `@hide "Char"` | `CharacterManager.HideCharacter(Char)` |
| `SetCharacterPose(Char, Pose)` | `@pose "Char" "Pose"` | `CharacterManager.SetCharacterPose(Char, Pose)` |
| `SetDialogue(DialogueLine)` | `@dialogue "S" "T"` | `DialogueBox.SetSpeaker()` + `SetLine()` (→ 자동 타이핑 시작) |
| `WaitForSeconds(Sec)` | `@wait "Sec"` | `RemainingWaitSeconds = Sec` |
| `JumpToLabel(Target)` | `@jump "Target"` | `ScriptEngine.JumpToLabel(Target)` |

#### `SetDialogue()` 상세

```cpp
void CVrdxNovelScene::SetDialogue(const FVrdxDialogueLine& DialogueLine)
{
    DialogueBox.SetSpeaker(DialogueLine.Speaker);
    DialogueBox.SetLine(DialogueLine.Text);
    // SetLine() 내부에서 StartTyping() 호출 → IsTyping() 즉시 true
}
```

- `@dialogue` Dispatch는 `true` 반환 (즉시 진행 가능)
- 하지만 `SetLine()`이 `StartTyping()`을 호출하므로 `IsTyping()`이 true가 됨
- → `CanAdvance()`가 false를 반환하여 ScriptEngine 루프 중단
- → 사용자 입력 대기 상태로 전환

#### `WaitForSeconds()` 상세

```cpp
void CVrdxNovelScene::WaitForSeconds(const float Seconds)
{
    RemainingWaitSeconds = Seconds;
}
```

- `@wait` Dispatch 후 `RemainingWaitSeconds > 0`이므로 `CanAdvance()` false
- `Update()`에서 매 프레임 감소, 0이 되면 해제

#### `JumpToLabel()` 상세

```cpp
void CVrdxNovelScene::JumpToLabel(const FVrdxString& TargetLabelName)
{
    ScriptEngine.JumpToLabel(TargetLabelName);
}
```

- ScriptEngine의 `Labels` 맵에서 대상 레이블 인덱스 조회
- `CurrentScriptLine`을 해당 인덱스로 설정
- `@label` 줄 자체는 no-op (Dispatch에서 아무 동작 안 함)

### 6.10 `EndScenario()`

```cpp
void CVrdxNovelScene::EndScenario()
{
    RequestExit();
}
```

- ScriptEngine이 `IsFinished()`를 반환할 때까지 진행
- 종료 후 NovelScene이 `RequestExit()` → SceneManager가 Pop → 스택 empty → Application 종료

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

ScriptEngine.h
├── ScriptLine.h
│     ├── Core/Common.h
│     ├── Core/String.h
│     ├── Core/Vector.h
│     └── Novel/CharacterManager.h
└── (전방 선언: CVrdxNovelScene)

NovelScene.h
├── Scene/Scene.h
│     └── Core/Common.h
├── Ui/DialogueBox.h
│     └── Ui/BaseWidget.h
├── Novel/Background.h
├── Novel/CharacterManager.h
├── Novel/DialogueLine.h
└── Novel/ScriptEngine.h

NovelScene.cpp
├── Scene/NovelScene.h
├── SFML/Window/Event.hpp
└── (ScriptEngine.cpp → #include "Scene/NovelScene.h")

ScriptEngine.cpp
├── Novel/ScriptEngine.h
├── Scene/NovelScene.h  (weak_ptr lock 후 CanAdvance/메서드 호출)
└── <fstream>, <string>

ScriptLine.cpp
├── Novel/ScriptLine.h
├── Scene/NovelScene.h  (Dispatch에서 shared_ptr<NovelScene> 메서드 호출)
└── <regex>, <unordered_map>, <string>
```

- 순환 의존성 없음
- DialogueBox는 Scene 시스템에 대해 전혀 모름 (완전히 독립적인 위젯)
- ScriptEngine/ScriptLine은 `Scene/NovelScene.h`를 include (구체 타입 필요)
  - 약한 순환: NovelScene → ScriptEngine (멤버), ScriptEngine → NovelScene (weak_ptr)
  - `shared_ptr` + `weak_ptr`로 소유권 방향 단방향 유지

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

### ClCompile (2단계 + 4단계 통합)

| 파일 | 설명 | 단계 |
|------|------|:----:|
| `Src\Ui\DialogueBox.cpp` | 대사창 구현 | 2 |
| `Src\Scene\NovelScene.cpp` | 노벨 씬 구현 | 2 (4단계 수정) |
| `Src\Novel\Background.cpp` | 배경 전환 구현 | 3 |
| `Src\Novel\CharacterManager.cpp` | 캐릭터 관리 구현 | 3 |
| `Src\Novel\ScriptEngine.cpp` | 스크립트 엔진 구현 | 4 |
| `Src\Novel\ScriptLine.cpp` | 스크립트 라인 파싱/실행 | 4 |

### ClInclude (2단계 + 4단계 통합)

| 파일 | 설명 | 단계 |
|------|------|:----:|
| `Src\Ui\BaseWidget.h` | 위젯 베이스 클래스 | 2 |
| `Src\Ui\DialogueBox.h` | 대사창 선언 | 2 |
| `Src\Scene\NovelScene.h` | 노벨 씬 선언 | 2 (4단계 수정) |
| `Src\Novel\Background.h` | 배경 전환 선언 | 3 |
| `Src\Novel\CharacterManager.h` | 캐릭터 관리 선언 | 3 |
| `Src\Novel\DialogueLine.h` | 대사 데이터 구조체 | 4 |
| `Src\Novel\ScriptEngine.h` | 스크립트 엔진 선언 | 4 |
| `Src\Novel\ScriptLine.h` | 스크립트 라인 선언 | 4 |

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
3. **스크립트 진행**:
   - `Assets/Scripts/TestScript.txt`를 ScriptEngine이 로드
   - `@bg` → 배경 전환, `@show` → 캐릭터 등장, `@dialogue` → 대사 타이핑 출력
   - Enter/Space/클릭 → 타이핑 중이면 전체 표시, 완료면 다음 명령 진행
4. **분기 테스트**:
   - `@jump`가 레이블로 정확히 이동하는지 확인 (A → B → Finish 순환)
   - `@wait` 동안 진행이 멈추는지 확인
5. **메모리**: 종료 시 shared_ptr 정리 — leak 없음

---

## 13. 구현 순서

### 2단계 — NovelScene + DialogueBox 기반

| 단계 | 작업 | 상태 |
|------|------|:----:|
| 1 | `Ui/BaseWidget.h` — CVrdxBaseWidget 선언 | ✅ 완료 |
| 2 | `Ui/DialogueBox.h` — 클래스 선언 (CVrdxBaseWidget 상속) | ✅ 완료 |
| 3 | `Ui/DialogueBox.cpp` — 생성자 (폰트 로드, 패널/텍스트 초기화) | ✅ 완료 |
| 4 | `Ui/DialogueBox.cpp` — SetSpeaker, SetLine, StartTyping, FinishTyping | ✅ 완료 |
| 5 | `Ui/DialogueBox.cpp` — Update (타이핑 타이머) | ✅ 완료 |
| 6 | `Ui/DialogueBox.cpp` — HandleEvent (클릭/키 → FinishTyping) | ✅ 완료 |
| 7 | `Ui/DialogueBox.cpp` — Draw (패널 + 이름 + 대사) | ✅ 완료 |
| 8 | `Scene/NovelScene.h` — 클래스 선언 (Background, CharacterManager, DialogueBox 소유) | ✅ 완료 |
| 9 | `Scene/NovelScene.cpp` — 생성자 (초기화) | ✅ 완료 |
| 10 | `Scene/NovelScene.cpp` — Update/Draw (3계층 렌더링) | ✅ 완료 |
| 11 | `Game.vcxproj` + `.filters` 파일 등록 | ✅ 완료 |
| 12 | 빌드 확인 및 실행 테스트 | ✅ 완료 |

### 4단계 — ScriptEngine 연동

| 단계 | 작업 | 상태 |
|------|------|:----:|
| 1 | SceneManager `unique_ptr` → `shared_ptr` 전환 | ✅ 완료 |
| 2 | NovelScene에 `enable_shared_from_this` 상속 추가 | ✅ 완료 |
| 3 | `ScriptEngine.h/cpp` — SetNovelScene, LoadScript, 레이블 맵 구축 | ✅ 완료 |
| 4 | `ScriptLine.h/cpp` — 명령어별 파생 struct, 팩토리 테이블, Construct/Dispatch | ✅ 완료 |
| 5 | NovelScene에 `CanAdvance`, `SetDialogue`, `WaitForSeconds`, `JumpToLabel` 추가 | ✅ 완료 |
| 6 | NovelScene `OnEnter()`에서 ScriptEngine 초기화 (shared_from_this() + LoadScript) | ✅ 완료 |
| 7 | NovelScene `HandleEvent()` → DialogueBox typing/input 전환 | ✅ 완료 |
| 8 | NovelScene `Update()` 순서 확정 (ScriptEngine → BG → Char → DialogueBox) | ✅ 완료 |
| 9 | `@bg`, `@show`, `@hide`, `@pose`, `@wait`, `@label`, `@jump`, `@dialogue` 8개 명령어 구현 | ✅ 완료 |
| 10 | `Assets/Scripts/TestScript.txt` — @label/@jump 분기 포함 샘플 | ✅ 완료 |
| 11 | `Game.vcxproj` + `.filters` — ScriptEngine, ScriptLine 등록 | ✅ 완료 |
| 12 | 빌드 확인 및 실행 테스트 | ✅ 완료 |
