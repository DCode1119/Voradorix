---
title: "AssetManager 명세"
project: Voradorix
phase: "Phase 1"
status: 구현 완료
tags:
  - voradorix
  - engine
  - asset-manager
  - spec
  - cpp
---

# Voradorix — AssetManager 명세

> **대상**: 에셋 로딩/캐싱/수명 관리를 전담하는 싱글톤 코어 모듈  
> **의존성**: nlohmann/json (Registry I/O), SFML (sf::Font, sf::Texture)  
> **영향**: DialogueBox, TextLabel, ChoiceWidget (1순위), Background, CharacterManager (2순위)  
> **상태**: 구현 완료 (`AssetManager.h/cpp` Phase 1.0), UI 폰트 및 배경/캐릭터 텍스처 경로 이관 완료

---

## 1. 개요

현재 엔진은 AssetManager를 통해 폰트/텍스처/스크립트 경로를 중앙 관리합니다. 초기에는 DialogueBox/TextLabel/ChoiceWidget의 폰트 중복 로딩 문제를 해결하는 것이 1차 목표였고, 이후 배경/캐릭터 텍스처와 스크립트 경로까지 AssetManager로 이관되었습니다. AssetManager는 모든 에셋 로딩을 중앙에서 관리하여:

- **중복 로딩 제거** — 동일 에셋을 여러 위젯이 공유
- **식별 체계 통일** — GUID + Alias 이중 접근
- **Registry 기반** — `AssetRegistry.json`에 에셋 목록을 저장/조회
- **Pre-load** — `InitializeInstance()` 시점에 모든 에셋을 미리 로드하여 캐싱

---

## 2. 클래스 설계

### 2.1 헤더 (`AssetManager.h`)

```cpp
struct FVrdxAssetEntry
{
    FVrdxString     Guid;
    EVrdxAssetType  Type;
    FVrdxString     Alias;          // null 가능
    FVrdxString     SourcePath;     // Assets/ 기준 상대 경로
};

class CVrdxAssetManager
{
public:
    // ── 생명주기 ──────────────────────────────────────────
    static CVrdxAssetManager& Get();                    // 싱글톤 접근자
    bool InitializeInstance();                          // Registry 로드 + pre-load
    void Shutdown();                                    // 캐시 정리

    // ── Font ──────────────────────────────────────────────
    TVrdxSharedPtr<sf::Font> GetFont(const FVrdxString& NameOrGuid);

    // ── Texture ───────────────────────────────────────────
    TVrdxSharedPtr<sf::Texture> GetTexture(const FVrdxString& NameOrGuid);

    // ── Script ───────────────────────────────────────────
    FVrdxString GetScriptPath(const FVrdxString& NameOrGuid) const;

    // ── Asset Registry ────────────────────────────────────
    bool IsRegistered(const FVrdxString& Guid) const;
    FVrdxString GetGuidByAlias(const FVrdxString& Alias) const;

private:
    // ── 비공개 ────────────────────────────────────────────
    bool LoadRegistry();                                // JSON → 내부 구조
    FVrdxString ResolvePath(const FVrdxString& SourcePath) const;
    bool IsGuid(const FVrdxString& Input) const;        // UUID v4 형식 + 등록 여부
    const FVrdxAssetEntry* GetAssetEntry(const FVrdxString& NameOrGuid) const;
    FVrdxString GetGuid(const FVrdxString& NameOrGuid) const;

    // 캐시
    std::unordered_map<FVrdxString, TVrdxSharedPtr<sf::Font>>    FontCache;      // Guid → Font
    std::unordered_map<FVrdxString, TVrdxSharedPtr<sf::Texture>> TextureCache;   // Guid → Texture
    std::unordered_map<FVrdxString, FVrdxString>                 ScriptPathCache;// Guid → 경로

    // Registry
    std::vector<FVrdxAssetEntry>                Assets;
    std::unordered_map<FVrdxString, size_t>     GuidIndex;      // Guid → Assets index
    std::unordered_map<FVrdxString, FVrdxString> AliasToGuid;   // Alias → Guid

    static CVrdxAssetManager* Instance;
    bool bInitialized = false;
};
```

