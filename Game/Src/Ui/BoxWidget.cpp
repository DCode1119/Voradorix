#include "Ui/BoxWidget.h"

#include <cmath>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>

CVrdxBoxWidget::CVrdxBoxWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
	, TransparentTexture(std::make_shared<sf::Texture>(CreateTransparentTexture()))
	, Texture(TransparentTexture)
	, Sprite(*Texture)
	, bTextureSet {false}
{
	CachedGeometry = MapToGlobal(sf::FloatRect({0,0}, Shape.getSize()));
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

void CVrdxBoxWidget::Update(float DeltaTick)
{
	CVrdxWidgetBase::Update(DeltaTick);

	const auto Geometry = MapToGlobal(sf::FloatRect({0,0}, Shape.getSize()));
	if (Equals(CachedGeometry, Geometry))
	{
		return;
	}

	UpdateSpriteGeometry();
	CachedGeometry = Geometry;
}

void CVrdxBoxWidget::Draw(sf::RenderWindow& Window) const
{
	if (!bTextureSet)
	{
		CVrdxWidgetBase::Draw(Window);
		return;
	}

	if (!GetVisibility())
	{
		return;
	}

	if (auto ParentWidget = Parent.lock())
	{
		for (const auto Sibling : ParentWidget->GetChildren())
		{
			if (Sibling->Hides(this))
			{
				return;
			}
		}
	}

	bool bDrawThis = true;
	for (const auto Child : Children)
	{
		if (Child->Hides(this))
		{
			bDrawThis = false;
			break;
		}
	}

	if (bDrawThis)
	{
		Window.draw(Sprite);
	}

	for (auto& Child : Children)
	{
		Child->Draw(Window);
	}
	//#endif
}

bool CVrdxBoxWidget::Equals(const sf::FloatRect& A, const sf::FloatRect& B) const
{
	constexpr float epsilon = 0.001f;
	return (std::abs(A.position.x - B.position.x) < epsilon)
		&& (std::abs(A.position.y - B.position.y) < epsilon)
		&& (std::abs(A.size.x - B.size.x) < epsilon)
		&& (std::abs(A.size.y - B.size.y) < epsilon);
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
