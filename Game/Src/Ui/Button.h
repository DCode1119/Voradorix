#pragma once

#include <functional>

#include <SFML/System/Vector2.hpp>

#include "Ui/BoxWidget.h"
#include "Core/String.h"

class CVrdxBoxWidget;
class CVrdxTextLabel;

class CVrdxButton: public CVrdxBoxWidget
{
public:
	CVrdxButton(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	
	virtual void OnPostCreate() override;
	virtual void OnPreDestroy() override;
	virtual void OnResized() override;
	virtual void OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition) override;
	virtual void OnMouseLeftButtonReleased(const sf::Vector2f& LocalPosition) override;

	void SetText(const FVrdxString& String);
	void SetColors(const sf::Color& NormalColor, const sf::Color& PressedColor);
	void SetLineColors(const sf::Color& NormalColor, const sf::Color& PressedColor);
	
	void SetCallback(std::function<void()>&& InCallback) { Callback = std::move(InCallback); }

private:
	TVrdxSharedPtr<CVrdxBoxWidget> Normal;
	TVrdxSharedPtr<CVrdxBoxWidget> Pressed;
	TVrdxSharedPtr<CVrdxTextLabel> Text;

	std::function<void()> Callback;
};