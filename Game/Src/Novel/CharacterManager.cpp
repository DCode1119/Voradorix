#include "Novel/CharacterManager.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <SFML/Graphics/Color.hpp>

namespace
{
	constexpr float LeftSlotX = 320.f;
	constexpr float CenterSlotX = 640.f;
	constexpr float RightSlotX = 960.f;
	constexpr float SlotAnchorY = 660.f;
	constexpr float MaxCharacterHeight = 1200.f;
}

FVrdxCharacterSlot::FVrdxCharacterSlot(const EVrdxCharacterPosition InSlot, const sf::Texture& TransparentTexture)
	: Slot(InSlot)
	, Texture(std::make_shared<sf::Texture>(TransparentTexture))
	, Sprite(*Texture)
{
}

CVrdxCharacterManager::CVrdxCharacterManager()
	: TransparentTexture(std::make_shared<sf::Texture>(CreateTransparentTexture()))
{
	Slots.Reserve(3);
	Slots.Add(FVrdxCharacterSlot(EVrdxCharacterPosition::Left, *TransparentTexture));
	Slots.Add(FVrdxCharacterSlot(EVrdxCharacterPosition::Center, *TransparentTexture));
	Slots.Add(FVrdxCharacterSlot(EVrdxCharacterPosition::Right, *TransparentTexture));
}

void CVrdxCharacterManager::ShowCharacter(const FVrdxString& CharacterName, EVrdxCharacterPosition Position, const FVrdxString& PoseName /*= "Normal"*/)
{
	FVrdxCharacterSlot* SlotState = FindSlotByPosition(Position);
	if (SlotState == nullptr)
	{
		return;
	}

	SlotState->CharacterName = CharacterName;
	SlotState->PoseName = PoseName;
	SlotState->Texture = ResolveTexture(CharacterName, PoseName);
	SlotState->Sprite.setTexture(*SlotState->Texture, true);
	UpdateSlotTransform(*SlotState);

	SlotState->Alpha = 1.0f;
	SlotState->StartAlpha = 1.0f;
	SlotState->TargetAlpha = 1.0f;
	SlotState->FadeTime = 0.0f;
	SlotState->FadeDuration = 0.0f;
	SlotState->bPendingPoseChange = false;
	SlotState->PendingPoseName = FVrdxString::Empty;
	SlotState->PendingTexture = nullptr;
	SlotState->bVisible = true;
	SetSpriteAlpha(SlotState->Sprite, 255.0f);
}


void CVrdxCharacterManager::SetCharacterPose(const FVrdxString& CharacterName, const FVrdxString& PoseName, const float FadeSeconds)
{
	FVrdxCharacterSlot* SlotState = FindSlotByCharacter(CharacterName);
	if (SlotState == nullptr)
	{
		return;
	}

	if (SlotState->PoseName == PoseName)
	{
		return;
	}

	if (FadeSeconds <= 0.0f)
	{
		SlotState->PoseName = PoseName;
		SlotState->Texture = ResolveTexture(CharacterName, PoseName);
		SlotState->Sprite.setTexture(*SlotState->Texture, true);
		UpdateSlotTransform(*SlotState);
		SlotState->Alpha = 1.0f;
		SlotState->StartAlpha = 1.0f;
		SlotState->TargetAlpha = 1.0f;
		SlotState->FadeTime = 0.0f;
		SlotState->FadeDuration = 0.0f;
		SlotState->bPendingPoseChange = false;
		SlotState->PendingPoseName = FVrdxString::Empty;
		SlotState->PendingTexture = nullptr;
		SlotState->bVisible = true;
		SetSpriteAlpha(SlotState->Sprite, 255.0f);
		return;
	}

	SlotState->bPendingPoseChange = true;
	SlotState->PendingPoseName = PoseName;
	SlotState->PendingTexture = ResolveTexture(CharacterName, PoseName);
	BeginFade(*SlotState, 0.0f, FadeSeconds * 0.5f);

}

void CVrdxCharacterManager::HideCharacter(const FVrdxString& CharacterName, const float FadeSeconds)
{
	FVrdxCharacterSlot* SlotState = FindSlotByCharacter(CharacterName);
	if (SlotState == nullptr)
	{
		return;
	}

	SlotState->bPendingPoseChange = false;
	SlotState->PendingPoseName = FVrdxString::Empty;
	SlotState->PendingTexture = nullptr;
	BeginFade(*SlotState, 0.0f, FadeSeconds);
}

void CVrdxCharacterManager::ClearSlot(const EVrdxCharacterPosition Slot, const float FadeSeconds)
{
	FVrdxCharacterSlot* SlotState = FindSlotByPosition(Slot);
	if (SlotState == nullptr)
	{
		return;
	}

	SlotState->bPendingPoseChange = false;
	SlotState->PendingPoseName = FVrdxString::Empty;
	SlotState->PendingTexture = nullptr;
	BeginFade(*SlotState, 0.0f, FadeSeconds);
}

