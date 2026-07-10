// Copyright DCode. All Rights Reserved.

#include "Novel/ChoiceWidget.h"

// C++ Standard Library
#include <unordered_map>

// Project Headers
#include "Core/AssetManager.h"
#include "Novel/NovelScene.h"

CVrdxChoiceWidget::CVrdxChoiceWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
{
}

void CVrdxChoiceWidget::SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene)
{
	WeakNovelScene = NovelScene;
}

void CVrdxChoiceWidget::SetChoices(const TVrdxVector<FVrdxChoiceOption>& Choices)
{
	ChoiceOptions = Choices;

	constexpr int32_t MaxChoices = 4;
	if (ChoiceOptions.Num() > MaxChoices)
	{
		//@todo: log error, invalid script.
		ChoiceOptions.Resize(MaxChoices);
	}
	ChoiceState = EVrdxChoiceState::Waiting;
	SelectedIndex = 0;
	bInvalidated = true;

	SetVisibility(true);
}

void CVrdxChoiceWidget::Clear()
{
	SetChoices({});
	ChoiceState = EVrdxChoiceState::Hidden;

	SetVisibility(false);
}

void CVrdxChoiceWidget::Update(const float DeltaTick)
{
	if (!bInvalidated)
	{
		return;
	}

	constexpr int32_t MaxChoices = 4;

	// Update state
	sf::Vector2f PanelSize = Shape.getSize();
	sf::Vector2f PanelPosition = Shape.getPosition();

	const float VerticalButtonMargin = 10.f;
	const float VerticalPivotSize = PanelSize.y / MaxChoices;

	sf::Vector2f ButtonPosition(PanelPosition);
	sf::Vector2f ButtonSize(PanelSize.x, (PanelSize.y / MaxChoices) - VerticalButtonMargin);

	Buttons.Clear();
	Buttons.Reserve(MaxChoices);
	ButtonTexts.Clear();
	ButtonTexts.Reserve(MaxChoices);

	auto Font = CVrdxAssetManager::Get().GetFont("malgun");
	const sf::Color SelectedColors[] = { sf::Color::Blue, sf::Color::Red };
	const sf::Color NormalColors[] = { sf::Color::Black, sf::Color::White};

	for(int32_t Index = 0; Index < ChoiceOptions.Num(); ++Index)
	{
		const auto& ChoiceOption = ChoiceOptions[Index];

		sf::RectangleShape ButtonShape;
		ButtonShape.setPosition(ButtonPosition);
		ButtonShape.setSize(ButtonSize);

		ButtonShape.setFillColor(Index == SelectedIndex ? SelectedColors[0] : NormalColors[0]);
		ButtonShape.setOutlineColor(Index == SelectedIndex ? SelectedColors[1] : NormalColors[1]);
		ButtonShape.setOutlineThickness(2.0f);

		Buttons.Add(ButtonShape);

		if (Font)
		{
			sf::Text ButtonText(*Font);
			ButtonText.setCharacterSize(20);
			ButtonText.setFillColor(sf::Color::Green);
			ButtonText.setPosition(ButtonPosition);
			ButtonText.setString(ChoiceOption.Text.ToSfString());
			ButtonTexts.Add(ButtonText);
		}

		ButtonPosition.y += VerticalPivotSize;
	}

	bInvalidated = false;
}

void CVrdxChoiceWidget::Draw(sf::RenderWindow& Window) const
{
	// Draw choice options
	if (ChoiceOptions.IsEmpty())
	{
		return;
	}

	//Panel has 4 slots for max 4 options.
	Window.draw(Shape);

	for (auto& Button : Buttons)
	{
		Window.draw(Button);
	}

	for (const auto& ButtonText : ButtonTexts)
	{
		Window.draw(ButtonText);
	}
}

bool CVrdxChoiceWidget::IsWaiting() const
{
	return ChoiceState == EVrdxChoiceState::Waiting;
}

bool CVrdxChoiceWidget::OnMouseMove(const sf::Vector2f& LocalPosition)
{
	int32_t Index = GetHoveredButtonIndex(MapToGlobal(LocalPosition));
	if (Index < 0 || SelectedIndex == Index)
	{
		return false;
	}

	SelectedIndex = Index;
	bInvalidated = true;

	// Pass the move event
	return false;
}

bool CVrdxChoiceWidget::OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition)
{
	if (GetHoveredButtonIndex(MapToGlobal(LocalPosition)) != -1)
	{
		ConfirmSelection();
		return true;
	}

	return false;
}

bool CVrdxChoiceWidget::OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode)
{
	enum class EInputCommmand { None, Previous, Next, Confirm, };
	EInputCommmand InputCommand = EInputCommmand::None;

	// look up table
	static std::unordered_map<sf::Keyboard::Scan, EInputCommmand> KeyboardInput =
	{
		{ sf::Keyboard::Scan::Up, EInputCommmand::Previous },
		{ sf::Keyboard::Scan::W, EInputCommmand::Previous },
		{ sf::Keyboard::Scan::Down, EInputCommmand::Next },
		{ sf::Keyboard::Scan::S, EInputCommmand::Next },
		{ sf::Keyboard::Scan::Enter, EInputCommmand::Confirm },
		{ sf::Keyboard::Scan::Space, EInputCommmand::Confirm },
	};

	auto LookupResult = KeyboardInput.find(ScanCode);
	if (LookupResult != KeyboardInput.end())
	{
		InputCommand = LookupResult->second;
	}

	switch (InputCommand)
	{
	case EInputCommmand::Next:
		MoveSelection(1);
		break;

	case EInputCommmand::Previous:
		MoveSelection(-1);
		break;

	case EInputCommmand::Confirm:
		ConfirmSelection();
		break;

	default:
		return false;
	}

	return true;
}

void CVrdxChoiceWidget::MoveSelection(int32_t Delta)
{
	const int32_t DesiredIndex = SelectedIndex + Delta;
	if (ChoiceOptions.IsValidIndex(DesiredIndex))
	{
		SelectedIndex = DesiredIndex;
		bInvalidated = true;
	}
}

void CVrdxChoiceWidget::ConfirmSelection()
{
	if (auto Locked = WeakNovelScene.lock())
	{
		if (ChoiceOptions.IsValidIndex(SelectedIndex))
		{
			Locked->JumpToLabel(ChoiceOptions[SelectedIndex].TargetLabel);
		}
	}
	Clear();
}


int32_t CVrdxChoiceWidget::GetHoveredButtonIndex(const sf::Vector2i& MousePosition) const
{
	return GetHoveredButtonIndex(sf::Vector2f(MousePosition));
}

int32_t CVrdxChoiceWidget::GetHoveredButtonIndex(const sf::Vector2f& MousePosition) const
{
	for (int32_t Index = 0; Index < ChoiceOptions.Num(); ++Index)
	{
		const sf::RectangleShape& ButtonShape = Buttons[Index];
		const sf::Rect ButtonRect(ButtonShape.getPosition(), ButtonShape.getSize());
		if (ButtonRect.contains(sf::Vector2f(MousePosition)))
		{
			return Index;
		}
	}

	return -1;
}
