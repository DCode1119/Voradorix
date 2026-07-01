---
title: "1단계: Scene 시스템"
project: Voradorix
phase: 1
status: 완료
tags:
  - voradorix
  - scene-system
  - spec
  - cpp
---
# Voradorix — 1단계: Scene 시스템 구현 명세

> **대상**: Scene 추상 인터페이스 + SceneManager + Application 리팩터  
> **프로젝트**: Voradorix  
> **관련 파일**: `Src/Core/Application.h/cpp`, `Src/Core/Common.h`, `Src/Scene/Scene.h`, `Src/Scene/SceneManager.h/cpp`, `Src/Scene/TestScene.h/cpp`, `Src/Main.cpp`  
> **의존성**: SFML 3.1.0 (`sf::RenderWindow`, `sf::Event`, `sf::Clock`)  
> **완료 조건**: Scene 기반으로 원을 그리는 테스트 씬이 창에 표시됨

---

## 1. 파일 구성

| # | 파일 | 설명 | 상태 |
|---|------|------|------|
| 1 | `Src/Core/Common.h` | 공용 타입 alias + 매크로 | ✅ 작성됨 |
| 2 | `Src/Scene/Scene.h` | Scene 추상 인터페이스 (WantsExit/RequestExit 포함) | ✅ 작성됨 |
| 3 | `Src/Scene/Scene.cpp` | (Scene.h include) | ✅ 작성됨 |
| 4 | `Src/Scene/SceneManager.h` | SceneManager 선언 | ✅ 작성됨 |
| 5 | `Src/Scene/SceneManager.cpp` | SceneManager 구현 | ✅ 작성됨 |
| 6 | `Src/Core/Application.h` | Application 클래스 선언 | ✅ 작성됨 |
| 7 | `Src/Core/Application.cpp` | Application 구현 (메인 루프) | ✅ 작성됨 |
| 8 | `Src/Scene/TestScene.h` | 검증용 테스트 씬 선언 | ✅ 작성됨 |
| 9 | `Src/Scene/TestScene.cpp` | 검증용 테스트 씬 구현 | ✅ 작성됨 |
| 10 | `Src/Main.cpp` | 진입점 (Application 생성/실행) | ✅ 리팩터 완료 |
| 11 | `Game.vcxproj` | ClCompile/ClInclude 항목 등록 | ✅ 등록 완료 |
| 12 | `Game.vcxproj.filters` | 필터 구성 | ✅ 구성됨 |

---

## 2. 네이밍 규칙

Voradorix 프로젝트 네이밍 규칙을 따른다 (자세한 내용은 `Docs/NAMING.md` 참조).

| 구분 | 규칙 | 예시 |
|------|------|------|
| 클래스 접두어 | `C` + `Vrdx` | `CVrdxScene`, `CVrdxSceneManager` |
| 템플릿 접두어 | `T` + `Vrdx` | `TVrdxUniquePtr<T>`, `TVrdxVector<T>` |
| 인터페이스 접두어 | `I` + `Vrdx` | `IVrdxSerializable` |
| 구조체 접두어 | `F` + `Vrdx` | `FVrdxSaveSlot` |
| 열거형 접두어 | `E` + `Vrdx` | `EVrdxSceneResult` |
| bool 접두어 | `b` | `bIsRunning`, `bEnabled` |
| 변수 | PascalCase, `m_` 없음 | `SceneStack`, `DeltaTime` |
| 메서드 | PascalCase | `OnEnter()`, `GetCount()` |
| 파일명 | PascalCase + 접두어 | `VrdxScene.h`, `VrdxSceneManager.cpp` |

매크로는 `Common.h`에 정의되어 있다.

---

## 3. Scene 추상 인터페이스 — `VrdxScene.h`

### 3.1 역할

모든 씬이 구현해야 하는 순수 가상 인터페이스. SceneManager는 `CVrdxScene` 포인터만으로 모든 씬을 균일하게 다룬다.

### 3.2 메서드 시그니처

