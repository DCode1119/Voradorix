# 8단계 — 메뉴 구성 (Title → Save/Load)

## 목표

- **TitleWindow**: 게임 실행 시 첫 화면, New Game / Continue 제공
- **SaveLoadWindow**: 세이브 슬롯 목록을 표시하는 오버레이, 저장/불러오기 모드 지원
- **화면 전환**: Widget Tree의 `BringToFront()`로 Title ↔ SaveLoadWindow ↔ NovelScene 전환
- 구조 변경(Widget Tree 전환)에서 확립한 아키텍처를 유지하며 메뉴 시스템을 추가

---

## 1. 위젯 트리 구조

```
Application (CVrdxApplication — Root)
├── TitleWindow (CVrdxTitleWindow)        ← 최초 생성, Z=0
├── NovelScene (CVrdxNovelScene)          ← New Game 시 전면, Z=1
└── SaveLoadWindow (CVrdxSaveLoadWindow)  ← Continue/오버레이, Z=2
```

- **TitleWindow**: Application 생성 시 함께 생성.
- **NovelScene**: New Game 선택 시 전면으로.
- **SaveLoadWindow**: Continue 선택 시 불러오기 모드로 표시. 추후 NovelScene 내 Ctrl+S/L로도 호출.
- TitleWindow는 파괴되지 않고 Background에 남아 있으며, `BringToFront()`로 재표시.

---

## 2. TitleWindow

### 파일 위치
- `Game/Src/Novel/TitleWindow.h`
- `Game/Src/Novel/TitleWindow.cpp`

### 클래스 시그니처 (실제)

```cpp
class CVrdxTitleWindow : public CVrdxBoxWidget
{
public:
    CVrdxTitleWindow(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

    virtual void OnPostCreate() override;
    virtual void OnPreDestroy() override;

    void OnNewGame();
    void OnContinueGame();

    TVrdxMulticastDelegate<>& GetRequestNewGame();
    TVrdxMulticastDelegate<>& GetRequestContinueGame();

private:
    TVrdxSharedPtr<CVrdxTextLabel> TitleLabel;
    TVrdxSharedPtr<CVrdxButton> NewGameButton;
    TVrdxSharedPtr<CVrdxButton> LoadGameButton;

    TVrdxMulticastDelegate<> RequestNewGame;
    TVrdxMulticastDelegate<> RequestContinueGame;
};
```

### 동작 (실제)

| 액션 | 처리 |
|------|------|
| **New Game** | `RequestNewGame.Broadcast()` → Main.cpp 람다: `NovelWindow->BringToFront()` |
| **Continue** | `RequestContinueGame.Broadcast()` → Main.cpp 람다: `SaveLoadWindow->ShowSaveLoadWindow(false)` |
| **Quit** | 아직 미구현 (버튼 없음) |

### 렌더링 순서

1. `CVrdxBoxWidget::Draw()` — 박스 배경
2. 자식 위젯들 (TitleLabel, NewGameButton, LoadGameButton) 자동 렌더링

---

## 3. SaveLoadWindow

### 파일 위치
- `Game/Src/Novel/SaveLoadWindow.h`
- `Game/Src/Novel/SaveLoadWindow.cpp`

### 클래스 시그니처 (실제)

```cpp
class CVrdxSaveLoadWindow : public CVrdxBoxWidget
{
public:
    CVrdxSaveLoadWindow(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

    virtual void OnPostCreate() override;
    virtual void OnPreDestroy() override;

    void ShowSaveLoadWindow(bool bSave);
    bool IsSaveMode() const;

    TVrdxMulticastDelegate<int32_t>& GetRequestSaveGame();
    TVrdxMulticastDelegate<int32_t>& GetRequestLoadGame();
    TVrdxMulticastDelegate<>& GetRequestBackToMain();

private:
    TVrdxSharedPtr<CVrdxTextLabel> TitleLabel;
    TVrdxSharedPtr<CVrdxButton> BackToMain;
    TVrdxVector<TVrdxSharedPtr<CVrdxButton>> ButtonSlots;

    TVrdxMulticastDelegate<> RequestBackToMain;
    TVrdxMulticastDelegate<int32_t> RequestSaveGame;
    TVrdxMulticastDelegate<int32_t> RequestLoadGame;

    bool bSaveMode = false;
};
```

