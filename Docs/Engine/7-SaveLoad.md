---
title: "7단계: SaveLoad"
project: Voradorix
phase: 7
status: 명세 작성 완료
tags:
  - voradorix
  - save-load
  - persistence
  - spec
  - cpp
---
# Voradorix — 7단계: Save/Load 기능 명세

> **대상**: 저장/불러오기 데이터 구조 + SaveManager + SaveLoadScene + 씬 상태 복원  
> **프로젝트**: Voradorix  
> **관련 파일**: `Src/Save/SaveManager.h/cpp`, `Src/Scene/SaveLoadScene.h/cpp`, `Src/Scene/NovelScene.h/cpp`, `Src/Scene/TitleScene.h/cpp`(추후), `Assets/Saves/`  
> **의존성**: `Core/String.h`, `Core/Vector.h`, `Scene/Scene.h`, SFML 3.1.0, JSON 직렬화 계층(추후 선택)  
> **완료 조건**: 현재 진행 중인 노벨 상태를 저장하고, 저장된 슬롯을 다시 불러와 동일한 진행 지점부터 재개할 수 있음

---

## 1. 범위

7단계는 **세이브/로드**를 담당한다.

- 저장 슬롯 목록 관리
- 현재 노벨 진행 상태 직렬화
- 저장 파일 생성/갱신
- 저장 데이터 복원 및 씬 재진입

이번 단계에서는 **타이틀 화면**, **설정 메뉴**, **연출 효과**, **조건 분기 시스템**은 다루지 않는다.

---

## 2. 파일 구성

| # | 파일 | 설명 | 상태 |
|---|------|------|------|
| 1 | `Src/Save/SaveManager.h` | 저장 데이터/슬롯 인터페이스 선언 | 📝 예정 |
| 2 | `Src/Save/SaveManager.cpp` | 저장/불러오기 구현 | 📝 예정 |
| 3 | `Src/Scene/SaveLoadScene.h` | 저장/불러오기 UI 씬 선언 | 📝 예정 |
| 4 | `Src/Scene/SaveLoadScene.cpp` | 저장/불러오기 UI 씬 구현 | 📝 예정 |
| 5 | `Src/Scene/NovelScene.h/cpp` | 상태 직렬화/복원 연동 | 📝 예정 |
| 6 | `Game.vcxproj` + `.filters` | Save/, SaveLoadScene 등록 | 📝 예정 |
| 7 | `Assets/Saves/` | 저장 파일 보관 폴더 | 📝 예정 |

---

## 3. 설계 원칙

- 저장은 **슬롯 기반**으로 한다.
- 저장 데이터는 **문자열/숫자 중심의 직렬화 가능한 형태**로 유지한다.
- `NovelScene`이 직접 상태를 모으고, `SaveManager`는 저장/불러오기 수단만 담당한다.
- 불러온 뒤에는 **씬 초기화 → 상태 복원 → 재생 재개** 순서를 따른다.

---

## 4. 저장 대상

### 4.1 필수 저장 항목

- 현재 시나리오 파일 경로
- 현재 재생 위치(레이블/스크립트 인덱스)
- 배경 이름
- 표시 중인 캐릭터 목록
- 각 캐릭터의 위치/표정 상태
- 현재 대사 스피커/텍스트
- 선택지 대기 여부
- 남은 대기 시간

### 4.2 선택 저장 항목

- 마지막 선택지 결과
- 플레이 타임
- 저장 시각
- 사용자 지정 메모
- 추후 플래그/변수 상태

---

## 5. 데이터 구조

### 5.1 저장 슬롯 데이터

```cpp
struct FVrdxSaveSlot
{
    FVrdxString SlotId;
    FVrdxString Title;
    FVrdxString SavedAt;
    FVrdxString SceneName;
    FVrdxString ScreenshotPath;
    bool bOccupied = false;
};
```

### 5.2 노벨 진행 상태

```cpp
struct FVrdxNovelSaveData
{
    FVrdxString ScriptPath;
    FVrdxString CurrentLabel;
    FVrdxString BackgroundName;
    FVrdxString DialogueSpeaker;
    FVrdxString DialogueText;
    float RemainingWaitSeconds = 0.f;
    bool bChoiceWaiting = false;
};
```

### 5.3 캐릭터 상태

```cpp
struct FVrdxCharacterSaveData
{
    FVrdxString CharacterName;
    FVrdxString Pose;
    FVrdxString Position;
    bool bVisible = false;
};
```

---

## 6. SaveManager

### 6.1 역할

저장 파일을 읽고 쓰며, 슬롯 메타데이터를 관리한다.

### 6.2 공개 인터페이스

```cpp
class CVrdxSaveManager
{
public:
    bool SaveSlot(const FVrdxString& SlotId, const FVrdxNovelSaveData& Data, const TVrdxVector<FVrdxCharacterSaveData>& Characters);
    bool LoadSlot(const FVrdxString& SlotId, FVrdxNovelSaveData& OutData, TVrdxVector<FVrdxCharacterSaveData>& OutCharacters);
    TVrdxVector<FVrdxSaveSlot> GetSlots() const;
    bool DeleteSlot(const FVrdxString& SlotId);
};
```

### 6.3 저장 포맷

- 기본 포맷은 JSON 계열로 가정한다.
- 파일명은 `slot_01.json` 같은 형태를 사용한다.
- 메타데이터와 본문 데이터를 분리할 수 있다.

---

## 7. SaveLoadScene

### 7.1 역할

저장/불러오기 슬롯을 보여주고, 슬롯 선택과 동작을 처리한다.

### 7.2 기능

- 슬롯 목록 표시
- 저장/불러오기 모드 전환
- 슬롯 선택
- 빈 슬롯 표시
- 덮어쓰기 확인

### 7.3 UI 구성

- 슬롯 카드/행 목록
- 현재 선택 강조
- 저장일/장면명/간단한 미리보기 표시

---

## 8. NovelScene 연동

### 8.1 저장 시

1. 현재 상태를 `FVrdxNovelSaveData`로 수집
2. 캐릭터 상태를 배열로 수집
3. `SaveManager.SaveSlot()` 호출

### 8.2 불러오기 시

1. `SaveManager.LoadSlot()`로 데이터 복원
2. `NovelScene` 초기화
3. 배경/캐릭터/대사/대기 상태 복원
4. `ScriptEngine`을 저장된 위치로 재설정

---

## 9. 저장/복원 규칙

- 저장 중에는 진행을 잠시 멈춘다.
- 불러오기 완료 후, `ScriptEngine`은 복원된 위치부터 재개한다.
- 선택지 대기 중 저장했다면, 복원 후에도 선택지 대기 상태를 유지한다.
- 잘못된 슬롯이나 손상된 파일은 실패로 처리하고 현재 상태를 보존한다.

---

## 10. 오류 처리

- **슬롯 없음**: 저장/불러오기 실패
- **파일 손상**: 복원 실패
- **버전 불일치**: 경고 후 중단
- **필수 필드 누락**: 복원 실패

> 초기 구현에서는 로깅 계층이 제한적이므로, 실패 시 false 반환 중심으로 단순 처리한다.

---

## 11. 검증 항목

- 저장 후 재실행 시 동일 지점으로 복원되는가
- 배경/캐릭터/대사가 저장 시점과 동일한가
- 선택지 대기 중 저장/복원이 가능한가
- 빈 슬롯과 저장된 슬롯이 명확히 구분되는가
- 잘못된 저장 파일이 전체 진행을 망치지 않는가
