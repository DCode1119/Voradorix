---
title: "9단계: EffectManager"
project: Voradorix
phase: 9
status: 보류
tags:
  - voradorix
  - effect
  - visual
  - spec
  - cpp
---
# Voradorix — 9단계: 연출 효과 (EffectManager)

> **⚠️ 보류 (2026-07-10)** — 프로젝트 방향 전환(게임 개발 → 엔진+에디터 개발)으로 인해
> 이 단계의 구현은 후순위로 미루어졌습니다.
> 자세한 사항은 [[Design/PROJECT_DIRECTION.md]] 참조.

> **대상**: 화면 페이드, 셰이크, 컬러 오버레이 등 연출 효과  
> **프로젝트**: Voradorix  
> **관련 파일**: `Novel/EffectManager.h/cpp`, `Novel/NovelScene.h/cpp`, `Ui/Application.h/cpp`  
> **의존성**: `Ui/WidgetBase.h`, `Core/Common.h`, SFML 3.1.0  
> **완료 조건**: NovelScene 재생 중 스크립트 명령어로 화면 페이드, 셰이크, 컬러 오버레이 효과를 실행할 수 있고, 복수의 효과를 순차적으로 큐잉할 수 있음

---

## 1. 범위

9단계는 **연출 효과(Effect) 시스템**을 담당한다.

- 화면 전체 페이드인/페이드아웃
- 화면 흔들림(Shake) 효과
- 컬러 오버레이 (흑백, 세피아, 단색 필터)
- 효과 큐(Queue) — 복수의 효과를 순차적으로 실행
- ScriptEngine `@fadein`, `@fadeout`, `@shake` 명령어 연동
- 기존 페이드 효과와의 통합 (Background의 페이드 전환과 화면 페이드는 별개)

---

## 2. 아키텍처

### 2.1 위젯 트리 위치

```
Application (CVrdxApplication — Root)
├── NovelScene (CVrdxNovelScene)
│   ├── Background
│   ├── CharacterManager
│   ├── DialogueBox
│   ├── ChoiceWidget
│   └── EffectManager (CVrdxEffectManager)    ← 신규
├── TitleWindow (CVrdxTitleWindow)
└── SaveLoadWindow (CVrdxSaveLoadWindow)
```

- **EffectManager는 NovelScene의 자식**으로 생성한다.
- NovelScene 내의 모든 위젯 위에 렌더링되어 화면 전체 효과를 표현한다.
- `BringToFront()`로 NovelScene 자식 중 가장 마지막에 렌더링되도록 보장한다.

### 2.2 설계 이유

- 효과는 NovelScene이 활성화된 동안에만 의미가 있으므로 NovelScene이 소유한다.
- TitleWindow나 SaveLoadWindow 위에 효과가 표시될 필요는 없다.
- Background의 자체 페이드 전환(배경 이미지 교체용)과 화면 전체 페이드(장면 전환용)는 별도로 동작한다.

---

## 3. 데이터 구조

### 3.1 효과 종류 열거형

```cpp
enum class EVrdxEffectType
{
    FadeIn,         // 화면이 검정→투명
    FadeOut,        // 화면이 투명→검정
    Shake,          // 화면 흔들림
    ColorOverlay,   // 단색 오버레이 (R, G, B, Alpha)
};
```

### 3.2 효과 명령 구조체

```cpp
struct FVrdxEffectCommand
{
    EVrdxEffectType Type;
    float Duration = 0.f;         // 효과 지속 시간 (초)
    float Amplitude = 0.f;        // 진폭 (Shake: 픽셀 거리)
    sf::Color OverlayColor = sf::Color::Black;  // FadeOut/ColorOverlay 용
    // 내부 상태
    float Elapsed = 0.f;
    bool bCompleted = false;
};
```

### 3.3 캡처 상태 구조체 (Shake용)

```cpp
struct FVrdxShakeState
{
    bool bActive = false;
    float Duration = 0.f;
    float Elapsed = 0.f;
    float Intensity = 0.f;        // 최대 흔들림 픽셀
    sf::Vector2f Offset = {0, 0}; // 현재 프레임 오프셋
};
```

---

## 4. 클래스 시그니처