### 동작 (실제)

| 항목 | 처리 |
|------|------|
| **생성 시** | TitleLabel + BackToMain 버튼 + 10개 슬롯 버튼 생성 |
| **ShowSaveLoadWindow(true)** | 저장 모드, 슬롯 텍스트 "Save Slot 1..10" |
| **ShowSaveLoadWindow(false)** | 불러오기 모드, 슬롯 텍스트 "Load Slot 1..10" |
| **슬롯 클릭** (저장 모드) | `RequestSaveGame.Broadcast(Index)` → Main.cpp 람다 연결 필요 |
| **슬롯 클릭** (불러오기 모드) | `RequestLoadGame.Broadcast(Index)` → Main.cpp: `NovelWindow->Load(...)` + `BringToFront()` |
| **BackToMain 클릭** | `RequestBackToMain.Broadcast()` → Main.cpp: `TitleWindow->BringToFront()` |

---

## 4. 화면 전환 흐름 (실제)

```
Application 생성
  └→ TitleWindow + NovelScene + SaveLoadWindow 모두 생성 (초기 Z=0)
        │
        ├─ [New Game] ─→ NovelWindow->BringToFront()
        │
        ├─ [Continue] ─→ SaveLoadWindow->ShowSaveLoadWindow(false)
        │                  └─ [슬롯 선택] → NovelWindow->Load(...) + BringToFront()
        │
        └─ SaveLoadWindow
             └─ [Back] ─→ TitleWindow->BringToFront()
```

**특징:**
- 모든 위젯은 Application 생성 시 함께 생성됨
- 전환은 오직 `BringToFront()`로만 이루어짐 (파괴/생성 없음)
- NovelScene의 `ResetScriptEngine()`으로 New Game 시 스크립트 재시작 지원
- save 파일 존재 여부는 `std::filesystem::exists()`로 체크

---

## 5. NovelScene 변경 사항

### 추가된 기능

```cpp
class CVrdxNovelScene : public CVrdxWidgetBase
{
    // 기존 유지
    // 추가:
    void ResetScriptEngine();   // ScriptEngine 재로드 (New Game 재시작용)

    // OnKeyboardPressed에 Ctrl+S / Ctrl+L 유지
};
```

**변경 포인트:**
1. `ResetScriptEngine()` — `ScriptEngine.LoadScript()` 재호출로 스크립트를 처음부터 재시작
2. Save/Load 경로 버그 수정: `"Saves" + Filename` → `"Saves/" + Filename`
3. Ctrl+S/L 단축키 유지 (Save0.dat 고정 파일명)

---

## 6. Main.cpp 변경 사항 (실제)

```cpp
CVrdxApplication::VRDX_Initializer Initializer = [](TVrdxWeakPtr<CVrdxWidgetBase>& RootWidget)
{
    auto NovelWindow = CreateWidget<CVrdxNovelScene>(RootWidget, Shape);
    auto TitleWindow = CreateWidget<CVrdxTitleWindow>(RootWidget, Shape);
    auto SaveLoadWindow = CreateWidget<CVrdxSaveLoadWindow>(RootWidget, Shape);

    TitleWindow->GetRequestNewGame().Add([NovelWindow]() { NovelWindow->BringToFront(); });
    TitleWindow->GetRequestContinueGame().Add([SaveLoadWindow]() { SaveLoadWindow->ShowSaveLoadWindow(false); });

    SaveLoadWindow->GetRequestBackToMain().Add([TitleWindow]() { TitleWindow->BringToFront(); });
    SaveLoadWindow->GetRequestLoadGame().Add([NovelWindow](int32_t SlotIndex)
    {
        const std::string Filename = "Save" + std::to_string(SlotIndex) + ".dat";
        if (std::filesystem::exists("Saves/" + Filename))
        {
            NovelWindow->Load(Filename);
            NovelWindow->BringToFront();
        }
    });

    TitleWindow->BringToFront();  // 초기 화면
};
```

