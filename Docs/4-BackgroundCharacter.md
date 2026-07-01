---
title: "3단계: Background + Character"
project: Voradorix
phase: 3
status: 설계
tags:
  - voradorix
  - background
  - character
  - spec
  - cpp
---
# Voradorix — 3단계: Background + Character 구현 명세

> **대상**: 배경 전환 + 캐릭터 스프라이트 표시/전환
> **프로젝트**: Voradorix
> **관련 파일**: `Src/Novel/Background.h/cpp`, `Src/Novel/CharacterManager.h/cpp`, `Src/Novel/NovelScene.h/cpp` (연동), `Assets/Backgrounds/`, `Assets/Characters/`
> **의존성**: SFML 3.1.0 (`sf::Texture`, `sf::Sprite`, `sf::RectangleShape`, `sf::Clock`)
> **완료 조건**: NovelScene이 배경을 전환하고, 좌/중/우 캐릭터 슬롯에 스프라이트를 표시·교체하며, 표정 변경을 자연스럽게 처리

---

## 1. 범위

3단계는 **연출 레이어**를 담당한다.

- 배경 이미지 표시 및 페이드 전환
- 캐릭터 스프라이트 로드/배치/표정 변경
- NovelScene과 DialogueBox 사이의 시각적 레이어 구성

이번 단계에서는 **스크립트 파싱**이나 **선택지 분기**를 다루지 않는다. 명령 해석은 4단계 `ScriptEngine`에서 담당한다.

---

## 2. 파일 구성

| # | 파일 | 설명 | 상태 |
|---|------|------|------|
| 1 | `Src/Novel/Background.h` | CBackground 선언 | 📝 작성 예정 |
| 2 | `Src/Novel/Background.cpp` | CBackground 구현 | 📝 작성 예정 |
| 3 | `Src/Novel/CharacterManager.h` | CCharacterManager 선언 | 📝 작성 예정 |
| 4 | `Src/Novel/CharacterManager.cpp` | CCharacterManager 구현 | 📝 작성 예정 |
| 5 | `Src/Novel/NovelScene.h/cpp` | 배경/캐릭터 렌더링 연동 | 📝 수정 예정 |
| 6 | `Game.vcxproj` | ClCompile/ClInclude 항목 등록 | 📝 등록 예정 |
| 7 | `Game.vcxproj.filters` | Novel/ 필터 구성 | 📝 구성 예정 |

---

## 3. 네이밍

| 구분 | 규칙 | 예시 |
|------|------|------|
| 클래스 | `C` + `Vrdx` 접두어는 문맥상 생략 가능 | `CBackground`, `CCharacterManager` |
| 파일명 | PascalCase | `Background.h`, `CharacterManager.cpp` |
| 구조체 | `F` + `Vrdx` | `FCharacterSlot`, `FBackgroundTransition` |
| 열거형 | `E` + `Vrdx` | `EVrdxCharacterSlot` |

---

## 4. CBackground — 배경 전환 담당

### 4.1 역할

배경 이미지를 로드하고, 현재 배경과 다음 배경 사이를 페이드로 전환한다.

### 4.2 클래스 개요

```cpp
class CBackground
{
public:
    CBackground();
    ~CBackground() VRDX_DEFAULT;

    void SetBackground(const FString& AssetName, float TransitionSeconds = 0.35f);
    void Update(float DeltaTick);
    void Draw(sf::RenderWindow& Window) const;

    bool IsTransitioning() const;

private:
    sf::Texture CurrentTexture;
    sf::Texture NextTexture;
    sf::Sprite   CurrentSprite;
    sf::Sprite   NextSprite;

    float        TransitionTime;
    float        TransitionDuration;
    bool         bHasCurrent;
    bool         bIsTransitioning;
};
```

### 4.3 동작 규칙

- `SetBackground()` 호출 시 새 배경을 요청한다.
- 동일 배경 재요청은 무시할 수 있다.
- 전환 중에는 `Update()`가 진행도를 갱신하고, `Draw()`가 두 스프라이트를 함께 렌더링한다.
- 전환 종료 후에는 다음 배경이 현재 배경이 된다.

### 4.4 배경 출력 규칙