```cpp
class CVrdxEffectManager : public CVrdxWidgetBase
{
public:
    CVrdxEffectManager(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget,
                       const sf::RectangleShape& InShape);

    // WidgetBase hooks
    virtual void OnPostCreate() override;
    virtual void Update(float DeltaTick) override;
    virtual void Draw(sf::RenderWindow& Window) const override;

    // ——— 효과 실행 API ———

    // 페이드인: 화면이 검정에서 투명해짐 (Duration 초 동안)
    void FadeIn(float Duration);

    // 페이드아웃: 화면이 투명에서 검정이 됨 (Duration 초 동안)
    void FadeOut(float Duration);

    // 화면 흔들림 (Duration 초 동안, Intensity 픽셀 범위)
    void Shake(float Duration, float Intensity);

    // 단색 오버레이 (Color 색상으로 Duration 초 동안 페이드인, 유지 후 페이드아웃)
    void ColorOverlay(const sf::Color& Color, float FadeInDuration,
                      float HoldDuration, float FadeOutDuration);

    // ——— 큐(Queue) 시스템 ———

    // 효과를 큐에 추가 (현재 실행 중인 효과가 끝난 후 순차 실행)
    void EnqueueEffect(const FVrdxEffectCommand& Command);

    // 모든 효과를 즉시 중단하고 큐를 비움
    void ClearAllEffects();

    // ——— 상태 조회 ———

    bool IsAnyEffectActive() const;
    bool IsFading() const;          // 현재 페이드 중인지
    sf::Color GetCurrentOverlay() const;  // 현재 오버레이 색상 (알파 포함)

    // ——— NovelScene 연동 ———

    // NovelScene이 셰이크 오프셋을 조회할 때 사용 (Draw position offset)
    sf::Vector2f GetShakeOffset() const { return ShakeState.Offset; }

private:
    // 내부 헬퍼
    void UpdateFade(float DeltaTick);
    void UpdateShake(float DeltaTick);
    void UpdateColorOverlay(float DeltaTick);
    void ProcessQueue();

    // 현재 활성 효과
    bool bFadeActive = false;
    EVrdxEffectType FadeDirection;   // FadeIn or FadeOut
    float FadeDuration = 0.f;
    float FadeElapsed = 0.f;

    sf::Color OverlayColor = sf::Color::Transparent;  // 현재 오버레이 (렌더링용)

    // 셰이크
    FVrdxShakeState ShakeState;

    // 컬러 오버레이 (ColorOverlay 전용 상태)
    struct FColorOverlayState
    {
        bool bActive = false;
        sf::Color TargetColor;
        float FadeInDuration = 0.f;
        float HoldDuration = 0.f;
        float FadeOutDuration = 0.f;
        float Elapsed = 0.f;
        enum class EPhase { FadeIn, Hold, FadeOut, Done } Phase = EPhase::Done;
    };
    FColorOverlayState ColorOverlayState;

    // 효과 큐
    TVrdxVector<FVrdxEffectCommand> EffectQueue;
};
```

---

## 5. 동작 상세

### 5.1 페이드인 (FadeIn)

| 단계 | 시간 | 오버레이 알파 |
|------|------|---------------|
| 시작 | t=0 | 255 (불투명 검정) |
| 진행 | 0 < t < Duration | 255 → 0 선형 보간 |
| 완료 | t=Duration | 0 (완전 투명) |

- `Draw()`에서 `sf::RectangleShape`로 화면 전체를 알파 블렌딩하여 렌더링
- 색상은 검정 고정

### 5.2 페이드아웃 (FadeOut)

| 단계 | 시간 | 오버레이 알파 |
|------|------|---------------|
| 시작 | t=0 | 0 (완전 투명) |
| 진행 | 0 < t < Duration | 0 → 255 선형 보간 |
| 완료 | t=Duration | 255 (불투명 검정) |

### 5.3 화면 흔들림 (Shake)

- 매 프레임 `ShakeState.Offset`을 랜덤 오프셋으로 갱신
- 오프셋 범위: `[-Intensity, +Intensity]` (X, Y 각각)
- NovelScene의 `Draw()`가 `EffectManager->GetShakeOffset()`을 조회하여 자식들의 Draw position에 offset 적용
- Duration 경과 후 Offset = {0, 0}으로 복귀

**셰이크 적용 방식 (NovelScene 측):**

