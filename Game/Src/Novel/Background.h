// Copyright DCode. All Rights Reserved.
#pragma once

// Third-party Library
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

// Project Headers
#include "Core/Common.h"
#include "Core/String.h"
#include "Ui/WidgetBase.h"

class CVrdxBackground : public CVrdxWidgetBase
{
public:
	CVrdxBackground(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

	void SetBackground(const FVrdxString& AssetName, float TransitionSeconds = 0.35f);
	void Update(float DeltaTick);
	void Draw(sf::RenderWindow& Window) const;

	bool IsTransitioning() const;

	FVrdxString GetCurrentAssetName() const { return CurrentAssetName; }

private:
	void SetupSprite(sf::Sprite& Sprite, const sf::Texture& Texture) const;
	void ApplyFullScreenTransform(sf::Sprite& Sprite, const sf::Texture& Texture, float TargetWidth, float TargetHeight) const;
	void SetSpriteAlpha(sf::Sprite& Sprite, float Alpha) const;
	static sf::Texture CreateTransparentTexture();

	sf::Texture TransparentTexture;
	sf::Texture CurrentTexture;
	sf::Texture NextTexture;
	sf::Sprite   CurrentSprite;
	sf::Sprite   NextSprite;

	FVrdxString  CurrentAssetName;
	FVrdxString  NextAssetName;

	float        TransitionTime;
	float        TransitionDuration;
	bool         bHasCurrent;
	bool         bIsTransitioning;
};
