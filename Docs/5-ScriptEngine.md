---
title: "4단계: ScriptEngine"
project: Voradorix
phase: 4
status: 구현 완료
tags:
  - voradorix
  - script-engine
  - parser
  - spec
  - cpp
---
# Voradorix — 4단계: ScriptEngine 구현 명세

> **대상**: 스크립트 파일 로드/파싱/명령 실행
> **프로젝트**: Voradorix
> **관련 파일**: `Src/Novel/ScriptEngine.h/cpp`, `Src/Novel/ScriptLine.h/cpp`, `Src/Scene/NovelScene.h/cpp` (연동), `Assets/Scripts/TestScript.txt`
> **의존성**: `Core/String.h`, `Scene/Scene.h`, `Ui/DialogueBox.h`, `Novel/Background.h`, `Novel/CharacterManager.h`, SFML 3.1.0
> **완료 조건**: 스크립트 파일을 읽어 배경/캐릭터/대사 명령이 순차 실행되고, NovelScene이 그 흐름을 제어할 수 있음
> **상태**: 4단계 구현 완료 (8개 명령어 지원, @label/@jump 분기, 실행 검증 완료)

---

## 1. 범위

4단계는 **시나리오 해석기**를 담당한다.

- UTF-8 스크립트 파일 로드
- 명령어/대사 라인 파싱
- 순차 실행 및 진행 제어
- NovelScene ↔ Background/CharacterManager/DialogueBox 연결

이번 단계에서는 **선택지 UI**, **세이브/로드**, **전역 이펙트 큐**를 완성하지 않는다.
다만 향후 확장을 위해 `@label`, `@jump` 같은 분기용 기초 구조는 확보한다.

---

## 2. 파일 구성

| # | 파일 | 설명 | 상태 |
|---|------|------|------|
| 1 | `Src/Novel/ScriptEngine.h` | `CVrdxScriptEngine` 선언 | ✅ 작성됨 |
| 2 | `Src/Novel/ScriptEngine.cpp` | `CVrdxScriptEngine` 구현 | ✅ 작성됨 |
| 3 | `Src/Novel/ScriptLine.h` | 명령어별 파생 struct + 팩토리 테이블 | ✅ 작성됨 |
| 4 | `Src/Novel/ScriptLine.cpp` | ParseScriptLine / Construct / Dispatch 구현 | ✅ 작성됨 |
| 5 | `Src/Scene/NovelScene.h/cpp` | ScriptEngine 연동 (shared_from_this, CanAdvance) | ✅ 수정 완료 |
| 6 | `Game.vcxproj` + `.filters` | ScriptEngine, ScriptLine 등록 | ✅ 등록 완료 |
| 7 | `Assets/Scripts/TestScript.txt` | @label/@jump 분기 포함 샘플 (25라인) | ✅ 준비 완료 |

---

## 3. 네이밍

| 구분 | 규칙 | 예시 |
|------|------|------|
| 클래스 | `C` + `Vrdx` 접두어 | `CVrdxScriptEngine` |
| 구조체 | `F` + `Vrdx` 접두어 | `FVrdxScriptLine` |
| 열거형 | `E` + `Vrdx` 접두어 | `EVrdxScriptLineType` |
| 파일명 | PascalCase | `ScriptEngine.h`, `ScriptEngine.cpp` |

---

## 4. ScriptEngine 역할

### 4.1 핵심 책임

`CVrdxScriptEngine`는 스크립트 파일을 읽고, 각 줄을 명령 단위로 해석한 뒤, NovelScene이 사용할 수 있는 실행 상태를 제공한다.

### 4.2 처리 대상

- 배경 전환
- 캐릭터 표시/숨김/포즈 변경
- 대사 출력
- 대기/레이블/점프

### 4.3 비대상

- 선택지 버튼 생성
- 세이브/로드
- BGM/SE 재생
- 화면 페이드/흔들림 같은 연출 큐

---

## 5. 스크립트 포맷

### 5.1 기본 규칙

- 인코딩: **UTF-8**
- 빈 줄은 무시
- 주석은 무시
- 한 줄에 하나의 명령 또는 대사만 허용
- 문자열은 큰따옴표(`"`) 사용

### 5.2 지원 문법 (4단계 구현 완료)

| 문법 | 의미 |
|------|------|
| `@bg "background_name"` | 배경 전환 |
| `@show "character" ["position"] ["pose"]` | 캐릭터 표시 (position 기본 Center) |
| `@hide "character"` | 캐릭터 숨김 |
| `@pose "character" "pose"` | 캐릭터 표정 변경 |
| `@wait "seconds"` | 일정 시간 대기 |
| `@label "name"` | 레이블 정의 |
| `@jump "name"` | 레이블로 이동 |
| `@dialogue "speaker" "text"` | 캐릭터 대사 출력 |