```cpp
void CVrdxNovelScene::Draw(sf::RenderWindow& Window) const
{
    sf::Vector2f ShakeOffset = EffectManager->GetShakeOffset();
    // 모든 Draw 호출 전에 Viewport 또는 transform 적용
    // 방법 A: sf::View를 offset하여 Window.setView()
    // 방법 B: 각 자식 Draw 전에 Window 포지션 오프셋 (복잡)
}
```

**권장 방식:** `Window`의 `sf::View`를 offset하여 화면 전체를 이동시킨 후 자식 Draw, 이후 복원.

### 5.4 컬러 오버레이 (ColorOverlay)

| 단계 | 시간 | 오버레이 알파 |
|------|------|---------------|
| FadeIn | 0 → FadeInDuration | 0 → TargetColor.a |
| Hold | FadeInDuration → +HoldDuration | TargetColor.a 유지 |
| FadeOut | → +FadeOutDuration | TargetColor.a → 0 |
| Done | — | 투명 |

- `Draw()`에서 지정된 Color로 화면 전체 사각형을 알파 블렌딩

### 5.5 큐 시스템

- `EnqueueEffect()`로 `FVrdxEffectCommand`를 `EffectQueue`에 추가
- `Update()`에서 현재 활성 효과가 없으면 큐에서 다음 효과를 꺼내 실행
- 한 번에 하나의 효과만 활성 (셰이크는 Fade/ColorOverlay와 병렬 가능 여부는 설계 선택)

**큐 예시:**

```cpp
// @fadeout "1.0"
// @wait "0.5"
// @fadein "1.0"
```

큐 → FadeOut(1.0s) → 완료 후 자동으로 FadeIn(1.0s) 실행

### 5.6 NovelScene과의 협력

NovelScene은 EffectManager를 다음과 같이 사용한다:

```cpp
// NovelScene::OnPostCreate() — EffectManager 생성
sf::RectangleShape EffectShape;
EffectShape.setSize({1280, 720});
EffectShape.setPosition({0, 0});
EffectShape.setFillColor(sf::Color::Transparent);
EffectManager = CreateWidget<CVrdxEffectManager>(SharedThis, EffectShape);
EffectManager->BringToFront();  // 가장 위에 렌더링

// NovelScene::Draw() — 셰이크 오프셋 적용
void CVrdxNovelScene::Draw(sf::RenderWindow& Window) const
{
    sf::View OriginalView = Window.getView();
    if (EffectManager->GetShakeOffset() != sf::Vector2f{0, 0})
    {
        sf::View ShakeView = OriginalView;
        ShakeView.move(EffectManager->GetShakeOffset());
        Window.setView(ShakeView);
    }

    CVrdxWidgetBase::Draw(Window);  // 자식들 Draw (EffectManager 포함)

    Window.setView(OriginalView);  // 복원
}

// NovelScene::OnKeyboardPressed — 디버그 키 (선택)
case sf::Keyboard::Scan::F: EffectManager->FadeOut(1.0f); return true;
```

---

## 6. ScriptEngine 연동

### 신규 스크립트 명령어

| 명령어 | 파라미터 | 설명 |
|--------|----------|------|
| `@fadeout` | `"seconds"` | 화면이 서서히 검정이 됨 (페이드아웃) |
| `@fadein` | `"seconds"` | 검정 화면에서 서서히 원래 화면으로 (페이드인) |
| `@shake` | `"seconds"` `"intensity"` | 화면 흔들림 (intensity: 픽셀 단위) |
| `@coloroverlay` | `"R"` `"G"` `"B"` `"A"` `"fadeIn"` `"hold"` `"fadeOut"` | 컬러 오버레이 |

### ScriptLine 처리

```cpp
// ScriptLine.h — 새로운 명령어 파생 클래스 추가
class FVrdxFadeOutScriptLine : public FVrdxScriptLine { /* ... */ };
class FVrdxFadeInScriptLine : public FVrdxScriptLine { /* ... */ };
class FVrdxShakeScriptLine : public FVrdxScriptLine { /* ... */ };
class FVrdxColorOverlayScriptLine : public FVrdxScriptLine { /* ... */ };
```

### ScriptLine.cpp 팩토리 테이블