- 기본 해상도는 1280×720 기준
- 배경은 화면 전체를 덮도록 스케일 조정
- UI보다 뒤에 렌더링

---

## 5. CCharacterManager — 캐릭터 표시 담당

### 5.1 역할

캐릭터 스프라이트를 좌/중/우 슬롯에 배치하고, 표정·가시성·전환을 관리한다.

### 5.2 슬롯 규칙

| 슬롯 | 위치 |
|------|------|
| Left | 화면 좌측 |
| Center | 화면 중앙 |
| Right | 화면 우측 |

### 5.3 데이터 구조

```cpp
enum class EVrdxCharacterSlot
{
    Left,
    Center,
    Right,
};

struct FCharacterSlot
{
    FString CharacterName;
    FString PoseName;
    EVrdxCharacterSlot Slot;
    bool bVisible = false;
    float Alpha = 1.0f;
};
```

### 5.4 클래스 개요

```cpp
class CCharacterManager
{
public:
    CCharacterManager();
    ~CCharacterManager() VRDX_DEFAULT;

    void ShowCharacter(const FString& CharacterName, const FString& PoseName, EVrdxCharacterSlot Slot);
    void HideCharacter(const FString& CharacterName, float FadeSeconds = 0.25f);
    void ClearSlot(EVrdxCharacterSlot Slot, float FadeSeconds = 0.25f);

    void Update(float DeltaTick);
    void Draw(sf::RenderWindow& Window) const;

private:
    void UpdateSlotTransform(FCharacterSlot& SlotState);

    TVrdxVector<FCharacterSlot> Slots;
};
```

### 5.5 동작 규칙

- 캐릭터는 동일 슬롯에 하나만 표시한다.
- 표정 변경은 같은 캐릭터의 다른 `PoseName`을 불러와 교체한다.
- 캐릭터 숨김은 즉시 제거보다 페이드아웃을 우선한다.
- 슬롯 재사용 시 이전 스프라이트와 겹치지 않도록 알파와 가시성을 정리한다.

### 5.6 캐릭터 배치

- 중심점 기준 정렬을 기본으로 한다.
- 발끝/허리 기준점은 캐릭터 리소스 제작 규칙에 맞춰 보정 가능하게 둔다.
- 레이어 우선순위는 Left → Center → Right 순으로 겹침을 조절할 수 있다.

---

## 6. NovelScene 연동

### 6.1 렌더 순서

1. 배경
2. 캐릭터
3. 대사창

### 6.2 갱신 순서

1. 배경 전환 업데이트
2. 캐릭터 전환/알파 업데이트
3. 대사창 업데이트

### 6.3 이벤트 연동

이 단계에서는 NovelScene이 직접 배경/캐릭터를 조작할 수 있는 내부 API만 확보한다.

- 배경 교체 요청
- 캐릭터 표시/숨김 요청
- 표정 변경 요청

실제 스크립트 명령 매핑은 4단계에서 연결한다.

---

## 7. 에셋 규칙

### 7.1 배경

- 위치: `Assets/Backgrounds/`
- 형식: PNG 권장, JPG 허용
- 네이밍: `scene_name.png`, `location_name.png`

### 7.2 캐릭터

- 위치: `Assets/Characters/`
- 구조 예시: `Assets/Characters/yuki/smile.png`
- 표정/포즈는 파일명으로 구분

### 7.3 로딩 원칙

- 파일명 기반 캐시를 둔다.
- 중복 로딩을 피하고, 실패 시 기본 대체 리소스를 사용할 수 있게 한다.

---

## 8. 구현 순서

1. `CBackground` 설계/구현
2. `CCharacterManager` 설계/구현
3. `NovelScene`에 렌더 순서 연동
4. `Game.vcxproj` 및 필터 등록
5. 배경/캐릭터 테스트용 장면 확인

---

## 9. 검증 방법

- 배경 변경 시 깜빡임 없이 전환되는가
- 캐릭터 슬롯이 겹치지 않고 올바른 위치에 표시되는가
- 표정 변경 시 기존 스프라이트가 자연스럽게 교체되는가
- 대사창이 항상 최상단에 표시되는가