> 모든 명령어는 `@` 접두어 사용. 인자는 큰따옴표(`"`)로 묶음.
> 일반 텍스트 대사(`speaker "text"`, `"text"`)는 현재 미지원 — 추후 확장 예정.

### 5.3 후속 단계로 미룸

| 문법 | 비고 |
|------|------|
| `@choice` | 5단계 선택지 시스템에서 처리 |
| `@if`, `@set` | 분기/플래그 시스템과 함께 확장 |
| `@bgm`, `@se` | 오디오 파트와 함께 확장 |
| `@fadein`, `@fadeout`, `@shake` | 연출 효과 파트와 함께 확장 |

---

## 6. 내부 데이터 구조

### 6.1 파싱 결과 — `ScriptLine.h`

`FVrdxScriptLine`은 명령어별 파생 구조체의 기반 클래스.
팩토리 테이블 + `Construct()`/`Dispatch()` 2단계 패턴 사용.

```cpp
// 기반 클래스
struct FVrdxScriptLine
{
    virtual ~FVrdxScriptLine() VRDX_DEFAULT;
    static TVrdxSharedPtr<FVrdxScriptLine> ParseScriptLine(const std::string& Line);
    static bool Test();

    virtual bool Construct();   // Arguments 검증 및 필드 초기화
    virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene);  // 실행

    FVrdxString RawText;
    TVrdxVector<FVrdxString> Arguments;
};
```

#### 명령어별 파생 구조체

| 구조체 | 필드 | Dispatch 동작 |
|---|---|---|
| `FVrdxBackgroundScriptLine` | `MapName` | `Scene->SetBackground(MapName)` |
| `FVrdxShowCharacterScriptLine` | `CharacterName`, `Position`, `Pose` | `Scene->ShowCharacter(Name, Position)` |
| `FVrdxHideCharacterScriptLine` | `CharacterName` | `Scene->HideCharacter(Name)` |
| `FVrdxCharacterPoseScriptLine` | `CharacterName`, `Pose` | `Scene->SetCharacterPose(Name, Pose)` |
| `FVrdxWaitScriptLine` | `Seconds` | `Scene->WaitForSeconds(Seconds)` |
| `FVrdxLabelScriptLine` | `LabelName` | (no-op) |
| `FVrdxJumpScriptLine` | `TargetLabelName` | `Scene->JumpToLabel(TargetLabelName)` |
| `FVrdxDialogueScriptLine` | `Speaker`, `Dialogue` | `Scene->SetDialogue({Speaker, Dialogue})` → `true` 반환 |

#### `ParseScriptLine()` 처리 흐름

1. 정규식으로 `@command` 매칭
2. 팩토리 테이블(`unordered_map<string, Factory>`)에서 명령어 검색
3. 해당 파생 구조체 인스턴스 생성
4. `Arguments` 채움 (RawText에서 큰따옴표 문자열 추출)
5. `Construct()` 호출 — 인자 검증 및 필드 초기화, 실패 시 nullptr 반환

### 6.2 ScriptEngine 실행 상태

```
CurrentScriptLine  — 현재 실행 중인 줄 인덱스 (int32_t)
ScriptLines        — TVrdxVector<TVrdxSharedPtr<FVrdxScriptLine>> (파싱된 명령 리스트)
WeakNovelScene     — TVrdxWeakPtr<CVrdxNovelScene> (shared_from_this()로 받은 weak_ptr)
Labels             — unordered_map<FVrdxString, int32_t> (레이블 이름 → ScriptLines 인덱스)
```

- `Labels`는 `LoadScript()`에서 `@label` 라인을 만날 때마다 구축
- `CurrentScriptLine`은 `ParseLine()` 내부에서 post-increment (`ScriptLines[CurrentScriptLine++]`)

### 6.3 라인 분류

모든 명령은 `@` 접두어로 시작. 팩토리 테이블이 인식하는 8개 명령어:

- `@bg`, `@show`, `@hide`, `@pose`, `@wait`, `@label`, `@jump`, `@dialogue`

인식되지 않는 `@` 명령어나 일반 텍스트는 `nullptr` 반환 → 무시.
빈 줄과 주석(미구현)도 무시.

---

## 7. 공개 인터페이스

### 7.1 클래스 개요 (현재 구현)

```cpp
class CVrdxScriptEngine
{
public:
    void SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene);
    bool LoadScript(const FVrdxString& ScriptPath);
    void Reset();

    void Update(float DeltaTick);

    bool CanAdvance() const;
    bool IsFinished() const;
    void JumpToLabel(const FVrdxString& TargetLabelName);

private:
    bool ParseLine();

    int32_t CurrentScriptLine;
    TVrdxVector<TVrdxSharedPtr<FVrdxScriptLine>> ScriptLines;
    TVrdxWeakPtr<CVrdxNovelScene> WeakNovelScene;
    std::unordered_map<FVrdxString, int32_t> Labels;
};
```

