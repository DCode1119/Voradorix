# 8단계: Scene 시스템 구조 변경 (Widget Tree 기반 전환)

## 배경

기존 Scene/SceneManager/Push/Pop 방식은 Pop N번 계산 문제, Background/CharacterManager의 독립 Update/Draw 필요성,
Window 전환의 범용성 부족 등 구조적 한계가 있음.

WidgetBase 기반의 BringToFront + CreateWidget/DestroyWidget만으로 모든 전환을 통일하는 방향으로 전환.

## 핵심 아이디어

```
Application (CVrdxApplication : CVrdxWidgetBase)
  └── NovelScene (CVrdxNovelScene : CVrdxWidgetBase)
        ├── Background (CVrdxBackground : CVrdxWidgetBase)
        ├── CharacterManager (CVrdxCharacterManager : CVrdxWidgetBase)
        ├── DialogueBox (CVrdxDialogueBox : CVrdxWidgetBase)
        └── ChoiceWidget (CVrdxChoiceWidget : CVrdxWidgetBase)
```

- **Window 전환** = ActiveWindow->BringToFront() → 자신을 형제 중 최상단 Z-Order로 이동
- **오버레이** = 새 Widget 생성 → BringToFront()
- **닫기** = DestroyWidget(해당 Widget) → 자연스럽게 아래 Window가 보임
- **모든 Update/Draw** = WidgetBase 트리가 자식 순회하며 자동 전파
- **Save/Load** = Application이 Children 순회하며 NovelScene 탐색 후 직렬화
- Push/Pop 횟수 계산 없음

## 적용된 변경

### 파일 구조 (최종)

```
Src/
├─ Core/          Common.h, String.h/cpp, Vector.h          (유틸리티)
├─ Novel/         Background, CharacterManager, ChoiceWidget,
│                 DialogueBox, DialogueLine, NovelScene,
│                 ScriptEngine, ScriptLine                   (Novel 전용, 15개)
├─ Ui/            Application, BoxWidget, Button, TextLabel,
│                 WidgetBase                                 (공통 컴포넌트, 10개)
└─ Main.cpp
```

### 단계별 실제 변경

#### 1단계: WidgetBase 보강
**파일:** `Ui/WidgetBase.h`, `Ui/WidgetBase.cpp`

- `BringToFront()` 추가 — Children에서 자신을 제거 후末尾 재추가 (형제 중 최상단 Z)

#### 2단계: Background → CVrdxWidgetBase 상속
**파일:** `Novel/Background.h`, `Novel/Background.cpp`

- `class CVrdxBackground : public CVrdxWidgetBase` (이름 유지)
- `Update(float)` / `Draw(RenderWindow&)` override
- 기존 public 인터페이스(SetBackground, FadeIn/Out 등) 유지

#### 3단계: CharacterManager → CVrdxWidgetBase 상속
**파일:** `Novel/CharacterManager.h`, `Novel/CharacterManager.cpp`

- `class CVrdxCharacterManager : public CVrdxWidgetBase` (이름 유지)
- `Update(float)` / `Draw(RenderWindow&)` override
- 기존 public 인터페이스(SetCharacter, RemoveCharacter, Fade 등) 유지
- `GetSaveData()/Reset()` 유지

#### 4단계: NovelScene → CVrdxWidgetBase 상속 + 파일 이동
**파일:** `Scene/NovelScene.h/cpp` → `Novel/NovelScene.h/cpp`

- `class CVrdxNovelScene : public CVrdxScene` → `class CVrdxNovelScene : public CVrdxWidgetBase` (이름 유지)
- 생성자: `(TVrdxWeakPtr<CVrdxWidgetBase>, sf::RectangleShape)`
- `OnEnter()/OnExit()` → `OnPostCreate()/OnPreDestroy()`로 대체
- `OnPostCreate()`에서 자식 Widget 생성 + `BringToFront()`로 Z-Order 정렬
- Save() / Load() — NovelScene에 유지

#### 5단계: Scene/SceneManager 폐기
**삭제된 파일:**
- `Scene/Scene.h`, `Scene/Scene.cpp`
- `Scene/SceneManager.h`, `Scene/SceneManager.cpp`
- `Scene/TestScene.h`, `Scene/TestScene.cpp`
- `.vcxproj` / `.vcxproj.filters` 참조 정리 및 Scene 필터 제거

#### 6단계: Application → CVrdxWidgetBase 상속 + 파일 이동
**파일:** `Core/Application.h/cpp` → `Ui/Application.h/cpp`

- `class CVrdxApplication : public CVrdxWidgetBase`
- SceneManager 멤버 제거, 직접 Widget 트리의 Root 역할
- `Run()` 루프: `Update()` → `Draw()` (WidgetBase 트리 자동 전파)
- `Save()/Load()`: Children 순회 + `dynamic_pointer_cast<CVrdxNovelScene>`으로 탐색 후 직렬화

#### 7단계: Main.cpp 정리
**파일:** `Main.cpp`

- `CreateWidget<CVrdxApplication>(nullptr, Panel)` 방식으로 Application 생성
- SceneManager / Scene 관련 코드 제거

## 진행 상태

| 단계 | 상태 | 비고 |
|------|------|------|
| 1. WidgetBase 보강 | ✅ 완료 | BringToFront() |
| 2. Background Widget화 | ✅ 완료 | 이름 유지, WidgetBase 상속 |
| 3. CharacterManager Widget화 | ✅ 완료 | 이름 유지, WidgetBase 상속 |
| 4. NovelScene 전환 | ✅ 완료 | WidgetBase 상속 + Novel/로 이동 |
| 5. Scene/SceneManager 폐기 | ✅ 완료 | 파일 삭제 + 참조 정리 |
| 6. Application 정리 | ✅ 완료 | Ui/로 이동 + WidgetBase 상속 |
| 7. Main.cpp 정리 | ✅ 완료 | |

## 잔여 항목 (차후 단계)

- **TitleWindow / SaveLoadWindow** 구현 (기존 8단계 계획)
- **NovelScene의 Widget 자식 교체** — Background/CharacterManager/DialogueBox/ChoiceWidget이 모두 CreateWidget으로 생성되고 NovelScene의 멤버로 유지, 추후 NovelWindow로 rename 검토 가능
- **BringToFront() 활용 화면 전환** — TitleWindow/NovelWindow/SaveLoadWindow 간 전환에 적용

## 고려사항

- Save/Load는 Application의 Children을 순회하여 NovelScene을 찾는 방식 (범용적이나 다중 Window 시 검토 필요)
- `.vcxproj.filters` 필터: `Core`, `Ui`, `Novel` 세 개로 정리됨
- **UI 폴리싱(Polishing) 작업은 여전히 보류**