### 2.2 에셋 타입 열거형 (`Common.h`)

```cpp
enum class EVrdxAssetType : uint8_t
{
    Texture = 0,
    Font,
    Script,
    // Audio, WidgetBlueprint ... (확장 가능)
    Count
};
```

---

## 3. 동작 흐름

### 3.1 초기화 (`InitializeInstance`)

```
CVrdxApplication::Initialize() 호출
  → CVrdxAssetManager::Get().InitializeInstance()
    → Assets/AssetRegistry.json 읽기
    → JSON 파싱
     → 각 에셋별 pre-load 및 캐시 구축
       · Font   → openFromFile() → FontCache 저장
      · Texture → loadFromFile() → TextureCache 저장
      · Script  → ScriptPathCache 저장
    → 로드 성공한 에셋만 GuidIndex / AliasToGuid 구축
    → 로드 실패한 에셋은 Assets에서 제거
    → bInitialized = true
```

### 3.2 GetFont (`GetFont("malgun")`)

```
① GetGuid(NameOrGuid)로 GUID 획득
   · IsGuid() — UUID v4 정규식 일치 AND GuidIndex.contains()
     → true  = 입력 자체가 GUID → 그대로 사용
     → false = Alias로 간주 → AliasToGuid 조회
   → GUID를 찾지 못하면 nullptr 반환

② FontCache에서 GUID로 조회
    → 캐시에서 조회 후 shared_ptr 반환
    → 없으면 nullptr 반환 (정상 초기화라면 발생하지 않음)
```

### 3.3 GetTexture (`GetTexture("bg_room")`)

```
① GetGuid(NameOrGuid)로 GUID 획득 (GetFont와 동일)
② TextureCache에서 GUID로 조회
    → 캐시에서 조회 후 shared_ptr 반환
    → 없으면 nullptr 반환 (정상 초기화라면 발생하지 않음)
```

### 3.4 GetScriptPath (`GetScriptPath("script_intro")`)

```
① GetGuid(NameOrGuid)로 GUID 획득
② ScriptPathCache에서 GUID로 조회
    → 있으면 경로(FVrdxString) 반환
    → 없으면 빈 문자열 반환
```

### 3.5 NovelScene 전환

- `Background`는 자체 파일 경로 로딩 대신 `GetTexture()`를 사용
- `CharacterManager`는 자체 텍스처 캐시를 제거하고 `GetTexture()`를 직접 사용
- `ResetScriptEngine()`는 스크립트만 다시 읽는 것이 아니라 `NovelScene::Reset()`을 통해 배경/캐릭터/대사/선택지 상태도 초기화함

### 3.6 종료 (`Shutdown`)

```
CVrdxApplication 소멸 또는 종료 시
  → 모든 캐시 clear (FontCache, TextureCache, ScriptPathCache)
  → Assets / GuidIndex / AliasToGuid clear
  → bInitialized = false
```

### 3.7 내부 유틸리티

```cpp
// ── EVrdxAssetType ↔ std::string ─────────────────────────────
static EVrdxAssetType StringToType(const std::string& Str)
{
    if (Str == "font")    return EVrdxAssetType::Font;
    if (Str == "texture") return EVrdxAssetType::Texture;
    if (Str == "script")  return EVrdxAssetType::Script;
    return EVrdxAssetType::Font; // fallback
}
```

```cpp
// ── JSON → FVrdxAssetEntry ───────────────────────────────────
void JsonToEntry(const nlohmann::json& Json, FVrdxAssetEntry& Entry)
{
    Entry.Guid       = FVrdxString(Json.at("guid").get<std::string>());
    Entry.Type       = StringToType(Json.at("type").get<std::string>());
    Entry.Alias      = Json.value("alias", std::string());
    Entry.SourcePath = FVrdxString(Json.at("sourcePath").get<std::string>());
}
```