void CVrdxCharacterManager::Update(const float DeltaTick)
{
	for (int32_t Index = 0; Index < Slots.Num(); ++Index)
	{
		FVrdxCharacterSlot& SlotState = Slots[Index];
		if (SlotState.FadeDuration <= 0.0f)
		{
			continue;
		}

		SlotState.FadeTime = std::min(SlotState.FadeTime + DeltaTick, SlotState.FadeDuration);
		const float Progress = (SlotState.FadeDuration > 0.0f) ? (SlotState.FadeTime / SlotState.FadeDuration) : 1.0f;
		SlotState.Alpha = SlotState.StartAlpha + ((SlotState.TargetAlpha - SlotState.StartAlpha) * Progress);
		SlotState.Alpha = std::clamp(SlotState.Alpha, 0.0f, 1.0f);
		SetSpriteAlpha(SlotState.Sprite, SlotState.Alpha * 255.0f);

		if (SlotState.FadeTime < SlotState.FadeDuration)
		{
			continue;
		}

		const bool bWasFadingOutForPoseChange = SlotState.bPendingPoseChange && SlotState.TargetAlpha <= 0.0f;
		SlotState.FadeDuration = 0.0f;
		SlotState.FadeTime = 0.0f;

		if (bWasFadingOutForPoseChange)
		{
			SlotState.PoseName = SlotState.PendingPoseName;
			SlotState.Texture = SlotState.PendingTexture;
			SlotState.Sprite.setTexture(*SlotState.Texture, true);
			UpdateSlotTransform(SlotState);
			SlotState.PendingPoseName = FVrdxString::Empty;
			SlotState.PendingTexture = nullptr;
			SlotState.bPendingPoseChange = false;
			BeginFade(SlotState, 1.0f, 0.0f);
			continue;
		}

		if (SlotState.TargetAlpha <= 0.0f)
		{
			ResetSlot(SlotState);
		}
	}
}

void CVrdxCharacterManager::Draw(sf::RenderWindow& Window) const
{
	for (int32_t Index = 0; Index < Slots.Num(); ++Index)
	{
		const FVrdxCharacterSlot& SlotState = Slots[Index];
		if (!SlotState.bVisible && SlotState.Alpha <= 0.0f)
		{
			continue;
		}

		sf::Sprite DrawSprite = SlotState.Sprite;
		const auto Alpha = static_cast<std::uint8_t>(std::clamp(SlotState.Alpha * 255.0f, 0.0f, 255.0f));
		DrawSprite.setColor(sf::Color(255, 255, 255, Alpha));
		Window.draw(DrawSprite);
	}
}


TVrdxVector<FVrdxCharacterSlotSaveData> CVrdxCharacterManager::GetSaveData() const
{
	TVrdxVector<FVrdxCharacterSlotSaveData> Result;
	
	const static EVrdxCharacterPosition Positions[] =
	{
		EVrdxCharacterPosition::Left,
		EVrdxCharacterPosition::Right,
		EVrdxCharacterPosition::Center,
	};

	for (const auto Position : Positions)
	{
		if (auto Slot = FindSlotByPosition(Position))
		{
			Result.Add({ .Position = Slot->Slot, .CharacterName = Slot->CharacterName, .CharacterPose = Slot->PoseName });
		}
	}

	return Result;
}


void CVrdxCharacterManager::Reset()
{
	const static EVrdxCharacterPosition Positions[] =
	{
		EVrdxCharacterPosition::Left,
		EVrdxCharacterPosition::Right,
		EVrdxCharacterPosition::Center,
	};

	for (const auto Position : Positions)
	{
		ClearSlot(Position);
	}
}

const FVrdxCharacterSlot* CVrdxCharacterManager::FindSlotByPosition(const EVrdxCharacterPosition Slot) const
{
	for (int32_t Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index].Slot == Slot)
		{
			return &Slots[Index];
		}
	}

	return nullptr;
}


FVrdxCharacterSlot* CVrdxCharacterManager::FindSlotByPosition(const EVrdxCharacterPosition Slot)
{
	for (int32_t Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index].Slot == Slot)
		{
			return &Slots[Index];
		}
	}

	return nullptr;
}

const FVrdxCharacterSlot* CVrdxCharacterManager::FindSlotByCharacter(const FVrdxString& CharacterName) const
{
	for (int32_t Index = 0; Index < Slots.Num(); ++Index)
	{
		const FVrdxCharacterSlot& SlotState = Slots[Index];
		if (SlotState.bVisible && SlotState.CharacterName == CharacterName)
		{
			return &SlotState;
		}
	}

	return nullptr;
}


FVrdxCharacterSlot* CVrdxCharacterManager::FindSlotByCharacter(const FVrdxString& CharacterName)
{
	for (int32_t Index = 0; Index < Slots.Num(); ++Index)
	{
		FVrdxCharacterSlot& SlotState = Slots[Index];
		if (SlotState.bVisible && SlotState.CharacterName == CharacterName)
		{
			return &SlotState;
		}
	}

	return nullptr;
}

