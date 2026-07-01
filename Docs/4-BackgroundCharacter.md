---
title: "3단계: Background + Character"
project: Voradorix
phase: 3
status: 구현 중 (Background + CharacterManager 완료)
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
> **의존성**: SFML 3.1.0 (`sf::Texture`, `sf::Sprite`, `sf::Image`, `sf::Color`), `Core/String.h`
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

| #   | 파일                               | 설명                         |    상태    |
| --- | -------------------------------- | -------------------------- | :------: |
| 1   | `Src/Novel/Background.h`         | `CVrdxBackground` 선언       |   ✅ 완료   |
| 2   | `Src/Novel/Background.cpp`       | `CVrdxBackground` 구현       |   ✅ 완료   |
| 3   | `Src/Novel/CharacterManager.h`   | `CVrdxCharacterManager` 선언 |   ✅ 완료   |
| 4   | `Src/Novel/CharacterManager.cpp` | `CVrdxCharacterManager` 구현 |   ✅ 완료   |
| 5   | `Src/Novel/NovelScene.h/cpp`     | 배경/캐릭터 렌더링 연동              | 📝 수정 예정 |
| 6   | `Game.vcxproj`                   | ClCompile/ClInclude 항목 등록  | 📝 등록 예정 |
| 7   | `Game.vcxproj.filters`           | Novel/ 필터 구성               | 📝 구성 예정 |

---

## 3. 네이밍

| 구분 | 규칙 | 예시 |
|------|------|------|
| 클래스 | `C` + `Vrdx` 접두어 | `CVrdxBackground`, `CVrdxCharacterManager` |
| 파일명 | PascalCase | `Background.h`, `CharacterManager.cpp` |
| 구조체 | `F` + `Vrdx` | `FVrdxCharacterSlot` |
| 열거형 | `E` + `Vrdx` | `EVrdxCharacterPosition` |

---

## 4. CVrdxBackground — 배경 전환 담당

### 4.1 역할

배경 이미지를 로드하고, 현재 배경과 다음 배경 사이를 페이드로 전환한다.

### 4.2 클래스 개요

```cpp
class CVrdxBackground
{
public:
    CVrdxBackground(float TargetWidth, float TargetHeight);
    ~CVrdxBackground() VRDX_DEFAULT;

    void SetBackground(const FVrdxString& AssetName, float TransitionSeconds = 0.35f);
    void Update(float DeltaTick);
    void Draw(sf::RenderWindow& Window) const;

    bool IsTransitioning() const;

private:
    void SetupSprite(sf::Sprite& Sprite, const sf::Texture& Texture) const;
    void ApplyFullScreenTransform(sf::Sprite& Sprite, const sf::Texture& Texture) const;
    void SetSpriteAlpha(sf::Sprite& Sprite, float Alpha) const;
    static sf::Texture CreateTransparentTexture();

    float        TargetWidth;
    float        TargetHeight;
    sf::Texture  TransparentTexture;
    sf::Texture  CurrentTexture;
    sf::Texture  NextTexture;
    sf::Sprite   CurrentSprite;
    sf::Sprite   NextSprite;

    FVrdxString  CurrentAssetName;
    FVrdxString  NextAssetName;

    float        TransitionTime;
    float        TransitionDuration;
    bool         bHasCurrent;
    bool         bIsTransitioning;
};
```

### 4.3 생성자

```cpp
CVrdxBackground(float TargetWidth, float TargetHeight);
```

- `TargetWidth`, `TargetHeight`: 출력 해상도 (배경 스프라이트를 이 크기에 맞게 스케일)
- 내부에서 `CreateTransparentTexture()`로 1×1 투명 텍스처를 생성하여 초기 Sprite에 할당

### 4.4 공개 메서드

| 메서드 | 설명 |
|--------|------|
| `SetBackground(AssetName, TransitionSeconds)` | 새 배경 로드 요청. **중복 요청 무시** (전환 중이 아닐 때 동일 Asset 재요청, 전환 중일 때 동일 NextAsset 재요청 → 무시). `Assets/Backgrounds/` 아래에서 파일 로드 |
| `Update(DeltaTick)` | 전환 시간 누적. `TransitionDuration` 경과 시 전환 완료 처리 (Next → Current 교체, `bIsTransitioning` 해제) |
| `Draw(Window)` | 비전환 시 `CurrentSprite` 단독 출력. 전환 중에는 현재/다음 스프라이트를 진행도(`TransitionTime / TransitionDuration`)에 따라 알파 블렌딩하여 함께 출력 |
| `IsTransitioning()` | 전환 중 여부 반환 |

