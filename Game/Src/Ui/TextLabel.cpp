// Copyright DCode. All Rights Reserved.

#include "Ui/TextLabel.h"

CVrdxTextLabel::CVrdxTextLabel(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxBoxWidget(ParentWidget, InShape)
	, Text(Font)
{
	if (!Font.openFromFile("Assets/Fonts/malgun.ttf"))
	{
		// @error
		return;
	}

	Text.setCharacterSize(24);
	Text.setFillColor(sf::Color::White);
	Text.setPosition(MapToGlobal({0,0}));
}

void CVrdxTextLabel::OnResized()
{
	Text.setPosition(MapToGlobal({ 0,0 }));
}

void CVrdxTextLabel::Draw(sf::RenderWindow& Window) const
{
	CVrdxBoxWidget::Draw(Window);

	if (IsDrawable())
	{
		Window.draw(Text);
	}
}

void CVrdxTextLabel::SetText(const FVrdxString& String)
{
	Text.setString(String.ToSfString());
}

void CVrdxTextLabel::SetFont(const sf::Font& InFont)
{
	Font = InFont;
}

void CVrdxTextLabel::SetFontSize(const unsigned int FontSize)
{
	Text.setCharacterSize(FontSize);
}

void CVrdxTextLabel::SetFontColor(const sf::Color& Color)
{
	Text.setFillColor(Color);
}
