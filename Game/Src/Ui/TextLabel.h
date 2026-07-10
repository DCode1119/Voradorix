// Copyright DCode. All Rights Reserved.
#pragma once

// Project Headers
#include "Core/String.h"
#include "Ui/BoxWidget.h"

namespace sf { class Text; }

class CVrdxTextLabel: public CVrdxBoxWidget
{
public:
	CVrdxTextLabel(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	
	virtual void OnResized() override;
	virtual void Draw(sf::RenderWindow& Window) const override;

	void SetText(const FVrdxString& String);
	void SetFont(const FVrdxString& FontName);
	void SetFontSize(const unsigned int FontSize);
	void SetFontColor(const sf::Color& Color);

private:
	TVrdxSharedPtr<sf::Text> Text;
};
