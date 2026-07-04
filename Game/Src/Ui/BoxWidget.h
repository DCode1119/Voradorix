#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "Ui/WidgetBase.h"

class CVrdxBoxWidget: public CVrdxWidgetBase
{
public:
	CVrdxBoxWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	void SetTexture(TVrdxSharedPtr<sf::Texture> InTexture);
	void ClearTexture();

	virtual void Update(float DeltaTick) override;
	virtual void Draw(sf::RenderWindow& Window) const override;

private:
	bool Equals(const sf::FloatRect& A, const sf::FloatRect& B) const;
	sf::Texture CreateTransparentTexture();
	void UpdateSpriteGeometry();

	bool bTextureSet = false;
	sf::Sprite Sprite;
	TVrdxSharedPtr<sf::Texture> Texture;
	TVrdxSharedPtr<sf::Texture> TransparentTexture;

	sf::FloatRect CachedGeometry;
};