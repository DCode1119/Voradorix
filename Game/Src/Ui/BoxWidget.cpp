// Copyright DCode. All Rights Reserved.

#include "Ui/BoxWidget.h"

// C++ Standard Library
#include <cmath>

// Third-party Library
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

CVrdxBoxWidget::CVrdxBoxWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
	, TransparentTexture(std::make_shared<sf::Texture>(CreateTransparentTexture()))
	, Texture(TransparentTexture)
	, Sprite(*Texture)
	, bTextureSet {false}
{
	
}

void CVrdxBoxWidget::SetTexture(TVrdxSharedPtr<sf::Texture> InTexture)
{
	if (!InTexture)
	{
		// error.
		return;
	}

	Texture = InTexture;

	Sprite.setTexture(*Texture);
	UpdateSpriteGeometry();

	bTextureSet = true;
}

void CVrdxBoxWidget::ClearTexture()
{
	SetTexture(TransparentTexture);
	bTextureSet = false;
}

void CVrdxBoxWidget::OnResized()
{
	UpdateSpriteGeometry();
}

void CVrdxBoxWidget::Draw(sf::RenderWindow& Window) const
{
	if (!bTextureSet)
	{
		CVrdxWidgetBase::Draw(Window);
		return;
	}

	if (!IsDrawable())
	{
		return;
	}

	Window.draw(Sprite);

	for (auto& Child : Children)
	{
		Child->Draw(Window);
	}
}

sf::Texture CVrdxBoxWidget::CreateTransparentTexture()
{
	const sf::Image Image(sf::Vector2u(1u, 1u), sf::Color::Transparent);
	return sf::Texture(Image);
}

void CVrdxBoxWidget::UpdateSpriteGeometry()
{
	if (!Texture)
	{
		return;
	}

	Sprite.setPosition(MapToGlobal({0,0}));

	sf::Vector2f ShapeSize = Shape.getSize();
	sf::Vector2u TextureSize = Texture->getSize();
	if (TextureSize.x == 0 || TextureSize.y == 0)
	{
		return;
	}

	//Stretch
	Sprite.setScale(sf::Vector2f(ShapeSize.x / TextureSize.x, ShapeSize.y / TextureSize.y));
}