### 4.5 비공개 헬퍼

| 메서드 | 설명 |
|--------|------|
| `SetupSprite(Sprite, Texture)` | 스프라이트에 텍스처를 설정하고 `ApplyFullScreenTransform()` 호출, 색상 White로 초기화 |
| `ApplyFullScreenTransform(Sprite, Texture)` | 스프라이트를 (0,0)에 위치시키고 `TargetWidth/TargetHeight`에 맞게 스케일 조정. 텍스처 크기가 0이면 무시 |
| `SetSpriteAlpha(Sprite, Alpha)` | 0~255 범위로 클램프 후 스프라이트 색상 알파 채널 설정 |
| `CreateTransparentTexture()` (static) | 1×1 투명 PNG 이미지로 `sf::Texture` 생성. 초기 빈 상태 표시용 |

### 4.6 동작 규칙

- `SetBackground()` 호출 시 새 배경을 요청한다.
- **동일 배경 재요청은 무시**한다 (`(bHasCurrent && !bIsTransitioning && AssetName == CurrentAssetName)` 또는 `(bIsTransitioning && AssetName == NextAssetName)`).
- 전환 시간이 0 이하이거나 첫 배경(`!bHasCurrent`)이면 **즉시 전환** (페이드 없음).
- 전환 중일 때 `Update()`는 시간을 누적하고, `Draw()`가 실시간 진행도에 따라 두 스프라이트를 알파 블렌딩으로 렌더링한다.
- 전환 완료 시 Next → Current 교체, NextSprite는 투명 텍스처로 리셋, `bIsTransitioning` 해제.

### 4.7 배경 출력 규칙

- 생성자에서 `TargetWidth`, `TargetHeight`를 받아 고정 해상도 기준으로 동작
- 배경은 화면 전체를 덮도록 스케일 조정 (`ApplyFullScreenTransform`)
- UI보다 뒤에 렌더링 (NovelScene의 렌더 순서에서 최하위)

---

## 5. CVrdxCharacterManager — 캐릭터 표시 담당

### 5.1 역할

캐릭터 스프라이트를 좌/중/우 슬롯에 배치하고, 표정·가시성·페이드 전환을 관리한다. 내부 텍스처 캐시로 중복 로딩을 방지한다.

### 5.2 슬롯 규칙

| 슬롯 | X 좌표 | 설명 |
|------|--------|------|
| Left | 320 | 화면 좌측 |
| Center | 640 | 화면 중앙 |
| Right | 960 | 화면 우측 |

캐릭터 수직 기준점(`SlotAnchorY`)은 660px, 최대 높이(`MaxCharacterHeight`)는 640px로 제한된다.

### 5.3 열거형 및 구조체

```cpp
enum class EVrdxCharacterPosition
{
    Left,
    Center,
    Right,
};

struct FVrdxCharacterSlot
{
    FVrdxCharacterSlot(EVrdxCharacterPosition InSlot, const sf::Texture& TransparentTexture);

    FVrdxString CharacterName;
    FVrdxString PoseName;
    EVrdxCharacterPosition Slot;
    bool bVisible = false;
    float Alpha = 0.0f;
    float StartAlpha = 0.0f;
    float TargetAlpha = 0.0f;
    float FadeTime = 0.0f;
    float FadeDuration = 0.0f;
    TVrdxSharedPtr<sf::Texture> Texture;
    sf::Sprite Sprite;
};
```

### 5.4 클래스 개요

```cpp
class CVrdxCharacterManager
{
public:
    CVrdxCharacterManager();
    ~CVrdxCharacterManager() VRDX_DEFAULT;

    void ShowCharacter(const FVrdxString& CharacterName, const FVrdxString& PoseName, EVrdxCharacterPosition Position);
    void HideCharacter(const FVrdxString& CharacterName, float FadeSeconds = 0.25f);
    void ClearSlot(EVrdxCharacterPosition Slot, float FadeSeconds = 0.25f);

    void Update(float DeltaTick);
    void Draw(sf::RenderWindow& Window) const;

private:
    static FVrdxString MakeTextureKey(const FVrdxString& CharacterName, const FVrdxString& PoseName);
    static FVrdxString MakeTexturePath(const FVrdxString& CharacterName, const FVrdxString& PoseName);
    static float GetSlotX(EVrdxCharacterPosition Slot);
    static sf::Texture CreateTransparentTexture();

    FVrdxCharacterSlot* FindSlotByPosition(EVrdxCharacterPosition Slot);
    FVrdxCharacterSlot* FindSlotByCharacter(const FVrdxString& CharacterName);
    const TVrdxSharedPtr<sf::Texture>& ResolveTexture(const FVrdxString& CharacterName, const FVrdxString& PoseName);
    void ResetSlot(FVrdxCharacterSlot& SlotState);
    void BeginFade(FVrdxCharacterSlot& SlotState, float TargetAlpha, float FadeSeconds);
    void UpdateSlotTransform(FVrdxCharacterSlot& SlotState);
    void SetSpriteAlpha(sf::Sprite& Sprite, float Alpha) const;

    TVrdxSharedPtr<sf::Texture> TransparentTexture;
    std::map<FVrdxString, TVrdxSharedPtr<sf::Texture>> TextureCache;
    TVrdxVector<FVrdxCharacterSlot> Slots;
};
```

