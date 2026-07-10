// Copyright DCode. All Rights Reserved.
#pragma once

// C++ Standard Library
#include <unordered_map>

// Third-party Library
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

// Project Headers
#include "Core/Common.h"
#include "Core/String.h"

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
	bool InitializeInstance();                          // Registry 로드, 초기화
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
	bool IsGuid(const FVrdxString& Input) const;
	const FVrdxAssetEntry* GetAssetEntry(const FVrdxString& NameOrGuid) const;
	FVrdxString GetGuid(const FVrdxString& NameOrGuid) const;

	// 캐시
	std::unordered_map<FVrdxString, TVrdxSharedPtr<sf::Font>>    FontCache;    // Guid → Font
	std::unordered_map<FVrdxString, TVrdxSharedPtr<sf::Texture>> TextureCache; // Guid → Texture
	std::unordered_map<FVrdxString, FVrdxString>                 ScriptPathCache;

	// Registry
	std::vector<FVrdxAssetEntry>        Assets;
	std::unordered_map<FVrdxString, size_t> GuidIndex;  // Guid → Assets index
	std::unordered_map<FVrdxString, FVrdxString> AliasToGuid;  // Alias → Guid

	static CVrdxAssetManager* Instance;
	bool bInitialized = false;
};
