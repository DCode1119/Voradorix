# 8단계 — 메뉴 구성 (Title → Save/Load)

## 목표

- **TitleWindow**: 게임 실행 시 첫 화면, New Game / Continue / Quit 제공
- **SaveLoadWindow**: 세이브 슬롯 목록을 표시하는 오버레이, 저장/불러오기 모드 지원
- **화면 전환**: Widget Tree의 `BringToFront()` / `DestroyWidget()`로 Title ↔ NovelScene 전환
- 구조 변경(Widget Tree 전환)에서 확립한 아키텍처를 유지하며 메뉴 시스템을 추가

---

## 1. 위젯 트리 구조

```
Application (CVrdxApplication — Root)
├── TitleWindow (CVrdxTitleWindow)    ← 최초 생성, Z=0
├── NovelScene (CVrdxNovelScene)      ← New Game 시 생성, Z=1
└── SaveLoadWindow (CVrdxSaveLoadWindow)  ← 오버레이, Z=2
```

- **TitleWindow**: Application 생성 직후 자동 생성.
- **NovelScene**: New Game 선택 시 생성, TitleWindow 위에 표시.
- **SaveLoadWindow**: Continue/NovelScene 내 Ctrl+S+L 키로 오버레이 표시, 가장 위에 표시.
- TitleWindow는 파괴되지 않고 Background에 남아 있으며, 추후 "타이틀로 돌아가기" 시 `BringToFront()`.

---

## 2. TitleWindow

### 파일 위치
- `Game/Src/Novel/TitleWindow.h`
- `Game/Src/Novel/TitleWindow.cpp`

### 클래스 시그니처

```cpp
class CVrdxTitleWindow : public CVrdxWidgetBase
{
public:
    explicit CVrdxTitleWindow(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget);
    ~CVrdxTitleWindow() override;

protected:
    bool OnCreate(TVrdxWeakPtr<CVrdxWidgetBase> self, TVrdxWeakPtr<CVrdxWidgetBase> Parent) override;
    bool OnDestroy() override;
    bool OnDraw() override;
    bool OnKeyboardPressed(const sf::Event::KeyPressed& Key) override;

private:
    void OnNewGame();
    void OnContinue();
    void OnQuit();

    // 자식 위젯들 (Button 등)
    TVrdxSharedPtr<CVrdxButton> pNewGameButton;
    TVrdxSharedPtr<CVrdxButton> pContinueButton;
    TVrdxSharedPtr<CVrdxButton> pQuitButton;

    sf::Sprite Background;
};
```

### 동작

| 액션 | 처리 |
|-------|--------|
| **New Game** | NovelScene이 없으면 `MakeVrdxShared<CVrdxNovelScene>(GetThisPtr(), ...)` 후 `BringToFront()`. 있으면 리셋 후 `BringToFront()`. |
| **Continue** | `MakeVrdxShared<CVrdxSaveLoadWindow>(GetThisPtr(), ...)` → `SetMode(false)` — 로드 모드 오버레이 |
| **Quit** | `CVrdxApplication::Quit()` 또는 `bIsRunning = false` |
| **ESC** | (타이틀에서는 무시 또는 종료 확인) |

### 렌더링 순서

1. 배경 스프라이트 그리기
2. 타이틀 텍스트/로고 (추후)
3. 버튼들 그리기 (자식 위젯의 OnDraw가 자동 호출됨)

---

## 3. SaveLoadWindow

### 파일 위치
- `Game/Src/Novel/SaveLoadWindow.h`
- `Game/Src/Novel/SaveLoadWindow.cpp`

### SaveSlotInfo 데이터 구조

```cpp
struct FSaveSlotInfo
{
    int Index;                     // 0 ~ 9
    bool bExists = false;          // 파일 존재 여부
    std::filesystem::path FilePath;
    std::string ModifiedTime;      // "2026-07-06 14:30:00" 형식
    std::string PreviewText;       // 챕터명 또는 첫 줄 (JSON에서 추출)
};
```

### 클래스 시그니처

