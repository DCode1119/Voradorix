// Copyright DCode. All Rights Reserved.
#pragma once

// Third-party Library
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

// Project Headers
#include "Ui/WidgetBase.h"

class CVrdxBoxWidget: public CVrdxWidgetBase
{
public:
	CVrdxBoxWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	void SetTexture(TVrdxSharedPtr<sf::Texture> InTexture);
	void ClearTexture();

	virtual void OnResized() override;
	virtual void Draw(sf::RenderWindow& Window) const override;

private:
	sf::Texture CreateTransparentTexture();
	void UpdateSpriteGeometry();

	bool bTextureSet = false;
	sf::Sprite Sprite;
	TVrdxSharedPtr<sf::Texture> Texture;
	TVrdxSharedPtr<sf::Texture> TransparentTexture;
};
