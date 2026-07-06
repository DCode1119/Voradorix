// Copyright DCode. All Rights Reserved.

#include "Ui/Button.h"

// Project Headers
#include "Ui/BoxWidget.h"
#include "Ui/TextLabel.h"

CVrdxButton::CVrdxButton(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxBoxWidget(ParentWidget, InShape)
{
	
}


void CVrdxButton::Update(const float DeltaTick)
{
	Text->SetFontColor(Normal->GetVisibility() ? NormalFontColor : PressedFontColor);
}

void CVrdxButton::OnPostCreate()
{
	Shape.setFillColor(sf::Color::Transparent);

	sf::RectangleShape ButtonShape = Shape;
	ButtonShape.setPosition({0, 0});
	ButtonShape.setOutlineThickness(1.0f);
	Normal = CVrdxWidgetBase::CreateWidget<CVrdxBoxWidget>(shared_from_this(), ButtonShape);
	Pressed = CVrdxWidgetBase::CreateWidget<CVrdxBoxWidget>(shared_from_this(), ButtonShape);
	Pressed->SetVisibility(false);

	sf::RectangleShape TextShape = ButtonShape;
	TextShape.setPosition({ 0,0 });
	TextShape.setFillColor(sf::Color::Transparent);
	TextShape.setOutlineColor(sf::Color::Transparent);
	Text = CVrdxWidgetBase::CreateWidget<CVrdxTextLabel>(shared_from_this(), TextShape);

	Normal->SetIgnoreEvent(true);
	Pressed->SetIgnoreEvent(true);
	Text->SetIgnoreEvent(true);
}

void CVrdxButton::OnPreDestroy()
{
	Normal = nullptr;
	Pressed = nullptr;
	Text = nullptr;
}

void CVrdxButton::OnResized()
{
	sf::Vector2f Size = Shape.getSize();
	Normal->GetShape().setSize(Size);
	Pressed->GetShape().setSize(Size);
	Text->GetShape().setSize(Size);
}

bool CVrdxButton::OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition)
{
	SetCapture(true);
	Normal->SetVisibility(false);
	Pressed->SetVisibility(true);

	return true;
}

bool CVrdxButton::OnMouseLeftButtonReleased(const sf::Vector2f& LocalPosition)
{
	SetCapture(false);
	Normal->SetVisibility(true);
	Pressed->SetVisibility(false);

	if (ContainsInLocal(LocalPosition))
	{
		OnClicked.Broadcast();
	}

	return true;
}

void CVrdxButton::SetText(const FVrdxString& String)
{
	Text->SetText(String);
}

void CVrdxButton::SetFontColors(const sf::Color& NormalColor, const sf::Color& PressedColor)
{
	NormalFontColor = NormalColor;
	PressedFontColor = PressedColor;
	Text->SetFontColor(Normal->GetVisibility() ? NormalColor : PressedColor);
}

void CVrdxButton::SetColors(const sf::Color& NormalColor, const sf::Color& PressedColor)
{
	Normal->GetShape().setFillColor(NormalColor);
	Pressed->GetShape().setFillColor(PressedColor);
}

void CVrdxButton::SetLineColors(const sf::Color& NormalColor, const sf::Color& PressedColor)
{
	Normal->GetShape().setOutlineColor(NormalColor);
	Pressed->GetShape().setOutlineColor(PressedColor);
}