```cpp
class CVrdxSaveLoadWindow : public CVrdxWidgetBase
{
public:
    explicit CVrdxSaveLoadWindow(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget);
    ~CVrdxSaveLoadWindow() override;

    void SetMode(bool bSaveMode);  // true = 저장, false = 불러오기

protected:
    bool OnCreate(TVrdxWeakPtr<CVrdxWidgetBase> self, TVrdxWeakPtr<CVrdxWidgetBase> Parent) override;
    bool OnDestroy() override;
    bool OnDraw() override;
    bool OnKeyboardPressed(const sf::Event::KeyPressed& Key) override;

private:
    void OnSlotSelected(int Index);
    void OnCancel();
    void RefreshSlotList();
    FSaveSlotInfo ScanSlot(int Index);

    bool bSaveMode = true;
    TVrdxVector<FSaveSlotInfo> SlotInfos;
    // 슬롯 표시용 자식 위젯들
    TVrdxVector<TVrdxSharedPtr<CVrdxButton>> SlotButtons;
};
```

### 동작

| 항목 | 처리 |
|------|--------|
| **생성 시** | `RefreshSlotList()` 호출, Saves/ 디렉토리 스캔 |
| **슬롯 선택** (저장 모드) | `NovelScene::Save("Saves/slot" + Index + ".vrdx")` |
| **슬롯 선택** (불러오기 모드) | `NovelScene::Load("Saves/slot" + Index + ".vrdx")` |
| **취소/닫기** | `DestroyWidget()` — 자신을 제거 |
| **ESC** | `DestroyWidget()` |

### 오버레이 렌더링

1. 반투명 검정 배경 (전체 화면)
2. 상단: 제목 ("저장하기" / "불러오기")
3. 중앙: 10개 슬롯 리스트 (각 슬롯: 인덱스, 날짜, 프리뷰)
4. 하단: 취소 버튼

---

## 4. 화면 전환 흐름

```
Application 생성
  └→ TitleWindow 생성 (OnCreate)
        │
        ├─ [New Game] ─→ NovelScene 생성 → BringToFront()
        │                    │
        │                    ├─ [ESC] ─→ TitleWindow BringToFront()
        │                    │           (선택: ESC 한 번 더 = 종료 확인)
        │                    │
        │                    ├─ [Ctrl+S] ─→ SaveLoadWindow 생성 (저장 모드)
        │                    │                └─ [취소/ESC] → DestroyWidget()
        │                    │
        │                    └─ [Ctrl+L] ─→ SaveLoadWindow 생성 (불러오기 모드)
        │                                     └─ [취소/ESC] → DestroyWidget()
        │
        ├─ [Continue] ─→ SaveLoadWindow 생성 (불러오기 모드)
        │                  ├─ [슬롯 선택] → Load → DestroyWidget() + NovelScene BringToFront()
        │                  └─ [취소/ESC] → DestroyWidget()
        │
        └─ [Quit] ─→ 종료
```

### NovelScene에서 타이틀로 복귀

- ESC 키: TitleWindow가 존재하면 `BringToFront()`, 없으면 종료 확인
- 복귀 시 NovelScene은 `DestroyWidget()` 하지 않고 그대로 둠 (재진입 시 빠름)
- 필요하다면 NovelScene에 `bActive` 플래그로 Draw/Update 생략 가능

---

## 5. Application 변경 사항

### CVrdxApplication

```cpp
class CVrdxApplication : public CVrdxWidgetBase
{
    // 기존 멤버 유지
    // 추가/변경:
    void Quit();   // bIsRunning = false

    // TitleWindow 생성 지원 — Initialize 콜백 내에서 처리
};
```

**변경 포인트:**
1. `Initialize` 콜백 내에서 `MakeVrdxShared<CVrdxTitleWindow>(...)` 호출
2. `Quit()` 메서드 추가 (버튼 콜백 등에서 접근 가능하도록)
3. 기타 변경 없음 (Widget Tree가 이벤트/드로우 자동 전파)

### Main.cpp

```cpp
MakeVrdxShared<CVrdxApplication>(nullptr)
    ->Initialize([](TVrdxWeakPtr<CVrdxWidgetBase> Parent)
    {
        auto App = Parent.lock();
        // TitleWindow 자동 생성
        MakeVrdxShared<CVrdxTitleWindow>(App);
    })
    ->Run();
```

---

## 6. NovelScene 변경 사항

### 추가할 기능

