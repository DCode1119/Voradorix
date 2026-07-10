---
title: "6단계: UI Foundation"
project: Voradorix
phase: 6
status: 구현 진행 중
tags:
  - voradorix
  - ui
  - widget
  - foundation
  - spec
---
# Voradorix — 6단계: UI Foundation 기능 명세

> **대상**: `CVrdxWidgetBase` 확장 + ImageWidget/TextLabel/Button/Container 같은 재사용 UI 컴포넌트  
> **프로젝트**: Voradorix  
> **관련 파일**: `Src/Ui/WidgetBase.h`, `Src/Ui/*.h/cpp`, `Src/Scene/*Scene.h/cpp`  
> **의존성**: `Core/String.h`, `Core/Vector.h`, SFML 3.1.0 (`sf::Event`, `sf::RenderWindow`, `sf::RectangleShape`, `sf::Text`)  
> **완료 조건**: 타이틀/설정/저장화면/노벨 UI가 공통 위젯 계층 위에서 동작하고, 입력/렌더/레이아웃이 UI 계층 내부로 분리된다.

---

## 1. 배경

현재 UI는 `CVrdxWidgetBase`를 중심으로 한 위젯 트리로 정리되었고, `DialogueBox`와 `ChoiceWidget`가 그 위에 올라가 있다.
이 구조는 노벨 재생 단계에서는 동작하지만, 타이틀 화면/설정/저장화면처럼 **UI 비중이 큰 화면**을 추가할 때 위젯 공통 계층이 더 필요하다.

따라서 현재는 UI Foundation을 진행 중인 단계로 두고, 7단계(Save/Load)와 8단계(메뉴 구성) 이전에 공통 UI 기초 계층을 먼저 정리한다.

---

## 2. 범위

- 공통 위젯 생명주기 정의
- 계층형 UI 컨테이너
- Hover / Leave / Click / Keyboard 입력 전달
- 재사용 가능한 ImageWidget / TextLabel / Button 컴포넌트
- 기존 `DialogueBox`, `ChoiceWidget`을 위젯 기반으로 정리

이번 단계에서는 다음을 다루지 않는다.

- 테마/스킨 시스템
- 애니메이션 프레임워크
- 스크롤/가상화 리스트
- 복잡한 포커스 탐색 규칙

---

## 3. 설계 원칙

- Scene는 화면 상태와 전환만 담당한다.
- Widget은 입력과 렌더, 상태 표시만 담당한다.
- UI는 조립 가능한 작은 단위로 나눈다.
- 하위 위젯은 상위 컨테이너를 통해 이벤트와 렌더 순서를 상속받는다.

---

## 4. 공통 위젯 계층

### 4.1 `CVrdxWidgetBase`

모든 UI 위젯의 공통 베이스이자 위젯 트리 컨테이너 역할을 담당한다.

```cpp
class CVrdxWidgetBase
{
public:
    virtual ~CVrdxWidgetBase() VRDX_DEFAULT;

    virtual bool HandleEvent(const sf::Event& Event);
    virtual void Update(const float DeltaTick);
    virtual void Draw(sf::RenderWindow& Window) const;

    virtual void OnPostCreate() {}
    virtual void OnPreDestroy() {}
};
```

실제 구현 상태:

- `Parent` / `Children` (위젯 트리 관계 관리)
- `Shape`(상대 좌표)
- `SetVisibility()` / `GetVisibility()`
- `SetIgnoreEvent()` / `bIgnoreEvent` (자식 표시 전용 위젯 입력 차단)
- `SetCapture()` / `bIsCapturing` (마우스 캡처 상태)
- `IsDrawable()` / `bCanBeDrawn` (완전 가림 시 Draw 생략)
- `MapToGlobal()` / `MapToLocal()`
- `Hides()`

위젯 생성/해제는 `CreateWidget()` / `DestroyWidget()`로 관리하며,
생성 직후 `OnPostCreate()`, 해제 직전 `OnPreDestroy()`를 호출한다.

