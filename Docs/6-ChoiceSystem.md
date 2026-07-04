---
title: "5단계: ChoiceSystem"
project: Voradorix
phase: 5
status: 구현 완료
tags:
  - voradorix
  - choice-system
  - branching
  - cpp
---
# Voradorix — 5단계: ChoiceSystem 구현 문서

> **대상**: 선택지 UI/입력 처리 + ScriptEngine 분기 연동 + NovelScene 흐름 제어  
> **프로젝트**: Voradorix  
> **관련 파일**: `Src/Ui/ChoiceWidget.h/cpp`, `Src/Novel/ScriptLine.h/cpp`, `Src/Scene/NovelScene.h/cpp`, `Assets/Scripts/TestScript.txt`  
> **의존성**: `Core/String.h`, `Core/Vector.h`, SFML 3.1.0 (`sf::Text`, `sf::Font`, `sf::RectangleShape`, `sf::Event`)  
> **완료 조건**: `@choice`가 나오면 선택지 패널이 표시되고, 사용자가 선택한 항목의 레이블로 즉시 분기된다.

---

## 1. 범위

5단계는 **분기 선택지 시스템**을 담당한다.

- `@choice` 명령 파싱 및 실행
- 선택지 패널/버튼 표시
- 키보드/마우스 입력 처리
- 선택 시 즉시 `NovelScene::JumpToLabel()`로 분기

이번 단계에서는 세이브/로드, 조건 분기, 메뉴/타이틀, 연출 효과는 다루지 않는다.

---

## 2. 파일 구성

| # | 파일 | 설명 | 상태 |
|---|------|------|------|
| 1 | `Src/Ui/ChoiceWidget.h` | `CVrdxChoiceWidget` 선언 | ✅ 완료 |
| 2 | `Src/Ui/ChoiceWidget.cpp` | `CVrdxChoiceWidget` 구현 | ✅ 완료 |
| 3 | `Src/Novel/ScriptLine.h` | `FVrdxChoiceScriptLine` 추가 | ✅ 완료 |
| 4 | `Src/Novel/ScriptLine.cpp` | `@choice` 파싱/Construct/Dispatch 추가 | ✅ 완료 |
| 5 | `Src/Scene/NovelScene.h/cpp` | ChoiceWidget 소유 및 흐름 연동 | ✅ 완료 |
| 6 | `Game.vcxproj` + `.filters` | ChoiceWidget 등록 | ✅ 완료 |
| 7 | `Assets/Scripts/TestScript.txt` | 선택지 검증용 샘플 스크립트 | ✅ 완료 |

---

## 3. 스크립트 문법

### 3.1 기본 형식

`@choice`는 한 줄에 `text`/`label` 쌍을 나열한다.

```text
@choice "미안해" "Apology" "변명하지마" "Defend"
```

규칙:

- 첫 인자는 선택지 텍스트, 두 번째 인자는 이동할 레이블 이름
- 이후에도 `text`/`label` 쌍으로 반복
- 선택지는 최소 1개 이상
- 총 인자 수는 `1 + (2 * N)` 형식
- 최대 4개 선택지를 지원

### 3.2 지원 범위

- `@choice` → `ChoiceWidget`가 대기 상태로 진입
- 선택 완료 → `NovelScene::JumpToLabel()` 즉시 호출

---

## 4. 내부 데이터 구조

### 4.1 선택지 데이터

```cpp
struct FVrdxChoiceOption
{
    FVrdxString Text;
    FVrdxString TargetLabel;
};
```

- `Text`와 `TargetLabel`은 비어 있으면 안 됨
- 순서는 스크립트에 적힌 그대로 유지

### 4.2 선택 상태

```cpp
enum class EVrdxChoiceState
{
    Hidden,
    Waiting,
};
```

- `Hidden`  → 선택지가 표시되지 않는 상태
- `Waiting` → 선택 입력을 대기하는 상태
- 선택 확정은 별도 보관 상태 없이 즉시 분기한다.

---

## 5. CVrdxChoiceWidget — 선택지 UI 담당

### 5.1 역할

선택지 버튼을 표시하고, 키보드/마우스 입력으로 선택을 받아 즉시 분기한다.

### 5.2 클래스 개요

```cpp
class CVrdxChoiceWidget
{
public:
    CVrdxChoiceWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

    void SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene);
    void SetChoices(const TVrdxVector<FVrdxChoiceOption>& Choices);
    void Clear();

    void Update(const float DeltaTick);
    void Draw(sf::RenderWindow& Window) const;

    bool IsWaiting() const;

    virtual void OnMouseMove(const sf::Vector2f& LocalPosition) override;
    virtual void OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition) override;
    virtual void OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode) override;

private:
    void MoveSelection(int32_t Delta);
    void ConfirmSelection();
    int32_t GetHoveredButtonIndex(const sf::Vector2i& MousePosition) const;

    TVrdxVector<FVrdxChoiceOption> ChoiceOptions;
    int32_t SelectedIndex = 0;
    EVrdxChoiceState ChoiceState = EVrdxChoiceState::Hidden;

    sf::Font Font;
    bool bFontLoaded = false;

    TVrdxVector<sf::RectangleShape> Buttons;
    TVrdxVector<sf::Text> ButtonTexts;

    TVrdxWeakPtr<CVrdxNovelScene> WeakNovelScene;
    bool bInvalidated = false;
};
```

