// Copyright DCode. All Rights Reserved.

#include "Novel/DialogueBox.h"

// Third-party Library
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

// Project Headers
#include "Core/Vector.h"

CVrdxDialogueBox::CVrdxDialogueBox(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
	, SpeakerText(Font)
	, LineText(Font)
	, VisibleCount(0)
	, TypeTimer(0.f)
	, TypeInterval(0.05f)
	, bFontLoaded(false)
	, bWaiting(false)
{
	bFontLoaded = Font.openFromFile("Assets/Fonts/malgun.ttf");

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

void CVrdxDialogueBox::Update(const float DeltaTick)
{
	CVrdxWidgetBase::Update(DeltaTick);

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
	CVrdxWidgetBase::Draw(Window);

	if (bFontLoaded)
	{
		Window.draw(SpeakerText);
		Window.draw(LineText);
	}
}

void CVrdxDialogueBox::OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition)
{
	AdvanceProcess();
}

void CVrdxDialogueBox::OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode)
{
	static TVrdxVector<sf::Keyboard::Scan> ScanKeys =
	{
		sf::Keyboard::Scan::Enter,
		sf::Keyboard::Scan::Space,
	};

	if (ScanKeys.Contains(ScanCode))
	{
		AdvanceProcess();
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

void CVrdxDialogueBox::AdvanceProcess()
{
	if (!IsWaiting())
	{
		return;
	}

	if (IsTyping())
	{
		FinishTyping();
	}
	else
	{
		bWaiting = false;
	}

}