---

## 5. 기본 컴포넌트

### 5.1 CVrdxBoxWidget (ImageWidget)

이미지/텍스처를 선택적으로 그릴 수 있는 위젯.

- 텍스처를 지정하지 않으면 `CVrdxWidgetBase`와 동일하게 동작
- 텍스처/스프라이트를 지정하면 해당 이미지를 위젯 영역에 출력

- 이미지/텍스처
- 출력 비율/정렬
- 필요 시 배경색 보조 출력

### 5.2 TextLabel

텍스트를 표시하는 위젯. 현재는 `CVrdxBoxWidget`를 기반으로 `sf::Text`를 추가한 형태로 구현되어 있다.

- `SetText()`
- `SetFont()`
- `SetFontSize()`
- `SetFontColor()`

현재는 폰트 리소스를 `AssetManager` 경유로 공유하며, `DialogueBox` / `ChoiceWidget` / `TextLabel`이 동일 자원을 사용한다.

### 5.3 Button

클릭 가능한 상호작용 위젯. 현재는 `Normal` / `Pressed` / `Text` 자식 위젯을 조합해 상태를 표시하는 구조로 구현 완료 상태다.

- Hover 상태
- Pressed 상태
- Focus 상태
- Keyboard Activate 상태
- 텍스트 라벨 포함 가능

현재 구현 상태:

- `SetCallback()` — 클릭 시 실행할 콜백 등록
- `SetText()` — 버튼 텍스트 설정
- `SetColors()` / `SetLineColors()` — 정상/눌림 상태 색상 설정
- `SetCapture()` 기반으로 누르는 동안 상태 유지

추후 폴리싱 항목:

- Hover 상태
- Focus 상태
- Keyboard Activate 상태
- 텍스트 정렬/서식

---

## 6. 입력/상호작용 규칙

- 마우스 이동 시 Hover 대상 갱신
- 좌클릭 시 현재 Hover 대상 활성화
- Enter/Space는 Focus 대상 활성화
- Leave는 이전 Hover 상태 해제
- 컨테이너는 자식 위젯으로 입력을 전달할 수 있어야 한다

우선순위:

1. Hover 갱신
2. Click 처리
3. Keyboard Activate
4. Update/Draw

---

## 7. 레이아웃 규칙

- 초기에는 절대 좌표 기반으로 시작
- 이후 앵커/정렬/패딩 확장 가능
- 컨테이너 내부 자식은 상대 좌표로 배치하는 방향을 권장한다

권장 필드:

- `Padding`
- `Spacing`
- `Alignment`
- `Anchor`

---

## 8. 적용 대상

- `DialogueBox`
- `ChoiceWidget`
- `TitleScene`
- `ConfigScene`
- `SaveLoadScene`

즉, UI가 있는 모든 화면은 이 계층을 기준으로 점진적으로 재구성한다.

---

## 9. 구현 순서

| 단계 | 작업 | 상태 |
|------|------|------|
| 1 | `CVrdxWidgetBase` 정리 | ✅ 완료 |
| 2 | `CVrdxBoxWidget` / `TextLabel` 작성 | ✅ 완료 |
| 3 | `Button` 작성 | ✅ 완료 |
| 4 | `DialogueBox` 위젯 정리 | ✅ 완료 |
| 5 | `ChoiceWidget` 위젯 정리 | ✅ 완료 |
| 6 | `TitleScene` / `ConfigScene` / `SaveLoadScene` 적용 | 📝 예정 |

---

## 10. 검증 항목

- 위젯끼리 중첩해도 입력/렌더 순서가 일관적인가
- Hover/Click/Keyboard가 각각 정상 동작하는가
- Scene 코드에 UI 렌더/입력 로직이 과도하게 남지 않는가
- Dialogue/Choice/Title/SaveLoad에서 공통 UI 로직이 재사용되는가
