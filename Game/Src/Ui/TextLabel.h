#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include "Core/String.h"
#include "Ui/BoxWidget.h"

class CVrdxTextLabel: public CVrdxBoxWidget
{
public:
	CVrdxTextLabel(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	
	virtual void OnResized() override;
	virtual void Draw(sf::RenderWindow& Window) const override;

	void SetText(const FVrdxString& String);
	void SetFont(const sf::Font& InFont);
	void SetFontSize(const unsigned int FontSize);
	void SetFontColor(const sf::Color& Color);

private:
	sf::Font Font;
	sf::Text Text;
};