### 5.3 UI 동작

- 기본 위치: 화면 하단 대사창 위의 중앙 영역
- 버튼은 세로 목록으로 배치
- 현재 선택 항목은 강조 표시
- 입력 방식:
  - 위/아래 또는 W/S → 항목 이동
  - Enter/Space/Left Click → 선택 확정
  - 마우스 이동 → Hover 항목 갱신

### 5.4 메서드 의미

| 메서드 | 의미 |
|--------|------|
| `SetChoices()` | 선택지 목록 설정 및 `Waiting` 진입 |
| `Clear()` | 선택지와 상태 초기화 |
| `HandleEvent()` | 키보드/마우스 입력 반영 |
| `Update()` | 버튼/텍스트 재구성 및 하이라이트 갱신 |
| `Draw()` | 패널/버튼/텍스트 렌더링 |
| `ConfirmSelection()` | 현재 선택 항목의 레이블로 즉시 분기 |

---

## 6. ScriptLine 연동

### 6.1 `FVrdxChoiceScriptLine`

`ScriptLine.h/cpp`에 추가된 파생 라인.

```cpp
struct FVrdxChoiceScriptLine : FVrdxScriptLine
{
    virtual bool Construct() override;
    virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

    TVrdxVector<FVrdxChoiceOption> ChoiceOptions;
};
```

### 6.2 파싱 규칙

- `Arguments[0]`는 항상 `choice`
- `Arguments[1]`부터 `Text`, `TargetLabel` 쌍으로 해석
- 인자 수가 홀수이거나 3개 미만이면 실패
- `Construct()` 단계에서 형식 검증

### 6.3 Dispatch 의미

```cpp
bool FVrdxChoiceScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
    Scene->SetChoices(ChoiceOptions);
    return false;
}
```

- `false` 반환: ScriptEngine의 연속 진행을 중단
- 실제 분기는 `ChoiceWidget::ConfirmSelection()`에서 즉시 처리

---

## 7. NovelScene 연동

### 7.1 추가 멤버

```cpp
TVrdxSharedPtr<CVrdxChoiceWidget> ChoiceWidget;
```

### 7.2 흐름

1. `@choice` Dispatch → `NovelScene::SetChoices()`
2. `ChoiceWidget.SetChoices()` → `Waiting` 상태 진입
3. `CanAdvance()`가 false가 되어 ScriptEngine 진행 정지
4. 사용자가 선택하면 `ChoiceWidget::ConfirmSelection()`이 즉시 `NovelScene::JumpToLabel()` 호출
5. `ChoiceWidget.Clear()` 후 다음 프레임부터 ScriptEngine 재개

### 7.3 렌더 순서

1. Background
2. CharacterManager
3. 위젯 루트(Applications)에서 DialogueBox / ChoiceWidget 렌더

---

## 8. 실행 규칙

```text
ScriptEngine.Update()
  → @choice 발견
  → NovelScene.SetChoices()
  → ChoiceWidget.Waiting
  → 사용자 입력
  → ChoiceWidget.ConfirmSelection()
  → NovelScene.JumpToLabel(target)
  → ChoiceWidget.Clear()
  → ScriptEngine 재개
```

`CanAdvance()` 조건:

```cpp
return !DialogueBox.IsTyping()
    && !(RemainingWaitSeconds > 0)
    && !ChoiceWidget.IsWaiting();
```

---

## 9. 오류 처리

- **인자 수 부족**: `@choice` 파싱 실패
- **짝이 맞지 않는 인자 수**: 파싱 실패
- **빈 텍스트/빈 레이블**: 파싱 실패
- **선택지 수 초과**: 최대 4개로 자동 절삭
- **존재하지 않는 레이블**: `JumpToLabel()` 단계에서 무시 또는 경고

> 현재 로깅 계층이 없으므로, 1차 구현에서는 실패를 조용히 무시하고 추후 로그를 추가한다.

---

## 10. 테스트 스크립트 예시

```text
@label "Main"
@dialogue "Laura" "어떤 선택을 할까요?"
@choice "사과한다" "Apology" "반문한다" "Defend"

@label "Apology"
@dialogue "Laura" "사과를 선택했습니다."
@jump "End"

@label "Defend"
@dialogue "Laura" "반문을 선택했습니다."
@jump "End"

@label "End"
@dialogue "Laura" "분기 테스트 종료."
```

---

## 11. 구현 결과

- `@choice` 파싱/실행 완료
- 마우스/키보드 선택 입력 완료
- hover 기반 선택 갱신 완료
- 즉시 분기 점프 완료
- `NovelScene` 흐름 제어 + 위젯 기반 선택지 렌더 완료