```cpp
class CVrdxNovelScene : public CVrdxWidgetBase
{
    // 기존 유지
    // 추가:
    void Reset();   // 진행 상태 초기화 (New Game 재시작용)

protected:
    bool OnKeyboardPressed(const sf::Event::KeyPressed& Key) override
    {
        // 기존 Ctrl+S / Ctrl+L 유지
        // ESC 추가: TitleWindow로 복귀
        if (Key.scancode == sf::Keyboard::Scan::Escape)
        {
            // 부모(Application) 아래 TitleWindow 찾기 → BringToFront()
            return true;
        }
        return false;
    }
};
```

**변경 포인트:**
1. `Reset()` — 스크립트 상태, 캐릭터, 배경 초기화 (New Game 재시작용)
2. `OnKeyboardPressed`에 ESC 처리 추가
3. SaveLoadWindow와 협력: Save/Load는 기존 메서드 그대로 사용

---

## 7. 파일 목록

### 신규 파일
| 파일 | 설명 |
|------|------|
| `Game/Src/Novel/TitleWindow.h` | TitleWindow 선언 |
| `Game/Src/Novel/TitleWindow.cpp` | TitleWindow 구현 |
| `Game/Src/Novel/SaveLoadWindow.h` | SaveLoadWindow 선언 |
| `Game/Src/Novel/SaveLoadWindow.cpp` | SaveLoadWindow 구현 |
| `Docs/8-Menu.md` | 본 명세서 |

### 수정 파일
| 파일 | 변경 내용 |
|------|-----------|
| `Game/Src/Novel/NovelScene.h` | `Reset()`, ESC 처리 추가 |
| `Game/Src/Novel/NovelScene.cpp` | `Reset()` 구현, `OnKeyboardPressed` ESC 추가 |
| `Game/Src/Ui/Application.h` | `Quit()` 메서드 추가 |
| `Game/Src/Ui/Application.cpp` | `Quit()` 구현 |
| `Game/Src/Main.cpp` | TitleWindow 생성 코드 추가 |
| `Game/Game.vcxproj` | 신규 4개 파일 포함 |
| `Game/Game.vcxproj.filters` | Novel 필터에 신규 파일 추가 |

---

## 8. 백로그 반영

기존 BACKLOG.md의 항목:

```
9단계 — 메뉴 구성 (TitleScene / SaveLoadScene) ← @8단계로 번호 조정
```

을 다음으로 갱신:

```
- [ ] 8단계 — 메뉴 구성
  - [ ] TitleWindow — New Game / Continue / Quit
  - [ ] SaveLoadWindow — 오버레이, 저장/불러오기 모드
  - [ ] ESC → TitleWindow 복귀
```

---

## 9. 참고 사항

- 모든 신규 위젯은 `CVrdxWidgetBase` 상속, 기존 Widget Tree 규칙 준수
- 이벤트 훅은 `bool` 반환 (true = 소비, false = 통과)
- `shared_from_this()` 사용 시 `std::static_pointer_cast`/`std::dynamic_pointer_cast` 필요
- 세이브 파일 스캔은 `std::filesystem::directory_iterator("Saves/")` 사용
- 슬롯 10개 고정, 파일명 `slot0.vrdx` ~ `slot9.vrdx`
- SaveLoadWindow는 오버레이로서만 동작, NovelScene/TitleWindow와 독립적
- TitleWindow의 버튼 배치는 간단한 하드코딩 좌표 (추후 리소스 매니저에서 조정)
- SaveLoadWindow의 슬롯 리스트는 스크롤 없이 10개 고정 표시

---

## 진행 상태

- [ ] TitleWindow 구현
  - [ ] 헤더/소스 생성, 클래스 작성
  - [ ] OnCreate: 배경 로드, 버튼 생성
  - [ ] OnNewGame: NovelScene 생성/리셋
  - [ ] OnContinue: SaveLoadWindow 오버레이
  - [ ] OnQuit: 종료
- [ ] SaveLoadWindow 구현
  - [ ] 헤더/소스 생성, 클래스 작성
  - [ ] FSaveSlotInfo 구조체
  - [ ] RefreshSlotList: 파일 스캔
  - [ ] OnSlotSelected: Save/Load 위임
  - [ ] OnCancel: DestroyWidget
- [ ] NovelScene 변경
  - [ ] Reset() 추가
  - [ ] ESC → TitleWindow 복귀
- [ ] Application 변경
  - [ ] Quit() 추가
- [ ] Main.cpp 변경
  - [ ] TitleWindow 생성 코드 추가
- [ ] vcxproj 갱신
- [ ] 빌드/테스트 확인