```cpp
#pragma once
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"

#include "../Core/Common.h"

class CVrdxScene
{
public:
    virtual ~CVrdxScene() VRDX_DEFAULT;

    virtual void OnEnter() VRDX_PURE_VIRTUAL;
    virtual void OnExit() VRDX_PURE_VIRTUAL;
    virtual void HandleEvent(const sf::Event&) VRDX_PURE_VIRTUAL;
    virtual void Update(const float DeltaTick) VRDX_PURE_VIRTUAL;
    virtual void Draw(sf::RenderWindow&) VRDX_PURE_VIRTUAL;

    bool WantsExit() const { return bWantExit; }

protected:
    void RequestExit() { bWantExit = true; }

private:
    bool bWantExit = false;
};
```

### 3.3 메서드별 설명

| 메서드 | 호출 시점 | 책임 |
|--------|-----------|------|
| `OnEnter()` | Push/Switch 직후, 첫 Update/HandleEvent 이전에 1회 | 초기화, 리소스 로딩, BGM 재생 |
| `OnExit()` | Pop/Switch 직전에 1회 | 정리, 리소스 해제, BGM 정지 |
| `HandleEvent(const sf::Event&)` | 매 프레임, 이벤트 큐 소비 시 | 키보드/마우스 입력 처리 |
| `Update(const float DeltaTick)` | 매 프레임, delta-time (초) | 상태 업데이트, 애니메이션, 타이머 |
| `Draw(sf::RenderWindow&)` | 매 프레임, Update 이후 | 모든 렌더링 호출 |
| `WantsExit()` | SceneManager가 Pop 전에 호출 | 씬이 자발적 종료 요청했는지 확인 |
| `RequestExit()` | Scene 내부에서 호출 (protected) | bWantExit 플래그 설정 → WantsExit()가 true 반환 |

---

## 4. SceneManager — `SceneManager.h/cpp`

### 4.1 역할

`CVrdxScene` 포인터를 `TVrdxVector<TVrdxUniquePtr<CVrdxScene>>` 스택으로 관리한다.
최상위 씬만 활성화되고, 아래 씬은 일시 정지된다.

### 4.2 클래스 설계

```cpp
class CVrdxSceneManager
{
public:
    CVrdxSceneManager() VRDX_DEFAULT;
    ~CVrdxSceneManager() VRDX_DEFAULT;

    CVrdxSceneManager(const CVrdxSceneManager&) VRDX_NO_COPY;
    CVrdxSceneManager& operator=(const CVrdxSceneManager&) VRDX_NO_COPY;

    void Push(TVrdxUniquePtr<CVrdxScene> Scene);
    void Pop();
    void Switch(TVrdxUniquePtr<CVrdxScene> Scene);

    void HandleEvent(const sf::Event& Event);
    void Update(const float DeltaTick);
    void Draw(sf::RenderWindow& Window);

    VRDX_NO_DISCARD bool IsEmpty() const;

private:
    TVrdxVector<TVrdxUniquePtr<CVrdxScene>> SceneStack;
};
```

### 4.3 주요 구현 상세

#### `Push(TVrdxUniquePtr<CVrdxScene> Scene)`

1. `Scene->OnEnter()` — 새 씬 초기화 (스택 추가 전)
2. `SceneStack.Add(VrdxMove(Scene))` — 스택에 새 씬 추가
3. 이전 씬의 `OnExit()`은 호출하지 않음 (일시 정지)

#### `Pop()`

1. `SceneStack`이 비어있으면 아무 동작 안 함
2. `SceneStack.Last()->OnExit()` — 현재 씬 정리
3. `SceneStack.Pop()` — 현재 씬 제거
4. **이전 씬의 `OnEnter()`를 호출하지 않음** — 스택이 비면 Application의 Run 루프가 `IsEmpty()` 감지 → 종료

#### `Switch(TVrdxUniquePtr<CVrdxScene> Scene)`

1. `Pop()` 호출 (내부에서 OnExit + Pop)
2. `Push(VrdxMove(Scene))` 호출 (내부에서 OnEnter + Add)

#### `HandleEvent(const sf::Event& Event)`

- `SceneStack`이 비어있으면 무시
- `SceneStack.Last()->HandleEvent(Event)` — 최상위 씬에만 전달
- 호출 후 `SceneStack.Last()->WantsExit()`가 true면 `Pop()` 호출 (ESC 등으로 인한 자발적 종료)