```cpp
// ── GUID 검증 ────────────────────────────────────────────────
bool CVrdxAssetManager::IsGuid(const FVrdxString& Input) const
{
    static const std::regex Pattern(
        R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-4[0-9a-fA-F]{3}-[89abAB][0-9a-fA-F]{3}-[0-9a-fA-F]{12}$)"
    );
    // 형식 일치 AND 등록된 GUID여야 함
    return std::regex_match(Input.ToUtf8(), Pattern) && GuidIndex.contains(Input);
}
```

---

## 4. Registry JSON 스키마

```json
{
  "version": 1,
  "assets": [
    {
      "guid": "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
      "type": "font",
      "alias": "malgun",
      "sourcePath": "Fonts/malgun.ttf"
    },
    {
      "guid": "fedcba09-8765-4321-abcd-ef1234567890",
      "type": "texture",
      "alias": null,
      "sourcePath": "Images/WhiteRoom.png"
    }
  ],
  "aliases": {
    "malgun": "a1b2c3d4-e5f6-7890-abcd-ef1234567890"
  }
}
```

- `sourcePath`는 `Assets/` 기준 **상대 경로**
- `aliases` 맵은 Editor가 작성하는 인덱스 (`alias → guid`)
- 엔진은 `aliases` 맵을 읽지 않고, 자체 `AliasToGuid` 맵을 구축함

---

## 5. 파일 구조

```
Game/Src/Core/
├── Common.h           → EVrdxAssetType 열거형 추가
├── AssetManager.h     → CVrdxAssetManager 클래스 선언
├── AssetManager.cpp   → 구현
├── String.h           → FVrdxString

Assets/
└── AssetRegistry.json → 중앙 레지스트리 (Editor와 공유)
```

---

## 6. 상태

### Phase 1.0 — 기본 골격 ✅ 구현 완료

| 항목 | 내용 |
|------|------|
| 싱글톤 | `Get()`, `InitializeInstance()`, `Shutdown()` |
| Registry I/O | `LoadRegistry()` — JSON 파싱 + `InitializeInstance()` 시 pre-load |
| GUID 검증 | `IsGuid()` — 정규식 + 등록 확인 |
| GetFont | GUID/Alias → FontCache 조회 |
| GetTexture | GUID/Alias → TextureCache 조회 |
| GetScriptPath | GUID/Alias → ScriptPathCache 조회 |
| EVrdxAssetType | `Common.h`에 `Font`, `Texture`, `Script` 정의 |

### Phase 1.1 — 기존 코드 전환 (부분 완료)

| 항목 | 내용 |
|------|------|
| DialogueBox 리팩터 | `GetFont()`로 교체 ✅ |
| TextLabel 리팩터 | `GetFont()`로 교체 ✅ |
| ChoiceWidget 리팩터 | `GetFont()`로 교체 ✅ |
| Background 리팩터 | `GetTexture()`로 교체 ✅ |
| CharacterManager 리팩터 | 에셋 로딩을 AssetManager로 이관 ✅ |
| Script 경로 조회 | `GetScriptPath()` 추가 ✅ |

---

## 7. 에러 처리

| 상황 | 처리 |
|------|------|
| Registry JSON 없음 | 빈 상태로 시작, false 반환 |
| JSON 파싱 실패 | false 반환 (catch) |
| Font 파일 로드 실패 | 해당 에셋 제외, 나머지 에셋은 정상 로드 |
| Texture 파일 로드 실패 | 해당 에셋 제외, 나머지 에셋은 정상 로드 |
| 존재하지 않는 Alias/GUID 조회 | `GetFont`/`GetTexture` → nullptr |
| `GetScriptPath` 실패 | 빈 `FVrdxString` 반환 |