### 5.5 생성자

- 3개의 슬롯(Left/Center/Right)을 미리 생성, 각각 투명 텍스처로 초기화
- `CreateTransparentTexture()`로 1×1 투명 텍스처 생성하여 공유

### 5.6 공개 메서드

| 메서드 | 설명 |
|--------|------|
| `ShowCharacter(CharacterName, PoseName, Position)` | 지정 위치 슬롯에 캐릭터 스프라이트를 표시. 텍스처 캐시에서 로드하거나 파일(`Assets/Characters/{Name}/{Pose}.png`)에서 로드하여 캐시에 저장. 알파를 즉시 1.0으로 설정, 페이드 없이 표시 |
| `HideCharacter(CharacterName, FadeSeconds)` | 이름이 일치하는 슬롯을 페이드아웃. `BeginFade(Slot, 0.0f, FadeSeconds)` 호출 |
| `ClearSlot(Position, FadeSeconds)` | 지정 위치 슬롯을 페이드아웃. `BeginFade(Slot, 0.0f, FadeSeconds)` 호출 |
| `Update(DeltaTick)` | 페이드 중인 모든 슬롯의 `Alpha`를 보간(`StartAlpha → TargetAlpha`). 페이드 완료 시 `TargetAlpha <= 0`이면 `ResetSlot`으로 비움 |
| `Draw(Window)` | `bVisible == true` 또는 `Alpha > 0`인 슬롯의 스프라이트를 현재 `Alpha` 값으로 알파 적용하여 렌더링 |

### 5.7 페이드 시스템

`BeginFade`는 세 가지 경우를 처리한다:

| 조건 | 동작 |
|------|------|
| `FadeSeconds <= 0` && `TargetAlpha <= 0` | 즉시 `ResetSlot` (완전 제거) |
| `FadeSeconds <= 0` && `TargetAlpha > 0` | 즉시 알파 설정 (페이드 없음) |
| `FadeSeconds > 0` | `StartAlpha` = 현재 Alpha, `TargetAlpha` = 목표값, 타이머 시작 |

`Update()`는 프레임마다 `FadeTime / FadeDuration` 진행도로 선형 보간하여 `Alpha`를 갱신하고, 완료 시 슬롯을 정리한다.

### 5.8 텍스처 캐싱

- `std::map<FVrdxString, TVrdxSharedPtr<sf::Texture>> TextureCache` — 키는 `"{CharacterName}/{PoseName}"` 문자열
- `ResolveTexture()`: 캐시 조회 후 미스 시 파일 로드 후 캐시에 등록, 실패 시 투명 텍스처 반환

### 5.9 캐릭터 배치

- 중심점 기준 정렬 (`setOrigin(TextureSize * 0.5f)`)
- 세로 비율 유지하며 `MaxCharacterHeight(640px)` 이내로 스케일
- 위치: `(GetSlotX(Position), SlotAnchorY(660))` — 즉, 화면 하단에서 60px 위
- 레이어 우선순위: 슬롯 배열 순서 (Left → Center → Right 순으로 드로우)

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

1. ✅ `CVrdxBackground` 설계/구현
2. ✅ `CVrdxCharacterManager` 설계/구현
3. ⬜ `NovelScene`에 렌더 순서 연동
4. ⬜ `Game.vcxproj` 및 필터 등록
5. ⬜ 배경/캐릭터 테스트용 장면 확인

---

## 9. 검증 방법

- 배경 변경 시 깜빡임 없이 전환되는가
- 캐릭터 슬롯이 겹치지 않고 올바른 위치에 표시되는가
- 표정 변경 시 기존 스프라이트가 자연스럽게 교체되는가
- 대사창이 항상 최상단에 표시되는가
