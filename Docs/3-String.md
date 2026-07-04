---
title: "유니코드 문자열 클래스"
project: Voradorix
phase: 2
status: 완료
tags:
  - voradorix
  - string
  - unicode
  - spec
  - cpp
---
# Voradorix — 유니코드 문자열 클래스 명세

> **대상**: `FVrdxString` (UTF-32 기반 유니코드 문자열 클래스)  
> **프로젝트**: Voradorix  
> **관련 파일**: `Src/Core/String.h` (신규), `Src/Core/Common.h` (FString alias 갱신)  
> **의존성**: SFML 3.1.0 (`sf::String`), C++17 (`std::u32string`, `char8_t` 변환)  
> **완료 조건**: 코드 포인트 단위 길이/인덱싱/부분문자열, SFML `sf::String` 변환, UTF-8 입출력

---

## 1. 배경

현재 `FString`은 `std::string`(UTF-8) alias로만 존재하며, 다음 문제가 있다:

- 타이핑 애니메이션: `std::string`의 `length()`는 바이트 수라서 한글 등 멀티바이트 문자에서 실제 글자 수와 불일치
- SFML 연동: `sf::Text::setString()`에 `std::string`을 넘기면 내부에서 UTF-32 변환이 필요함
- 향후 ScriptEngine에서 UTF-8 파일을 파싱할 때 코드 포인트 단위 처리가 필요

해결책: 내부 저장을 **UTF-32 (`std::u32string`)** 로 통일하고, UTF-8 입출력 인터페이스를 제공한다.

---

## 2. 파일 구성

| # | 파일 | 설명 | 상태 |
|---|------|------|------|
| 1 | `Src/Core/String.h` | FVrdxString 클래스 선언 + 구현 | 📝 작성 예정 |
| 2 | `Src/Core/Common.h` | `using FString = FVrdxString` alias 추가 | 📝 갱신 예정 |
| 3 | `Game.vcxproj` | ClInclude 항목 등록 | 📝 등록 예정 |

- **header-only** 설계 — 별도 `.cpp` 없음 (구현이 간결하므로 인라인 처리)

---

## 3. 네이밍

| 구분 | 규칙 | 예시 |
|------|------|------|
| 클래스 | `F` + `Vrdx` (핵심 데이터 타입이므로 `F` 접두어) | `FVrdxString` |
| public alias | `FString` | `using FString = FVrdxString;` |
| 파일명 | PascalCase | `String.h` |

`FString`은 Unreal Engine에서도 널리 쓰이는 문자열 타입 이름으로,
`Common.h`에서 `using FString = FVrdxString;` alias를 제공한다.

---

## 4. FVrdxString 클래스 설계

### 4.1 내부 저장

```cpp
class FVrdxString
{
private:
    std::u32string Buffer;   // UTF-32 LE 코드 포인트 열
};
```

- 모든 문자는 **1 code point = 1 요소** (한글, CJK, 이모지 포함)
- `std::u32string::size()`가 곧 실제 글자 수

### 4.2 생성 / 대입

```cpp
public:
    FVrdxString() VRDX_DEFAULT;
    FVrdxString(const FVrdxString&) VRDX_DEFAULT;
    FVrdxString& operator=(const FVrdxString&) VRDX_DEFAULT;
    FVrdxString(FVrdxString&&) VRDX_DEFAULT;
    FVrdxString& operator=(FVrdxString&&) VRDX_DEFAULT;

    // UTF-8 → UTF-32 변환
    explicit FVrdxString(const char* Utf8Str);
    explicit FVrdxString(const std::string& Utf8Str);

    // SFML sf::String → 변환
    explicit FVrdxString(const sf::String& SfStr);
```

- `explicit` 생성자로 암시적 변환 방지
- UTF-8 바이트 시퀀스는 생성 시 UTF-32로 디코딩
- 잘못된 UTF-8 시퀀스는 `U+FFFD` (REPLACEMENT CHARACTER)로 대체

### 4.3 UTF-8 출력

```cpp
    // UTF-32 → UTF-8 변환
    std::string ToUtf8() const;
    const char* ToCStr() const;  // null-terminated UTF-8 (내부 캐시)
```

- `ToUtf8()`: UTF-32 버퍼를 UTF-8로 인코딩하여 `std::string` 반환
- `ToCStr()`: 동일하나 내부에 캐싱된 `std::string`의 `c_str()` 반환

### 4.4 SFML 연동

```cpp
    // SFML sf::String으로 변환 (std::basic_string<sf::Uint32> 기반)
    sf::String ToSfString() const;
```

- `std::u32string`과 `sf::String`은 모두 `char32_t`/`sf::Uint32` 기반이므로 복사만으로 충분
- `sf::Text::setString()`에 직접 전달 가능

### 4.5 코드 포인트 단위 연산