const TVrdxSharedPtr<sf::Texture>& CVrdxCharacterManager::ResolveTexture(const FVrdxString& CharacterName, const FVrdxString& PoseName)
{
	const FVrdxString Key = MakeTextureKey(CharacterName, PoseName);
	if (const auto It = TextureCache.find(Key); It != TextureCache.end())
	{
		return It->second;
	}

	TVrdxSharedPtr<sf::Texture> LoadedTexture = std::make_shared<sf::Texture>();
	if (!LoadedTexture->loadFromFile(MakeTexturePath(CharacterName, PoseName).ToUtf8()))
	{
		LoadedTexture = TransparentTexture;
	}
	else
	{
		LoadedTexture->setSmooth(true);
	}

	auto [It, _] = TextureCache.emplace(Key, LoadedTexture);
	return It->second;
}

void CVrdxCharacterManager::ResetSlot(FVrdxCharacterSlot& SlotState)
{
	SlotState.CharacterName = FVrdxString::Empty;
	SlotState.PoseName = FVrdxString::Empty;
	SlotState.PendingPoseName = FVrdxString::Empty;
	SlotState.Texture = TransparentTexture;
	SlotState.PendingTexture = nullptr;
	SlotState.Sprite.setTexture(*SlotState.Texture, true);
	UpdateSlotTransform(SlotState);
	SetSpriteAlpha(SlotState.Sprite, 0.0f);
	SlotState.Alpha = 0.0f;
	SlotState.StartAlpha = 0.0f;
	SlotState.TargetAlpha = 0.0f;
	SlotState.FadeTime = 0.0f;
	SlotState.FadeDuration = 0.0f;
	SlotState.bPendingPoseChange = false;
	SlotState.bVisible = false;
}

void CVrdxCharacterManager::BeginFade(FVrdxCharacterSlot& SlotState, const float TargetAlpha, const float FadeSeconds)
{
	if (FadeSeconds <= 0.0f)
	{
		if (TargetAlpha <= 0.0f)
		{
			ResetSlot(SlotState);
			return;
		}

		SlotState.Alpha = TargetAlpha;
		SlotState.StartAlpha = TargetAlpha;
		SlotState.TargetAlpha = TargetAlpha;
		SlotState.FadeTime = 0.0f;
		SlotState.FadeDuration = 0.0f;
		SlotState.bVisible = true;
		SetSpriteAlpha(SlotState.Sprite, TargetAlpha * 255.0f);
		return;
	}

	SlotState.StartAlpha = SlotState.Alpha;
	SlotState.TargetAlpha = TargetAlpha;
	SlotState.FadeTime = 0.0f;
	SlotState.FadeDuration = FadeSeconds;
	SlotState.bVisible = true;
}

void CVrdxCharacterManager::UpdateSlotTransform(FVrdxCharacterSlot& SlotState)
{
	const sf::Vector2u TextureSize = SlotState.Sprite.getTexture().getSize();
	if (TextureSize.x == 0 || TextureSize.y == 0)
	{
		return;
	}

	const float HeightScale = MaxCharacterHeight / static_cast<float>(TextureSize.y);
	const float Scale = std::min(1.0f, HeightScale);
	SlotState.Sprite.setOrigin(sf::Vector2f(TextureSize.x * 0.5f, TextureSize.y * 0.5f));
	SlotState.Sprite.setScale(sf::Vector2f(Scale, Scale));
	SlotState.Sprite.setPosition(sf::Vector2f(GetSlotX(SlotState.Slot), SlotAnchorY));
}

FVrdxString CVrdxCharacterManager::MakeTextureKey(const FVrdxString& CharacterName, const FVrdxString& PoseName)
{
	return CharacterName.ToUtf8() + "/" + PoseName.ToUtf8();
}

FVrdxString CVrdxCharacterManager::MakeTexturePath(const FVrdxString& CharacterName, const FVrdxString& PoseName)
{
	return std::string("Assets/Characters/") + MakeTextureKey(CharacterName, PoseName).ToUtf8() + ".png";
}

float CVrdxCharacterManager::GetSlotX(const EVrdxCharacterPosition Slot)
{
	switch (Slot)
	{
	case EVrdxCharacterPosition::Left:
		return LeftSlotX;
	case EVrdxCharacterPosition::Center:
		return CenterSlotX;
	case EVrdxCharacterPosition::Right:
		return RightSlotX;
	}

	return CenterSlotX;
}

void CVrdxCharacterManager::SetSpriteAlpha(sf::Sprite& Sprite, const float Alpha) const
{
	const auto ClampedAlpha = static_cast<std::uint8_t>(std::clamp(Alpha, 0.0f, 255.0f));
	Sprite.setColor(sf::Color(255, 255, 255, ClampedAlpha));
}

sf::Texture CVrdxCharacterManager::CreateTransparentTexture()
{
	const sf::Image Image(sf::Vector2u(1u, 1u), sf::Color(0, 0, 0, 0));
	return sf::Texture(Image);
}

