// Copyright DCode. All Rights Reserved.
#pragma once

// Third-party Library
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

// Project Headers
#include "Core/Common.h"
#include "Core/String.h"
#include "Core/Vector.h"
#include "Ui/WidgetBase.h"

class CVrdxNovelScene;

struct FVrdxChoiceOption
{
	FVrdxString Text;
	FVrdxString TargetLabel;
};

class CVrdxChoiceWidget : public CVrdxWidgetBase
{
private:
	enum class EVrdxChoiceState { Hidden, Waiting, };

public:
	CVrdxChoiceWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

	//CVrdxChoiceWidget();
	//~CVrdxChoiceWidget() VRDX_DEFAULT;

	void SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene);
	void SetChoices(const TVrdxVector<FVrdxChoiceOption>& Choices);
	void Clear();

	void Update(const float DeltaTick);
	void Draw(sf::RenderWindow& Window) const;

	bool IsWaiting() const;

	virtual void OnMouseMove(const sf::Vector2f& LocalPosition) override;
	virtual void OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition) override;
	virtual void OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode) override;

private:
	void MoveSelection(int32_t Delta);
	void ConfirmSelection();
	int32_t GetHoveredButtonIndex(const sf::Vector2i& MousePosition) const;
	int32_t GetHoveredButtonIndex(const sf::Vector2f& MousePosition) const;

	TVrdxVector<FVrdxChoiceOption> ChoiceOptions;
	int32_t SelectedIndex = 0;

	EVrdxChoiceState ChoiceState = EVrdxChoiceState::Hidden;

	sf::Font Font;
	bool bFontLoaded = false;

	TVrdxVector<sf::RectangleShape> Buttons;
	TVrdxVector<sf::Text> ButtonTexts;

	TVrdxWeakPtr<CVrdxNovelScene> WeakNovelScene;

	bool bInvalidated = false;
};