#### `Update(const float DeltaTick)`

- `SceneStack`이 비어있으면 무시
- `SceneStack.Last()->Update(DeltaTick)` — 최상위 씬만 갱신

#### `Draw(sf::RenderWindow& Window)`

- `SceneStack`이 비어있으면 무시
- `SceneStack.Last()->Draw(Window)` — 최상위 씬만 렌더링

### 4.4 소유권 정책

- `TVrdxUniquePtr<CVrdxScene>` 사용 — 외부에서 소유권을 넘기면 SceneManager가 독점 관리
- SceneManager 소멸 시 모든 Scene이 자동 정리 (`TVrdxVector` 소멸자 → `unique_ptr` 소멸자 연쇄)
- 씬 외부에서 Scene 포인터를 보관하지 않음 (dangling 방지)

---

## 5. Application — `Application.h/cpp`

### 5.1 역할

기존 `main.cpp`에 있던 창 생성 + 이벤트 루프 + 렌더링을 캡슐화.
`CVrdxSceneManager`를 소유하고 매 프레임 구동한다.

### 5.2 클래스 설계

```cpp
class CApplication
{
public:
    CApplication();
    ~CApplication();

    void Run();

private:
    void HandleEvents();
    void Update(float DeltaTick);
    void Draw();

    sf::RenderWindow     Window;
    CVrdxSceneManager    SceneManager;
    sf::Clock            DeltaClock;
    bool                 bIsRunning;
};
```

### 5.3 생성자

1. `sf::RenderWindow` 생성
   - 해상도: 1280×720
   - 타이틀: `"Visual Novel Engine"`
   - VSync 활성화 (`setVerticalSyncEnabled(true)`)
   - 프레임 제한: 60fps (`setFramerateLimit(60)`)
2. `bIsRunning = true` 초기화
3. 최초 Scene Push: `SceneManager.Push(MakeVrdxUnique<CTestScene>())`

### 5.4 메인 루프

```cpp
while (bIsRunning && Window.isOpen())
{
    float DeltaTick = DeltaClock.restart().asSeconds();
    HandleEvents();
    Update(DeltaTick);
    Draw();
}
```

### 5.5 `HandleEvents()`

```cpp
while (const auto OptionalEvent = Window.pollEvent())
{
    const auto& Event = OptionalEvent.value();
    if (Event.is<sf::Event::Closed>())
    {
        bIsRunning = false;
    }
    SceneManager.HandleEvent(Event);
}
```

### 5.6 `Update(float DeltaTick)`

`SceneManager.Update(DeltaTick);`

### 5.7 `Draw()`

```cpp
Window.clear(sf::Color::Black);
SceneManager.Draw(Window);
Window.display();
```

### 5.8 종료 조건

- 창 닫힘 (`sf::Event::Closed`)
- `SceneManager` 스택이 비었을 때 → `bIsRunning = false`
  - `Pop()` 후 `IsEmpty()`가 true면 종료 플래그 설정
  - 혹은 `HandleEvents()`에서 ESC 키가 Pop을 호출하고 스택이 비면 종료

---

## 6. `main.cpp` — 진입점 최소화

```cpp
#include "Core/Application.h"

int main()
{
    CApplication App;
    App.Run();
    return 0;
}
```

기존의 윈도우 생성, 이벤트 루프, 원 그리기 코드는 모두 제거한다.

---

## 7. CVrdxTestScene — 1단계 검증용 임시 씬

**파일**: `Src/Scene/TestScene.h` / `Src/Scene/TestScene.cpp` (별도 파일)

클래스: `CVrdxTestScene` (`CVrdxScene` 상속)

| 메서드 | 구현 |
|--------|------|
| `OnEnter()` | (비어 있음) |
| `OnExit()` | (비어 있음) |
| `HandleEvent()` | ESC 키 → `RequestExit()` |
| `Update()` | (비어 있음) |
| `Draw()` | 화면 정중앙에 파란색 원(`sf::CircleShape`, 반지름 100, `setOrigin`으로 중앙 정렬) |

