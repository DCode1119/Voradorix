---
title: 네이밍 규칙
project: Voradorix
tags:
  - voradorix
  - convention
  - cpp
---
# Voradorix Naming Convention

> 프로젝트: **Voradorix** (SFML_Project)
> 엔진: SFML 3.1.0 — C++20 — Visual Studio 2022 x64
> 적용일: 2026-06-30

---

## 1. 프로젝트 접두어

모든 클래스, 구조체, 템플릿, 열거형, 인터페이스는 **Vrdx**를 접두어로 사용한다.
파일명 규칙은 Section 4 참조.

```
CVrdxScene         (클래스)
FVrdxSaveSlot      (구조체)
EVrdxSceneResult   (열거형)
IVrdxSerializable  (인터페이스)
TVrdxUniquePtr<T>  (템플릿)
SceneManager.h (파일명)
```

---

## 2. 타입별 접두어 (Unreal Engine 스타일)

| 접두어 | 대상 | 예시 |
|--------|------|------|
| `C` | 일반 클래스 | `CVrdxScene`, `CVrdxSceneManager` |
| `F` | 구조체 / 값 타입 | `FVrdxSaveSlot`, `FVrdxColor`, `FVrdxString` |
| `E` | 열거형 | `EVrdxSceneResult`, `EVrdxLanguage` |
| `I` | 인터페이스 (순수 가상만 있는 클래스) | `IVrdxSerializable` |
| `T` | 템플릿 | `TVrdxUniquePtr<T>`, `TVrdxVector<T>` |
| `b` | bool 변수 접두어 | `bIsRunning`, `bEnabled` |

---

## 3. 변수 / 메서드

| 항목 | 규칙 | 예시 |
|------|------|------|
| 지역 변수 | PascalCase | `float DeltaTime`, `int32_t Index` |
| 멤버 변수 | PascalCase, 접두어 없음 | `float DeltaTime`, `bool bLoaded` |
| bool 변수 | PascalCase + `b` 접두어 | `bool bIsFullscreen` |
| 메서드 | PascalCase | `void OnEnter()`, `int32_t GetCount()` |
| 매개변수 | PascalCase | `const sf::Event& Event` |

- 헝가리안 `m_` 접두어를 사용하지 않는다.
- 언더스코어(`_`)는 사용하지 않는다.
- 약어도 PascalCase로 표기한다 (예: `Fps`, `Json`, `Xml`, `Ui`, `Bgm`, `FadeIn`, `FadeOut`).

---

## 4. 파일명 / 디렉토리

| 항목 | 규칙 | 예시 |
|------|------|------|
| 헤더 파일 | PascalCase | `Scene.h`, `Common.h` |
| 구현 파일 | PascalCase | `Scene.cpp` |
| 디렉토리 | PascalCase | `Core/`, `Scene/`, `Novel/`, `Save/`, `Ui/` |

- 접두어가 없는 단축 파일명을 사용할 수 있다 (예: `Scene.h` → `CVrdxScene` 클래스).
- Common.h, Main.cpp 등 프로젝트 전역 유틸리티는 접두어를 생략한다.

---

## 5. 매크로

| 매크로 | 확장 | 용도 |
|--------|------|------|
| `VRDX_DEFAULT` | `= default` | 기본 생성자/소멸자 |
| `VRDX_PURE_VIRTUAL` | `= 0` | 순수 가상 함수 |
| `VRDX_NO_COPY` | `= delete` | 복사 생성자/대입 금지 |
| `VRDX_NO_DISCARD` | `[[nodiscard]]` | 반환값 무시 경고 |
| `VRDX_OVERRIDE` | `override` | 오버라이드 명시 |
| `VRDX_FINAL` | `final` | 상속 금지 명시 |

매크로는 `Core/Common.h`에 정의되어 있으며, 전환의 자유도를 높이기 위해 사용한다.

---

## 6. 기타 포맷팅 규칙

- 중괄호 스타일: Allman (BSD) — `{`는 새 줄에서 시작
- 들여쓰기: 탭 1개
- `#pragma once` — 모든 헤더에 사용 (include guard 대체)
- `#include` 순서: 표준 라이브러리 → SFML → 프로젝트 헤더
