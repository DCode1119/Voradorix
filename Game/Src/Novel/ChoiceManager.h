#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include "Core/Common.h"
#include "Core/String.h"
#include "Core/Vector.h"

class CVrdxNovelScene;

struct FVrdxChoiceOption
{
	FVrdxString Text;
	FVrdxString TargetLabel;
};

class CVrdxChoiceManager
{
private:
	enum class EVrdxChoiceState { Hidden, Waiting, };

public:
	CVrdxChoiceManager();
	~CVrdxChoiceManager() VRDX_DEFAULT;

	void SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene);
	void SetChoices(const TVrdxVector<FVrdxChoiceOption>& Choices);
	void Clear();

	void HandleEvent(const sf::Event& Event);
	void Update(const float DeltaTick);
	void Draw(sf::RenderWindow& Window) const;

	bool IsWaiting() const;

private:
	void MoveSelection(int32_t Delta);
	void ConfirmSelection();
	int32_t GetHoveredButtonIndex(const sf::Vector2i& MousePosition) const;

	TVrdxVector<FVrdxChoiceOption> ChoiceOptions;
	int32_t SelectedIndex = 0;

	EVrdxChoiceState ChoiceState = EVrdxChoiceState::Hidden;

	sf::Font Font;
	bool bFontLoaded = false;

	sf::RectangleShape Panel;
	TVrdxVector<sf::RectangleShape> Buttons;
	TVrdxVector<sf::Text> ButtonTexts;

	TVrdxWeakPtr<CVrdxNovelScene> WeakNovelScene;

	bool bInvalidated = false;
};