ESC 누르면 `RequestExit()` → SceneManager가 `WantsExit()` 감지 → `Pop()` → 스택이 비면 `bIsRunning = false` → Application 종료.

---

## 8. `TVrdxVector<T>` 사용 규칙

SceneManager에서 사용하는 메서드:

| 메서드 | 사용처 |
|--------|--------|
| `IsEmpty()` | Pop/Switch/HandleEvent/Update/Draw에서 가드 |
| `Last()` | Top 씬 접근 (참조 반환) |
| `Add(T&&)` | Push/Switch에서 씬 추가 |
| `Pop()` | Pop/Switch에서 씬 제거 |

- `Last()`는 `TVrdxUniquePtr<CVrdxScene>&`를 반환하므로, `.get()` 또는 `->`로 접근
- `TVrdxVector`가 비어있을 때 `Last()`를 호출하지 않도록 주의 (Pop 등에서 IsEmpty 검사 선행)

---

## 9. `.vcxproj` 변경 사항

### ClCompile 항목 (등록된 파일)

| 파일 | 상태 |
|------|------|
| `Src\Main.cpp` | ✅ 등록됨 |
| `Src\Core\Application.cpp` | ✅ 등록됨 |
| `Src\Scene\Scene.cpp` | ✅ 등록됨 |
| `Src\Scene\SceneManager.cpp` | ✅ 등록됨 |
| `Src\Scene\TestScene.cpp` | ✅ 등록됨 |

### ClInclude 항목 (등록된 파일)

| 파일 | 상태 |
|------|------|
| `Src\Core\Common.h` | ✅ 등록됨 |
| `Src\Core\Vector.h` | ✅ 등록됨 |
| `Src\Core\Application.h` | ✅ 등록됨 |
| `Src\Scene\Scene.h` | ✅ 등록됨 |
| `Src\Scene\SceneManager.h` | ✅ 등록됨 |
| `Src\Scene\TestScene.h` | ✅ 등록됨 |

---

## 10. 의존성 그래프

```
Main.cpp
  └── Application.h
        ├── SFML/Graphics/RenderWindow.hpp
        ├── SFML/Window/Event.hpp
        ├── Common.h
        └── SceneManager.h
              ├── Common.h
              ├── Vector.h
              └── Scene.h
                    └── Common.h
```

- 순환 의존성 없음
- 각 헤더는 필요한 SFML 헤더만 `#include`

---

## 11. 컴파일 / 링크 확인 사항

- `Application.cpp`에서 `sf::RenderWindow`, `sf::Event`, `sf::Clock` 사용
- `TestScene.cpp`에서 `sf::CircleShape`, `sf::Color` 사용
- 링크 대상 (Debug): `sfml-graphics-d.lib`, `sfml-window-d.lib`, `sfml-system-d.lib`
- PostBuild: `sfml-*-d-3.dll` 복사 (기존 설정 유지)

---

## 12. 검증 방법

1. **빌드**: Visual Studio 2022 x64 Debug, Warning Level 3, Clean Build 성공
2. **실행**: 1280×720 창이 열리고, 가운데 파란색 원이 표시됨
3. **종료**: ESC 키 입력 → Pop → 스택 empty → 자동 종료
4. **메모리**: 종료 시 `unique_ptr` 정리 — leak 없음 (VS Debug 출력 확인)

---

## 13. 다음 단계

2단계에서 `CNovelScene`이 `CVrdxTestScene`을 대체한다.

---

## 14. 구현 순서 (완료)

| 단계 | 작업 | 상태 |
|------|------|------|
| 1 | `TVrdxVector<T>` 클래스 구현 | ✅ 완료 |
| 2 | `SceneManager.cpp`에 Push/Pop/Switch/HandleEvent/Update/Draw 구현 | ✅ 완료 |
| 3 | `Application.h` + `Application.cpp` 작성 (`CVrdxTestScene` Push) | ✅ 완료 |
| 4 | `Main.cpp` 리팩터 | ✅ 완료 |
| 5 | `Game.vcxproj`에 모든 파일 등록 | ✅ 완료 |
| 6 | `Docs/` 문서 현행화 | ✅ 완료 |
