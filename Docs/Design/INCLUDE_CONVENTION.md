---
title: Include 전처리문 규칙
project: Voradorix
tags:
  - voradorix
  - convention
  - cpp
---
# Voradorix Include Convention

> 프로젝트: **Voradorix** (SFML_Project)
> 엔진: SFML 3.1.0 — C++17 — Visual Studio 2022 x64
> 적용일: 2026-07-06
> 근거: Game.vcxproj.filters 에 정의된 가상 디렉토리(Filter) 구조

---

## 1. 저작권 문구

모든 `.cpp` / `.h` 파일은 **반드시 첫 줄**에 아래 주석으로 시작한다. (`#pragma once` 보다 위)

```cpp
// Copyright DCode. All Rights Reserved.
```

---

## 2. 헤더 파일 (`.h`)

```
// Copyright DCode. All Rights Reserved.
#pragma once

// C++ Standard Library
#include <algorithm>
#include <memory>
#include <string>

// Third-party Library
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/String.hpp>
#include <nlohmann/json.hpp>

// Project Headers
#include "Core/Common.h"
#include "Core/String.h"
#include "Novel/NovelScene.h"
#include "Ui/WidgetBase.h"
```

### 규칙
- `#pragma once` 로 시작한다 (저작권 문구 다음).
- 각 그룹(C++ Standard Library / Third-party Library / Project Headers)은 해당 주석으로 시작한다.
- C++ 표준 라이브러리 헤더를 `<>` 로 include 하며 알파벳 순으로 정렬한다.
- 서드파티 라이브러리 헤더를 `<>` 로 include 하며 알파벳 순으로 정렬한다.
- 프로젝트 내부 헤더를 `""` 로 include 하며 알파벳 순으로 정렬한다.
  - 같은 디렉토리에 있더라도 **항상 filter 경로(`Core/`, `Scene/`, `Ui/`, `Novel/` 등)를 prefix로 붙인다.**
  - 예: 같은 `Novel/` 폴더 내에서 `NovelScene.h` 를 include 할 때도 `#include "Novel/NovelScene.h"` 로 쓴다.
- 각 그룹 사이는 **빈 줄로 구분**한다.

---

## 3. 구현 파일 (`.cpp`)

```
// Copyright DCode. All Rights Reserved.

// Corresponding Header
#include "Novel/NovelScene.h"

// C++ Standard Library
#include <algorithm>
#include <filesystem>

// Third-party Library
#include <SFML/Window/Event.hpp>
#include <nlohmann/json.hpp>

// Project Headers
#include "Core/String.h"
#include "Novel/Background.h"
#include "Novel/ChoiceWidget.h"
#include "Ui/Application.h"
```

### 규칙
- **반드시 이 `.cpp` 파일에 대응되는 헤더가 첫 번째 include**여야 한다.
  - `NovelScene.cpp` → `#include "Novel/NovelScene.h"` (첫 줄)
  - `WidgetBase.cpp` → `#include "Ui/WidgetBase.h"` (첫 줄)
- 나머지 그룹은 헤더 파일과 동일한 규칙을 따른다.
- `#include "Core/Application.h"` 같은 **존재하지 않는 경로를 사용하지 않는다** — 실제 filter 구조와 일치해야 한다.

---

## 4. 그룹 분류 기준

| 그룹 | 내용 | 기호 | 예시 |
|------|------|------|------|
| C++ Standard Library | 표준 라이브러리 전용 | `<>` | `<algorithm>`, `<memory>`, `<string>`, `<vector>` |
| Third-party Library | 서드파티 (SFML, nlohmann 등) | `<>` | `<SFML/Graphics.hpp>`, `<nlohmann/json.hpp>` |
| Project Headers | 프로젝트 자체 헤더 | `""` | `"Ui/WidgetBase.h"`, `"Novel/ScriptEngine.h"` |

### 주의
- Windows SDK 헤더(`<windows.h>`)도 **b** 그룹에 포함한다.
- 서드파티라도 `""` 로만 include 가능한 구조면 **c** 그룹에 넣고 기호는 `""` 를 유지한다.
- 프로젝트 내부 헤더는 **절대 `<>` 를 사용하지 않는다.**

---

## 5. 예외

- `Main.cpp` 는 대응되는 헤더가 없으므로 **a 단계를 생략**하고 바로 b/c/d 로 시작한다.
- `pch.h` / `stdafx.h` 등 precompiled header는 모든 include보다 위에 단독으로 위치시킨다.

---

## 6. 적용 예시

### 헤더 파일 예: `Ui/WidgetBase.h`

```cpp
// Copyright DCode. All Rights Reserved.
#pragma once

// C++ Standard Library
#include <memory>

// Third-party Library
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

// Project Headers
#include "Core/Common.h"
#include "Core/Vector.h"
```

### 구현 파일 예: `Ui/WidgetBase.cpp`

```cpp
// Copyright DCode. All Rights Reserved.

// Corresponding Header
#include "Ui/WidgetBase.h"

// C++ Standard Library
#include <algorithm>
#include <ranges>
