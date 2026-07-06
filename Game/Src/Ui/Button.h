// Copyright DCode. All Rights Reserved.
#pragma once

// C++ Standard Library
#include <functional>

// Third-party Library
#include <SFML/System/Vector2.hpp>

// Project Headers
#include "Core/String.h"
#include "Ui/BoxWidget.h"

class CVrdxBoxWidget;
class CVrdxTextLabel;

class CVrdxButton: public CVrdxBoxWidget
{
public:
	CVrdxButton(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	
	virtual void OnPostCreate() override;
	virtual void OnPreDestroy() override;
	virtual void OnResized() override;
	virtual bool OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition) override;
	virtual bool OnMouseLeftButtonReleased(const sf::Vector2f& LocalPosition) override;

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
