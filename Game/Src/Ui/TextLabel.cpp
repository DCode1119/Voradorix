// Copyright DCode. All Rights Reserved.

// Third-party Library
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

// Project headers
#include "Core/AssetManager.h"
#include "Ui/TextLabel.h"

CVrdxTextLabel::CVrdxTextLabel(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxBoxWidget(ParentWidget, InShape)
{
	if (auto Font = CVrdxAssetManager::Get().GetFont("malgun"))
	{
		Text = MakeVrdxShared<sf::Text>(*Font);
		Text->setCharacterSize(24);
		Text->setFillColor(sf::Color::White);
		Text->setPosition(MapToGlobal({ 0,0 }));
	}
}

void CVrdxTextLabel::OnResized()
{
	if (Text)
	{
		Text->setPosition(MapToGlobal({ 0,0 }));
	}
}

void CVrdxTextLabel::Draw(sf::RenderWindow& Window) const
{
	CVrdxBoxWidget::Draw(Window);

	if (IsDrawable())
	{
		Window.draw(*Text);
	}
}

void CVrdxTextLabel::SetText(const FVrdxString& String)
{
	if (Text)
	{
		Text->setString(String.ToSfString());
	}
}

void CVrdxTextLabel::SetFont(const FVrdxString& String)
{
	if (auto Font = CVrdxAssetManager::Get().GetFont("malgun"))
	{
		if (Text)
		{
			Text->setFont(*Font);
		}
	}
}

void CVrdxTextLabel::SetFontSize(const unsigned int FontSize)
{
	if (Text)
	{
		Text->setCharacterSize(FontSize);
	}
}

void CVrdxTextLabel::SetFontColor(const sf::Color& Color)
{
	if (Text)
	{
		Text->setFillColor(Color);
	}
}
