#include "Ui/DialogueBox.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "Core/Vector.h"

CVrdxDialogueBox::CVrdxDialogueBox()
	: SpeakerText(Font)
	, LineText(Font)
	, VisibleCount(0)
	, TypeTimer(0.f)
	, TypeInterval(0.05f)
	, bFontLoaded(false)
{
	bFontLoaded = Font.openFromFile("Assets/Fonts/malgun.ttf");

	Panel.setSize(sf::Vector2f(1200.f, 200.f));
	Panel.setPosition(sf::Vector2f(40.f, 500.f));
	Panel.setFillColor(sf::Color(20, 20, 20, 220));

	if (bFontLoaded)
	{
		SpeakerText.setCharacterSize(24);
		SpeakerText.setFillColor(sf::Color::White);
		SpeakerText.setPosition(sf::Vector2f(60.f, 510.f));

		LineText.setCharacterSize(20);
		LineText.setFillColor(sf::Color(220, 220, 220));
		LineText.setPosition(sf::Vector2f(60.f, 550.f));
	}
}

void CVrdxDialogueBox::HandleEvent(const sf::Event& Event)
{
	static TVrdxVector<sf::Keyboard::Scan> ScanKeys =
	{
		sf::Keyboard::Scan::Enter,
		sf::Keyboard::Scan::Space,
	};

	bool bFinishTyping = false;
	if (const auto* KeyPressed = Event.getIf<sf::Event::KeyPressed>())
	{
		if (ScanKeys.Contains(KeyPressed->scancode))
		{
			bFinishTyping = true;
		}
	}

	else if (const auto* MousePressed = Event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (MousePressed->button == sf::Mouse::Button::Left)
		{
			bFinishTyping = true;
		}
	}

	if (bFinishTyping)
	{


		if (IsTyping())
		{
			// Complete typing, wait for user input.
			FinishTyping();
		}
		else
		{
			bWaiting = false;
		}
	}
}

void CVrdxDialogueBox::Update(const float DeltaTick)
{
	if (!IsTyping())
	{
		return;
	}

	TypeTimer += DeltaTick;
	while (TypeTimer >= TypeInterval && VisibleCount < CurrentText.Length())
	{
		VisibleCount++;
		TypeTimer -= TypeInterval;
	}

	const FVrdxString DisplayText = CurrentText.Left(VisibleCount);
	LineText.setString(DisplayText.ToSfString());
}

void CVrdxDialogueBox::Draw(sf::RenderWindow& Window) const
{
	Window.draw(Panel);

	if (bFontLoaded)
	{
		Window.draw(SpeakerText);
		Window.draw(LineText);
	}
}

void CVrdxDialogueBox::SetSpeaker(const FVrdxString& Name)
{
	SpeakerName = Name;

	if (bFontLoaded)
	{
		SpeakerText.setString(Name.ToSfString());
	}
}

void CVrdxDialogueBox::SetLine(const FVrdxString& Text)
{
	CurrentText = Text;
	StartTyping();
}

bool CVrdxDialogueBox::IsTyping() const
{
	return VisibleCount < CurrentText.Length();
}

bool CVrdxDialogueBox::IsFinished() const
{
	return !CurrentText.IsEmpty() && !IsTyping();
}

bool CVrdxDialogueBox::IsWaiting() const
{
	return bWaiting;
}

void CVrdxDialogueBox::StartTyping()
{
	bWaiting = true;
	VisibleCount = 0;
	TypeTimer = 0.f;

	if (bFontLoaded)
	{
		LineText.setString(sf::String());
	}
}

void CVrdxDialogueBox::FinishTyping()
{
	VisibleCount = CurrentText.Length();

	if (bFontLoaded)
	{
		LineText.setString(CurrentText.ToSfString());
	}
}