### 7.2 동작 의미

| 메서드 | 설명 |
|--------|------|
| `SetNovelScene(shared_ptr)` | NovelScene의 `shared_from_this()`를 `weak_ptr`로 저장 |
| `LoadScript(Path)` | 스크립트 파일 읽기 → 줄 단위 파싱 → Labels 맵 구축 |
| `Reset()` | (미구현) 실행 상태 초기화 |
| `Update(DeltaTick)` | 내부 while 루프: `CanAdvance() && ParseLine()` 반복 실행 |
| `CanAdvance()` | `NovelScene->CanAdvance()` 위임 (typing/wait/input 상태 확인) |
| `IsFinished()` | `CurrentScriptLine >= ScriptLines.Num()` — 스크립트 종료 여부 |
| `JumpToLabel(Target)` | Labels[Target]으로 `CurrentScriptLine` 설정 |

---

## 8. 실행 규칙

### 8.1 기본 흐름

1. `OnEnter()` → `ScriptEngine.SetNovelScene(shared_from_this())` + `LoadScript()`
2. `Update()`가 프레임마다 호출 → 내부 while 루프
3. while 루프: `CanAdvance()`가 true면 `ParseLine()` 실행, false면 중단
4. `ParseLine()` → `ScriptLines[CurrentScriptLine++]->Dispatch(NovelScene)`
5. Dispatch 결과가 true면 즉시 다음 줄 시도 (루프 계속), false면 중단
6. @wait/@dialogue는 NovelScene의 상태를 변경하여 CanAdvance() 차단

### 8.2 CanAdvance 체인

```
ScriptEngine::CanAdvance()
  └→ NovelScene::CanAdvance()
        ├─ !DialogueBox.IsTyping()      — 타이핑 중이면 대기
        ├─ !(RemainingWaitSeconds > 0)  — @wait 시간이 남았으면 대기
        └─ !bWaitingInput               — 사용자 입력 대기 중이면 대기
```

- `@dialogue` Dispatch: `SetLine()`이 `StartTyping()` 호출 → IsTyping() 즉시 true → CanAdvance false
- `@wait` Dispatch: `RemainingWaitSeconds = Seconds` → CanAdvance false
- 사용자 입력(Enter/Space/Click): `bWaitingInput = false` → CanAdvance true

### 8.3 명령 처리 상세

| 명령 | Dispatch 반환 | CanAdvance 영향 | 설명 |
|---|---|---|---|
| `@bg` | `true` | 없음 (계속 진행) | `Background.SetBackground(Name)` 즉시 전환 시작 (비동기 페이드) |
| `@show` | `true` | 없음 (계속 진행) | 캐릭터 즉시 표시 |
| `@hide` | `true` | 없음 (계속 진행) | 캐릭터 즉시 숨김 |
| `@pose` | `true` | 없음 (계속 진행) | 표정 즉시 변경 |
| `@wait` | `true` | `RemainingWaitSeconds > 0`으로 차단 | Update에서 감소, 0 도달 시 해제 |
| `@label` | `true` | 없음 (계속 진행) | no-op (LoadScript에서 이미 맵 구축) |
| `@jump` | `true` | 없음 (계속 진행) | CurrentScriptLine 재설정 → 다음 줄부터 실행 |
| `@dialogue` | `true` | `IsTyping() == true`로 차단 | SetLine → StartTyping, 입력 시 FinishTyping |

### 8.4 확인 입력 (NovelScene.HandleEvent)

- 타이핑 중(`DialogueBox.IsTyping()`) → `DialogueBox.FinishTyping()` (전체 표시, CanAdvance는 아직 false)
- 타이핑 완료(`DialogueBox.IsFinished()`) → `bWaitingInput = false` (CanAdvance 해제)

### 8.5 @jump 세부 동작

```
@jump "Target"
  └→ NovelScene::JumpToLabel("Target")
        └→ ScriptEngine::JumpToLabel("Target")
              └→ CurrentScriptLine = Labels["Target"]
```

- Labels는 `LoadScript()`에서 `@label` 라인 인덱스를 저장한 맵
- `@jump`는 `@label` 바로 다음 줄(또는 해당 인덱스)부터 실행
- 존재하지 않는 레이블: `JumpToLabel()`은 아무 동작 안 함 (맵.find() 실패 시 무시)

### 8.6 @wait 세부 동작

```
@wait "0.3"
  └→ NovelScene::WaitForSeconds(0.3f)
        └→ RemainingWaitSeconds = 0.3f
```

- `Update()`에서 매 프레임 `RemainingWaitSeconds -= DeltaTick`
- `CanAdvance()`가 `RemainingWaitSeconds > 0` 체크 — 0 이하가 될 때까지 대기

