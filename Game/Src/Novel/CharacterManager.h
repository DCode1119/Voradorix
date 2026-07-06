// Copyright DCode. All Rights Reserved.
#pragma once

// C++ Standard Library
#include <string>
#include <unordered_map>

// Third-party Library
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

// Project Headers
#include "Core/Common.h"
#include "Core/String.h"
#include "Core/Vector.h"
#include "Ui/WidgetBase.h"

enum class EVrdxCharacterPosition : uint8_t
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
	bool bPendingPoseChange = false;
	FVrdxString PendingPoseName;
	TVrdxSharedPtr<sf::Texture> PendingTexture;
	TVrdxSharedPtr<sf::Texture> Texture;
	sf::Sprite Sprite;
};

struct FVrdxCharacterSlotSaveData
{
	EVrdxCharacterPosition Position;
	FVrdxString CharacterName;
	FVrdxString CharacterPose;
};

class CVrdxCharacterManager : public CVrdxWidgetBase
{
public:
	CVrdxCharacterManager(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	~CVrdxCharacterManager() VRDX_DEFAULT;

	void ShowCharacter(const FVrdxString& CharacterName, EVrdxCharacterPosition Position, const FVrdxString& PoseName = "Normal");
	void SetCharacterPose(const FVrdxString& CharacterName, const FVrdxString& PoseName, float FadeSeconds = 0.25f);
	void HideCharacter(const FVrdxString& CharacterName, float FadeSeconds = 0.25f);
	void ClearSlot(EVrdxCharacterPosition Slot, float FadeSeconds = 0.25f);

	void Update(float DeltaTick);
	void Draw(sf::RenderWindow& Window) const;

	TVrdxVector<FVrdxCharacterSlotSaveData> GetSaveData() const;

	void Reset();

private:
	static FVrdxString MakeTextureKey(const FVrdxString& CharacterName, const FVrdxString& PoseName);
	static FVrdxString MakeTexturePath(const FVrdxString& CharacterName, const FVrdxString& PoseName);
	static float GetSlotX(EVrdxCharacterPosition Slot);
	static sf::Texture CreateTransparentTexture();

	FVrdxCharacterSlot* FindSlotByPosition(const EVrdxCharacterPosition Slot);
	const FVrdxCharacterSlot* FindSlotByPosition(const EVrdxCharacterPosition Slot) const;
	FVrdxCharacterSlot* FindSlotByCharacter(const FVrdxString& CharacterName);
	const FVrdxCharacterSlot* FindSlotByCharacter(const FVrdxString& CharacterName) const;

	const TVrdxSharedPtr<sf::Texture>& ResolveTexture(const FVrdxString& CharacterName, const FVrdxString& PoseName);
	void ResetSlot(FVrdxCharacterSlot& SlotState);
	void BeginFade(FVrdxCharacterSlot& SlotState, float TargetAlpha, float FadeSeconds);
	void UpdateSlotTransform(FVrdxCharacterSlot& SlotState);
	void SetSpriteAlpha(sf::Sprite& Sprite, float Alpha) const;

	TVrdxSharedPtr<sf::Texture> TransparentTexture;
	std::unordered_map<FVrdxString, TVrdxSharedPtr<sf::Texture>> TextureCache;
	TVrdxVector<FVrdxCharacterSlot> Slots;
};
