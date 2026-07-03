#include "Novel/ChoiceManager.h"

#include <unordered_map>

#include "Scene/NovelScene.h"

CVrdxChoiceManager::CVrdxChoiceManager()
{
	Panel.setSize(sf::Vector2f(1200.f, 200.f));
	Panel.setPosition(sf::Vector2f(40.f, 500.f));
	Panel.setFillColor(sf::Color(20, 20, 20, 220));

	bFontLoaded = Font.openFromFile("Assets/Fonts/malgun.ttf");
}

void CVrdxChoiceManager::SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene)
{
	WeakNovelScene = NovelScene;
}

void CVrdxChoiceManager::SetChoices(const TVrdxVector<FVrdxChoiceOption>& Choices)
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
}

void CVrdxChoiceManager::Clear()
{
	SetChoices({});
	ChoiceState = EVrdxChoiceState::Hidden;
}

void CVrdxChoiceManager::HandleEvent(const sf::Event& Event)
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

	if (const auto* MousePressed = Event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (MousePressed->button == sf::Mouse::Button::Left)
		{
			if (GetHoveredButtonIndex(MousePressed->position) != -1)
			{
				InputCommand = EInputCommmand::Confirm;
			}
		}
	}

	if (const auto* MouseMoved = Event.getIf<sf::Event::MouseMoved>())
	{
		const int32_t HoveredButtonIndex = GetHoveredButtonIndex(MouseMoved->position);
		if (HoveredButtonIndex < 0 || SelectedIndex == HoveredButtonIndex)
		{
			return;
		}

		SelectedIndex = HoveredButtonIndex;
		bInvalidated = true;
	}

	if (const auto* KeyPressed = Event.getIf<sf::Event::KeyPressed>())
	{
		auto LookupResult = KeyboardInput.find(KeyPressed->scancode);
		if (LookupResult != KeyboardInput.end())
		{
			InputCommand = LookupResult->second;
		}
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
	}
}

void CVrdxChoiceManager::Update(const float DeltaTick)
{
	if (!bInvalidated)
	{
		return;
	}

	constexpr int32_t MaxChoices = 4;

	// Update state
	sf::Vector2f PanelSize = Panel.getSize();
	sf::Vector2f PanelPosition = Panel.getPosition();

	const float VerticalButtonMargin = 10.f;
	const float VerticalPivotSize = PanelSize.y / MaxChoices;

	sf::Vector2f ButtonPosition(PanelPosition);
	sf::Vector2f ButtonSize(PanelSize.x, (PanelSize.y / MaxChoices) - VerticalButtonMargin);

	Buttons.Clear();
	Buttons.Reserve(MaxChoices);
	ButtonTexts.Clear();
	ButtonTexts.Reserve(MaxChoices);

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

		if (bFontLoaded)
		{
			sf::Text ButtonText(Font);
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

void CVrdxChoiceManager::Draw(sf::RenderWindow& Window) const
{
	// Draw choice options
	if (ChoiceOptions.IsEmpty())
	{
		return;
	}

	//Panel has 4 slots for max 4 options.
	Window.draw(Panel);

	for (auto& Button : Buttons)
	{
		Window.draw(Button);
	}

	for (const auto& ButtonText : ButtonTexts)
	{
		Window.draw(ButtonText);
	}
}

bool CVrdxChoiceManager::IsWaiting() const
{
	return ChoiceState == EVrdxChoiceState::Waiting;
}

void CVrdxChoiceManager::MoveSelection(int32_t Delta)
{
	const int32_t DesiredIndex = SelectedIndex + Delta;
	if (ChoiceOptions.IsValidIndex(DesiredIndex))
	{
		SelectedIndex = DesiredIndex;
		bInvalidated = true;
	}
}

void CVrdxChoiceManager::ConfirmSelection()
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

int32_t CVrdxChoiceManager::GetHoveredButtonIndex(const sf::Vector2i& MousePosition) const
{
	for (int32_t Index = 0; Index < ChoiceOptions.Num(); ++Index)
	{
		const sf::RectangleShape& ButtonShape = Buttons[Index];
		const sf::Vector2f ButtonPosition = ButtonShape.getPosition();
		const sf::Vector2f ButtonSize = ButtonShape.getSize();

		const sf::Vector2f LeftTop = ButtonPosition;
		const sf::Vector2f RightBottom = ButtonPosition + ButtonSize;

		if ((LeftTop.x <= MousePosition.x) && (LeftTop.y <= MousePosition.y)
			&& (MousePosition.x <= RightBottom.x) && (MousePosition.y <= RightBottom.y))
		{
			return Index;
		}
	}

	return -1;
}