### 8.7 연속 실행 최적화

ScriptEngine의 `Update()`는 while 루프로 동작하므로,
`@bg` → `@show` → `@wait`처럼 연속된 명령은 한 프레임 내에서 모두 실행됨.
`@wait`이나 `@dialogue`처럼 CanAdvance를 차단하는 명령에서만 중단.

---

## 9. NovelScene 연동

### 9.1 책임 분리

- `NovelScene`: 화면 갱신, 입력 전달, 렌더 순서 유지, `CanAdvance()` 상태 통합
- `ScriptEngine`: 스토리 진행 상태와 명령 해석, `weak_ptr<NovelScene>`으로 Dispatch
- `DialogueBox`: 대사 UI 출력 및 타이핑 애니메이션
- `Background` / `CharacterManager`: 비주얼 연출 실행

### 9.2 소유권 구조

```
SceneManager (TVrdxSharedPtr<CVrdxScene>)
  └── NovelScene (shared_ptr, enable_shared_from_this)
        ├── ScriptEngine (값 멤버)
        │     └── WeakNovelScene (weak_ptr, set via shared_from_this())
        ├── Background (값 멤버)
        ├── CharacterManager (값 멤버)
        └── DialogueBox (값 멤버)
```

### 9.3 갱신 순서 (NovelScene::Update)

```
① ScriptEngine.Update(DeltaTick)
    └─ while CanAdvance() && ParseLine()
② Background.Update(DeltaTick)
③ CharacterManager.Update(DeltaTick)
④ DialogueBox.Update(DeltaTick)
⑤ RemainingWaitSeconds 감소
⑥ bWaitingInput = DialogueBox.IsFinished()
```

### 9.4 렌더 순서 (NovelScene::Draw)

```
① Window.clear(Color::Black)
② Background.Draw(Window)
③ CharacterManager.Draw(Window)
④ DialogueBox.Draw(Window)
```

### 9.5 입력 처리 (NovelScene::HandleEvent)

- Enter/Space/Left Click 감지
- 타이핑 중 → `DialogueBox.FinishTyping()`
- 타이핑 완료 → `bWaitingInput = false` (ScriptEngine 진행 허용)

---

## 10. 오류 처리

- **파일 없음**: `LoadScript()` false 반환 → `OnEnter()`에서 조기 리턴
- **알 수 없는 명령**: `ParseScriptLine()`이 `nullptr` 반환 → `ScriptLines`에 추가 안 됨 (무시)
- **인수 부족**: `Construct()`에서 `Arguments.Num()` 검증 실패 → `nullptr` 반환 (무시)
- **존재하지 않는 레이블**: `JumpToLabel()`에서 `Labels.find()` 실패 시 아무 동작 안 함 (조용히 무시)
- **잘못된 배경/캐릭터 이름**: `Background.SetBackground()`/`CharacterManager` 내부에서 텍스처 로드 실패 시 투명 텍스처 fallback

> **참고**: 현재 오류 메시지 출력(로그)은 구현되어 있지 않음. 추후 로그 시스템 도입 시 개선 예정.

---

## 11. 테스트 스크립트 — `Assets/Scripts/TestScript.txt`

```text
@jump "Main"

@label "A"
@bg "WhiteRoom_Sunset"
@dialogue "Laura" "확인되었나요? B로 점프합니다."
@jump "B"

@label "B"
@bg "WhiteRoom"
@dialogue "Laura" "확인되었나요? 자 이제 마무리!"
@jump "Finish"

@label "Main"
@bg "WhiteRoom"
@show "Laura"
@dialogue "Laura" "안녕하세요?"
@dialogue "Laura" "숨어볼게요."
@hide "Laura"
@dialogue "Laura" "어때요? 다시 나갈게요."
@show "Laura"
@dialogue "Laura" "헤헤 이제 레이블 테스트를 할게요. A로 점프합니다."
@jump "A"

@label "Finish"
@dialogue "Laura" "마지막 레이블에 도달했어요. 수고했어요."
```

- 총 25라인, 8개 `@label`/`@jump` 분기 포함
- 실행 경로: Main → A → B → Finish
- `@show`/`@hide`로 캐릭터 등장/퇴장, `@bg`로 배경 전환, `@dialogue`로 대사 출력

---

## 12. 검증 방법

- 스크립트 파일(`Assets/Scripts/TestScript.txt`)을 정상적으로 읽는가
- 배경/캐릭터/대사가 순서대로 실행되는가
- `@wait` 동안 진행이 멈추는가
- `@jump`가 레이블로 정확히 이동하는가 (Main → A → B → Finish)
- `@show`/`@hide`로 캐릭터 표시/숨김이 동작하는가
- 잘못된 명령이 있어도 전체 실행이 중단되지 않는가
