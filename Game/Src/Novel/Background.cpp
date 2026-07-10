// Copyright DCode. All Rights Reserved.

#include "Novel/Background.h"

// C++ Standard Library
#include <algorithm>
#include <cstdint>

// Third-party Library
#include <SFML/Graphics/Color.hpp>

// Project Headers
#include "Core/AssetManager.h"

//Will be moved to configuration module.
namespace
{
	constexpr float BackgroundWidth = 1280.f;
	constexpr float BackgroundHeight = 720.f;
}

CVrdxBackground::CVrdxBackground(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
	, TransparentTexture(CreateTransparentTexture())
	, CurrentSprite(TransparentTexture)
	, NextSprite(TransparentTexture)
	, TransitionTime(0.f)
	, TransitionDuration(0.f)
	, bHasCurrent(false)
	, bIsTransitioning(false)
{
	Shape.setFillColor(sf::Color::Transparent);
}

void CVrdxBackground::SetBackground(const FVrdxString& AssetName, float TransitionSeconds /*= 0.35f*/)
{
	if (bHasCurrent && !bIsTransitioning && AssetName == CurrentAssetName)
	{
		return;
	}

	if (bIsTransitioning && AssetName == NextAssetName)
	{
		return;
	}

	NextTexture = CVrdxAssetManager::Get().GetTexture(AssetName);
	if (!NextTexture)
	{
		return;
	}

	SetupSprite(NextSprite, *NextTexture);
	NextAssetName = AssetName;

	TransitionDuration = std::max(TransitionSeconds, 0.f);
	TransitionTime = 0.f;

	if (!bHasCurrent || TransitionDuration <= 0.f)
	{
		CurrentTexture = NextTexture;
		SetupSprite(CurrentSprite, *CurrentTexture);
		CurrentAssetName = NextAssetName;
		bHasCurrent = true;
		bIsTransitioning = false;
		SetupSprite(NextSprite, TransparentTexture);
		TransitionDuration = 0.f;
		TransitionTime = 0.f;
		return;
	}

	bIsTransitioning = true;
}

void CVrdxBackground::Update(float DeltaTick)
{
	if (!bIsTransitioning)
	{
		return;
	}

	TransitionTime += DeltaTick;
	if (TransitionTime < TransitionDuration)
	{
		return;
	}

	CurrentTexture = NextTexture;
	SetupSprite(CurrentSprite, *CurrentTexture);
	CurrentAssetName = NextAssetName;
	bHasCurrent = true;
	bIsTransitioning = false;
	SetupSprite(NextSprite, TransparentTexture);
	TransitionTime = 0.f;
	TransitionDuration = 0.f;
}

void CVrdxBackground::Draw(sf::RenderWindow& Window) const
{
	if (!bHasCurrent && !bIsTransitioning)
	{
		return;
	}

	if (!bIsTransitioning)
	{
		Window.draw(CurrentSprite);
		return;
	}

	const float Progress = (TransitionDuration > 0.f)
		? std::clamp(TransitionTime / TransitionDuration, 0.f, 1.f)
		: 1.f;

	sf::Sprite CurrentDraw = CurrentSprite;
	sf::Sprite NextDraw = NextSprite;
	SetSpriteAlpha(CurrentDraw, 255.f * (1.f - Progress));
	SetSpriteAlpha(NextDraw, 255.f * Progress);

	Window.draw(CurrentDraw);
	Window.draw(NextDraw);
}

bool CVrdxBackground::IsTransitioning() const
{
	return bIsTransitioning;
}

void CVrdxBackground::Clear()
{
	TransitionTime = 0.f;
	TransitionDuration = 0.f;
	bHasCurrent = false;
	bIsTransitioning = false;

	CurrentSprite.setTexture(TransparentTexture);
	NextSprite.setTexture(TransparentTexture);
	CurrentAssetName = "";
	NextAssetName = "";
}

void CVrdxBackground::SetupSprite(sf::Sprite& Sprite, const sf::Texture& Texture) const
{
	Sprite.setTexture(Texture, true);
	ApplyFullScreenTransform(Sprite, Texture, BackgroundWidth, BackgroundHeight);
	Sprite.setColor(sf::Color::White);
}

void CVrdxBackground::ApplyFullScreenTransform(sf::Sprite& Sprite, const sf::Texture& Texture, float TargetWidth, float TargetHeight) const
{
	const sf::Vector2u TextureSize = Texture.getSize();
	if (TextureSize.x == 0 || TextureSize.y == 0 || TargetWidth <= 0.f || TargetHeight <= 0.f)
	{
		return;
	}

	Sprite.setOrigin(sf::Vector2f(0.f, 0.f));
	Sprite.setPosition(sf::Vector2f(0.f, 0.f));
	Sprite.setScale(sf::Vector2f(
		TargetWidth / static_cast<float>(TextureSize.x),
		TargetHeight / static_cast<float>(TextureSize.y)));
}

void CVrdxBackground::SetSpriteAlpha(sf::Sprite& Sprite, const float Alpha) const
{
	const auto ClampedAlpha = static_cast<std::uint8_t>(std::clamp(Alpha, 0.f, 255.f));
	Sprite.setColor(sf::Color(255, 255, 255, ClampedAlpha));
}

sf::Texture CVrdxBackground::CreateTransparentTexture()
{
	const sf::Image Image(sf::Vector2u(1u, 1u), sf::Color(0, 0, 0, 0));
	const sf::Texture Texture(Image);
	return Texture;
}
