// Copyright DCode. All Rights Reserved.

// Corresponding Header
#include "Core/AssetManager.h"

// C++ Standard Library
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>

// Third-party Library
#include <nlohmann/json.hpp>

CVrdxAssetManager* CVrdxAssetManager::Instance = nullptr;

// internal used static functions
namespace
{
	static EVrdxAssetType StringToType(const std::string& Str)
	{
		if (Str == "font")    return EVrdxAssetType::Font;
		if (Str == "texture") return EVrdxAssetType::Texture;
		if (Str == "script")  return EVrdxAssetType::Script;
		return EVrdxAssetType::Font; // fallback
	}

	void JsonToEntry(const nlohmann::json& Json, FVrdxAssetEntry& Entry)
	{
		Entry.Guid = FVrdxString(Json.at("guid").get<std::string>());
		Entry.Type = StringToType(Json.at("type").get<std::string>());

		auto Alias = Json.at("alias");
		Entry.Alias = Alias.is_null() ? "" : Alias.get<std::string>();
		Entry.SourcePath = FVrdxString(Json.at("sourcePath").get<std::string>());
	}
}

CVrdxAssetManager& CVrdxAssetManager::Get()
{
	if (!Instance)
	{
		Instance = new CVrdxAssetManager;
		Instance->InitializeInstance();
	}

	return *Instance;
}

bool CVrdxAssetManager::InitializeInstance()
{
	if (bInitialized)
	{
		return true;
	}

	if (!LoadRegistry())
	{
		return false;
	}

	bInitialized = true;

	return true;
}

void CVrdxAssetManager::Shutdown()
{
	FontCache.clear();
	TextureCache.clear();
	ScriptPathCache.clear();
	Assets.clear();
	GuidIndex.clear();
	AliasToGuid.clear();
	bInitialized = false;
	Instance = nullptr;
}

bool CVrdxAssetManager::LoadRegistry()
{
	try
	{
		std::ifstream File(ResolvePath("AssetRegistry.json").ToUtf8());
		if (File.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
			auto Json = nlohmann::json::parse(content);
			for (const auto& Item : Json["assets"])
			{
				FVrdxAssetEntry Entry;
				JsonToEntry(Item, Entry);
				Assets.push_back(Entry);
			}
		}
	}
	catch (std::exception&)
	{
		// @todo: display log, critical error.
		return false;
	}

	TVrdxVector<size_t> InvalidAssets;

	// Load assets and cache
	for (size_t Index = 0; Index < Assets.size(); ++Index)
	{
		bool bValid = false;

		const auto& Asset = Assets[Index];
		const auto ResolvedPath = ResolvePath(Asset.SourcePath).ToUtf8();

		switch (Asset.Type)
		{
		case EVrdxAssetType::Font:
			{
				if (auto Font = MakeVrdxShared<sf::Font>())
				{
					if (Font->openFromFile(ResolvedPath))
					{
						FontCache.emplace(Asset.Guid, Font);
						bValid = true;
					}
					// @todo: log error
				}
				else
				{
					// @todo: log error
				}
				
				break;
			}
		case EVrdxAssetType::Texture:
			{
				auto Texture = MakeVrdxShared<sf::Texture>();
				if (Texture->loadFromFile(ResolvedPath))
				{
					TextureCache.emplace(Asset.Guid, Texture);
					bValid = true;
				}
				else
				{
					// @todo: log error
				}
				break;
			}
		case EVrdxAssetType::Script:
			{
				ScriptPathCache.emplace(Asset.Guid, ResolvedPath);
				break;
			}
		}
		
		if (bValid)
		{
			GuidIndex.emplace(Asset.Guid, Index);
			if (!Asset.Alias.IsEmpty())
			{
				AliasToGuid.emplace(Asset.Alias, Asset.Guid);
			}
		}
		else
		{
			InvalidAssets.Add(Index);
		}
	}

	for (auto& Index : std::ranges::reverse_view(InvalidAssets))
	{
		Assets.erase(Assets.begin() + Index);
	}

	return true;
}

FVrdxString CVrdxAssetManager::ResolvePath(const FVrdxString& SourcePath) const
{
	return std::string("Assets/") + SourcePath.ToUtf8();
}

bool CVrdxAssetManager::IsGuid(const FVrdxString& Input) const
{
	static const std::regex Pattern(
		R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-4[0-9a-fA-F]{3}-[89abAB][0-9a-fA-F]{3}-[0-9a-fA-F]{12}$)"
	);
	return std::regex_match(Input.ToUtf8(), Pattern) && GuidIndex.contains(Input);
}

const FVrdxAssetEntry* CVrdxAssetManager::GetAssetEntry(const FVrdxString& NameOrGuid) const
{
	const FVrdxString Guid = GetGuid(NameOrGuid);
	if (Guid.IsEmpty())
	{
		return nullptr;
	}

	const size_t AssetIndex = GuidIndex.at(Guid);
	return (0 <= AssetIndex && AssetIndex < Assets.size()) ? &Assets[AssetIndex] : nullptr;
}

FVrdxString CVrdxAssetManager::GetGuid(const FVrdxString& NameOrGuid) const
{
	return IsGuid(NameOrGuid) ? NameOrGuid : GetGuidByAlias(NameOrGuid);
}

TVrdxSharedPtr<sf::Font> CVrdxAssetManager::GetFont(const FVrdxString& NameOrGuid)
{
	const FVrdxString Guid = GetGuid(NameOrGuid);
	if (Guid.IsEmpty())
	{
		return nullptr;
	}

	if (FontCache.contains(Guid))
	{
		return FontCache.at(Guid);
	}

	return nullptr;
}

TVrdxSharedPtr<sf::Texture> CVrdxAssetManager::GetTexture(const FVrdxString& NameOrGuid)
{
	const FVrdxString Guid = GetGuid(NameOrGuid);
	if (Guid.IsEmpty())
	{
		return nullptr;
	}

	if (TextureCache.contains(Guid))
	{
		return TextureCache.at(Guid);
	}

	return nullptr;
}

FVrdxString CVrdxAssetManager::GetScriptPath(const FVrdxString& NameOrGuid) const
{
	const FVrdxString Guid = GetGuid(NameOrGuid);
	if (Guid.IsEmpty())
	{
		return {};
	}

	if (ScriptPathCache.contains(Guid))
	{
		return ScriptPathCache.at(Guid);
	}

	return {};
}

bool CVrdxAssetManager::IsRegistered(const FVrdxString& Guid) const
{
	return GuidIndex.contains(Guid);
}

FVrdxString CVrdxAssetManager::GetGuidByAlias(const FVrdxString& Alias) const
{
	return AliasToGuid.contains(Alias) ? AliasToGuid.at(Alias) : FVrdxString{};
}