```cpp
    size_t Length() const;      // 코드 포인트 수 (= Buffer.size())
    bool IsEmpty() const;       // Buffer.empty()

    // 앞에서 N글자만큼의 부분 문자열 (타이핑 애니메이션)
    FVrdxString Left(const size_t Count) const;
    // Count 위치부터 끝까지
    FVrdxString Right(const size_t Offset) const;
    // Offset부터 Count글자
    FVrdxString Substr(const size_t Offset, const size_t Count = npos) const;

    // 인덱스 접근 (읽기 전용)
    char32_t operator[](const size_t Index) const;
    char32_t CodePointAt(const size_t Index) const;

    char32_t Front() const;    // 첫 번째 코드 포인트
    char32_t Back() const;     // 마지막 코드 포인트
```

- `Length()`는 `Buffer.size()`를 반환 — 바이트 수가 아닌 실제 문자 수
- `Left(N)`: 타이핑 애니메이션에서 `VisibleCount`만큼의 부분 문자열을 구할 때 사용

### 4.6 편의 연산자

```cpp
    // 연결
    FVrdxString operator+(const FVrdxString& Other) const;
    FVrdxString& operator+=(const FVrdxString& Other);
    FVrdxString& operator+=(const char32_t CodePoint);

    // 비교
    bool operator==(const FVrdxString& Other) const;
    bool operator!=(const FVrdxString& Other) const;
    bool operator<(const FVrdxString& Other) const;

    // 빈 문자열 상수
    static const FVrdxString Empty;
```

### 4.7 상수

```cpp
    static constexpr size_t npos = static_cast<size_t>(-1);
```

---

## 5. UTF-8 변환 상세

### 5.1 UTF-8 → UTF-32 (생성자)

입력 바이트 시퀀스를 표준 UTF-8 디코딩 규칙으로 변환:

| 바이트 범위 | 의미 | 코드 포인트 |
|------------|------|-----------|
| `0xxxxxxx` | 1바이트 ASCII | `U+0000` ~ `U+007F` |
| `110xxxxx 10xxxxxx` | 2바이트 | `U+0080` ~ `U+07FF` |
| `1110xxxx 10xxxxxx 10xxxxxx` | 3바이트 | `U+0800` ~ `U+FFFF` (한글 포함) |
| `11110xxx 10xxxxxx 10xxxxxx 10xxxxxx` | 4바이트 | `U+10000` ~ `U+10FFFF` (이모지 등) |

- 잘못된 바이트 시퀀스: `U+FFFD` (REPLACEMENT CHARACTER)로 대체 후 계속 진행
- 오버롱(overlong) 인코딩: 무효 처리

### 5.2 UTF-32 → UTF-8 (`ToUtf8()`)

위 표의 역변환. 1~4바이트 가변 길이 인코딩.

---

## 6. 사용 예시 (DialogueBox 연동)

```cpp
// 타이핑 애니메이션 — Left() 사용
void CVrdxDialogueBox::Update(const float DeltaTick)
{
    if (!bIsTyping) return;

    TypeTimer += DeltaTick;
    while (TypeTimer >= TypeInterval && VisibleCount < FullText.Length())
    {
        VisibleCount++;
        TypeTimer -= TypeInterval;
    }

    // 앞에서 VisibleCount글자만 표시
    sf::String DisplayText = FullText.Left(VisibleCount).ToSfString();
    LineText.setString(DisplayText);

    if (VisibleCount >= FullText.Length())
        bIsTyping = false;
}
```

---

## 7. Common.h 변경

```cpp
// Common.h 하단에 추가
#include "Core/String.h"

using FString = FVrdxString;
```

기존 코드에서 `FString`을 그대로 사용할 수 있고, 기존의 `std::string` 사용처는 `::std::string`으로 명시하거나 필요시 `using FString`만 변경하면 된다.

---

## 8. 의존성 그래프

```
String.h
├── <string>         (std::string, std::u32string)
├── <cstdint>        (uint8_t 등 UTF-8 디코딩용)
└── SFML/System/String.hpp   (sf::String)

Common.h
└── Core/String.h    (using FString = FVrdxString)
```

---

## 9. .vcxproj 등록

### ClInclude (신규 등록)

| 파일 | 설명 |
|------|------|
| `Src\Core\String.h` | FVrdxString 선언/구현 |

`Common.h`가 `#include "Core/String.h"`를 추가하므로,
`Common.h`를 포함하는 모든 파일이 `FVrdxString`에 접근 가능.

---

## 10. 컴파일 확인 사항

- `<cstdint>`, `<cstddef>` 필요 (UTF-8 변환 구현)
- `sf::String` 사용 → `sfml-system-d.lib` 링크 필요 (이미 sfml-graphics-d.lib 통해 간접 링크)
- header-only 설계이므로 링크 이슈 없음
- `inline` 함수는 `.h`에 직접 구현

---

## 11. 검증 방법

1. **빌드**: `Core/String.h` include만으로 컴파일 성공
2. **단위 동작** (TestScene 또는 임시 코드로 확인):
   | 입력 | 기대 결과 |
   |------|----------|
   | `FVrdxString("Hello")` | `Length() == 5` |
   | `FVrdxString("안녕")` | `Length() == 2` (바이트가 아닌 글자 수) |
   | `Left(1)` on `"안녕"` | `"안"` |
   | `ToSfString()` 결과를 `sf::Text::setString()`에 전달 | 정상 렌더링 |
3. **회귀**: 기존 `std::string` → `FString`으로 교체 후 빌드 정상 확인