```cpp
// 팩토리 테이블에 추가
{ "fadeout",      MakeScriptLine<FVrdxFadeOutScriptLine> },
{ "fadein",       MakeScriptLine<FVrdxFadeInScriptLine> },
{ "shake",        MakeScriptLine<FVrdxShakeScriptLine> },
{ "coloroverlay", MakeScriptLine<FVrdxColorOverlayScriptLine> },
```

### Dispatch — NovelScene 위임

```cpp
void FVrdxFadeOutScriptLine::Dispatch(CVrdxNovelScene& Scene) const
{
    Scene.GetEffectManager()->FadeOut(Duration);
}
```

---

## 7. 변경되는 파일

### 신규 파일

| 파일 | 설명 |
|------|------|
| `Novel/EffectManager.h` | `CVrdxEffectManager` 선언 |
| `Novel/EffectManager.cpp` | `CVrdxEffectManager` 구현 |
| `Docs/9-EffectManager.md` | 본 명세서 |

### 수정 파일

| 파일 | 변경 내용 |
|------|-----------|
| `Novel/NovelScene.h` | `EffectManager` 멤버 추가, `GetEffectManager()`, `Draw()` 오버라이드 |
| `Novel/NovelScene.cpp` | `OnPostCreate()`에서 EffectManager 생성, `Draw()`에 셰이크 오프셋 적용 |
| `Novel/ScriptLine.h` | 신규 명령어 파생 클래스 4개 추가 |
| `Novel/ScriptLine.cpp` | 팩토리 테이블에 4개 명령어 등록, `Dispatch()` 구현 |
| `Novel/NovelScene.h/cpp` | EffectManager 참조를 통한 명령어 Dispatch 처리 |
| `Game/Game.vcxproj` | 신규 파일 등록 |
| `Game/Game.vcxproj.filters` | Novel 필터에 EffectManager 추가 |

---

## 8. 우선순위 (구현 순서)

| 순서 | 작업 | 비고 |
|------|------|------|
| 1 | `EffectManager.h/cpp` — 클래스 골격, Update/Draw | WidgetBase 상속, NovelScene 자식으로 생성 |
| 2 | `FadeIn` / `FadeOut` 구현 | 화면 전체 sf::RectangleShape 알파 블렌딩 |
| 3 | `Shake` 구현 | NovelScene::Draw에서 View offset 협력 필요 |
| 4 | `ColorOverlay` 구현 | 다단계 (FadeIn → Hold → FadeOut) |
| 5 | 큐 시스템 (`EnqueueEffect` / `ProcessQueue`) | 순차 실행 |
| 6 | `@fadein` / `@fadeout` ScriptLine | ScriptEngine 연동 |
| 7 | `@shake` / `@coloroverlay` ScriptLine | ScriptEngine 연동 |
| 8 | 빌드/테스트 | 스크립트 기반 효과 재생 검증 |

---

## 9. 고려사항 및 주의점

- **Background의 페이드 전환과 혼동 주의**: `CVrdxBackground::SetBackground()`는 배경 이미지 교체용 자체 페이드를 가지고 있음. 화면 전체 페이드와는 별개로 동작하며, 두 효과가 동시에 실행될 수 있음.
- **셰이크와 페이드는 독립적으로 동작**: 셰이크 효과가 활성화된 상태에서도 페이드/컬러 오버레이는 별도로 실행 가능.
- **View 복원**: `NovelScene::Draw()`에서 `setView()`로 셰이크를 적용한 후 반드시 원래 View로 복원해야 함. 복원하지 않으면 다른 위젯(타이틀 등)의 렌더링에 영향을 줌.
- **셰이크 시드**: `std::default_random_engine`을 멤버로 유지하여 매 프레임 일관된 난수 생성.
- **성능**: 오버레이 사각형 하나만 Draw하면 되므로 성능 부담 미미.
- **큐 소진 시 자동 정리**: 큐가 비어 있고 활성 효과도 없으면 오버레이 알파는 0(투명)으로 유지되어 Draw call을 생략할 수 있음.

---

## 10. 향후 확장

- `@flash` — 화면 섬광 (흰색 오버레이 순간 표시)
- `@vignette` — 가장자리 어둡게
- `@blur` — 화면 흐림 (별도 셰이더 필요)
- 효과 프리셋 시스템 (`"earthquake"` = Shake + FadeOut 조합)