---

## 7. 파일 목록

### 신규 파일
| 파일 | 설명 |
|------|------|
| `Game/Src/Novel/TitleWindow.h` | TitleWindow 선언 |
| `Game/Src/Novel/TitleWindow.cpp` | TitleWindow 구현 |
| `Game/Src/Novel/SaveLoadWindow.h` | SaveLoadWindow 선언 |
| `Game/Src/Novel/SaveLoadWindow.cpp` | SaveLoadWindow 구현 |

### 수정 파일
| 파일 | 변경 내용 |
|------|-----------|
| `Game/Src/Novel/NovelScene.h` | `ResetScriptEngine()` 추가 |
| `Game/Src/Novel/NovelScene.cpp` | `ResetScriptEngine()` 구현, Save/Load 경로 버그 수정 |
| `Game/Src/Ui/TextLabel.cpp` | `OnResized` 위치 기준 통일 (`{0,0}`) |
| `Game/Src/Main.cpp` | TitleWindow, SaveLoadWindow 생성 및 델리게이트 연결 |
| `Game/Game.vcxproj` | 신규 2개 파일 포함 |
| `Game/Game.vcxproj.filters` | Novel 필터에 신규 파일 추가 |

---

## 8. 백로그 반영

BACKLOG.md 항목:

```
## 8단계 — Menu

- [ ] TitleWindow — New Game / Continue / Quit
- [ ] SaveLoadWindow — 오버레이, 저장/불러오기 모드
- [ ] ConfigScene — (추후)
```

을 다음으로 갱신:

```
## 8단계 — Menu

- [x] TitleWindow — New Game / Continue (Quit는 미구현)
- [x] SaveLoadWindow — 오버레이, 저장/불러오기 모드
- [ ] ConfigScene — (추후)
- [ ] ESC → TitleWindow 복귀 (미구현)
- [ ] Quit 버튼 (미구현)
```

---

## 9. 참고 사항

- 모든 신규 위젯은 `CVrdxBoxWidget` 상속 (WidgetBase → BoxWidget 체인)
- 이벤트 훅은 `bool` 반환 (true = 소비, false = 통과)
- `shared_from_this()` → `dynamic_pointer_cast` + `weak_ptr::lock()` 패턴 사용
- 화면 전환은 `BringToFront()`만으로 처리 (위젯 파괴/생성 없음)
- 슬롯 10개 고정, 세이브 파일명 `Save{Index}.dat`
- SaveLoadWindow는 오버레이로서만 동작, `ShowSaveLoadWindow()`로 모드 전환
- TitleWindow의 버튼 배치는 하드코딩 좌표
- 슬롯 리스트는 스크롤 없이 10개 고정 표시
- 백로그, Quit 버튼, ESC 복귀 등은 차후 단계에서 구현 예정

---

## 진행 상태

- [x] TitleWindow 구현
  - [x] 헤더/소스 생성, 클래스 작성
  - [x] OnPostCreate: TitleLabel, NewGameButton, LoadGameButton 생성
  - [x] OnNewGame: RequestNewGame.Broadcast()
  - [x] OnContinueGame: RequestContinueGame.Broadcast()
  - [ ] OnQuit: 종료 (미구현)
- [x] SaveLoadWindow 구현
  - [x] 헤더/소스 생성, 클래스 작성
  - [x] OnPostCreate: TitleLabel, BackToMain, 10개 슬롯 버튼 생성
  - [x] ShowSaveLoadWindow: 모드 전환 + 슬롯 텍스트 변경
  - [ ] FSaveSlotInfo 구조체 (미구현, 단순 버튼 리스트)
  - [ ] RefreshSlotList: 파일 스캔 (미구현)
- [x] NovelScene 변경
  - [x] ResetScriptEngine() 추가
  - [x] Save/Load 경로 버그 수정
  - [ ] ESC → TitleWindow 복귀 (미구현)
- [x] Main.cpp 변경
  - [x] TitleWindow, SaveLoadWindow 생성 및 델리게이트 연결
- [x] vcxproj / filters 갱신
- [x] 빌드/테스트 확인
