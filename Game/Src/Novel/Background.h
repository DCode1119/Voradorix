#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "Core/Common.h"
#include "Core/String.h"

class CVrdxBackground
{
public:
	CVrdxBackground();
	~CVrdxBackground() VRDX_DEFAULT;

	void SetBackground(const FVrdxString& AssetName, float TransitionSeconds = 0.35f);
	void Update(float DeltaTick);
	void Draw(sf::RenderWindow& Window) const;

	bool IsTransitioning() const;

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
