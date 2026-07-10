// Copyright DCode. All Rights Reserved.

#include "Novel/DialogueBox.h"

// Third-party Library
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

// Project Headers
#include "Core/Vector.h"
#include "Core/AssetManager.h"

CVrdxDialogueBox::CVrdxDialogueBox(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
	, VisibleCount(0)
	, TypeTimer(0.f)
	, TypeInterval(0.05f)
	, bWaiting(false)
{
	if (auto Font = CVrdxAssetManager::Get().GetFont("malgun"))
	{
		SpeakerText = MakeVrdxShared<sf::Text>(*Font);
		SpeakerText->setCharacterSize(24);
		SpeakerText->setFillColor(sf::Color::White);
		SpeakerText->setPosition(sf::Vector2f(MapToGlobal({20, 10})));

		LineText = MakeVrdxShared<sf::Text>(*Font);
		LineText->setCharacterSize(20);
		LineText->setFillColor(sf::Color(220, 220, 220));
		LineText->setPosition(sf::Vector2f(MapToGlobal({20, 50})));
	}
}

void CVrdxDialogueBox::Update(const float DeltaTick)
{
	CVrdxWidgetBase::Update(DeltaTick);

	if (!IsTyping() || ! LineText)
	{
		return;
	}

	TypeTimer += DeltaTick;
	while (TypeTimer >= TypeInterval && VisibleCount < CurrentTextString.Length())
	{
		VisibleCount++;
		TypeTimer -= TypeInterval;
	}

	const FVrdxString DisplayText = CurrentTextString.Left(VisibleCount);
	LineText->setString(DisplayText.ToSfString());
}

void CVrdxDialogueBox::Draw(sf::RenderWindow& Window) const
{
	if (!IsDrawable())
	{
		return;
	}

	CVrdxWidgetBase::Draw(Window);

	if (auto Font = CVrdxAssetManager::Get().GetFont("malgun"))
	{
		if (SpeakerText && LineText)
		{
			Window.draw(*SpeakerText);
			Window.draw(*LineText);
		}
	}
}

bool CVrdxDialogueBox::OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition)
{
	AdvanceProcess();

	return true;
}

bool CVrdxDialogueBox::OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode)
{
	static TVrdxVector<sf::Keyboard::Scan> ScanKeys =
	{
		sf::Keyboard::Scan::Enter,
		sf::Keyboard::Scan::Space,
	};

	if (ScanKeys.Contains(ScanCode))
	{
		AdvanceProcess();
		return true;
	}

	return false;
}

void CVrdxDialogueBox::SetSpeaker(const FVrdxString& Name)
{
	SpeakerName = Name;

	if (SpeakerText)
	{
		SpeakerText->setString(Name.ToSfString());
	}
}

void CVrdxDialogueBox::SetLine(const FVrdxString& Text)
{
	CurrentTextString = Text;
	StartTyping();
}

bool CVrdxDialogueBox::IsTyping() const
{
	return VisibleCount < CurrentTextString.Length();
}

bool CVrdxDialogueBox::IsFinished() const
{
	return !CurrentTextString.IsEmpty() && !IsTyping();
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

	if (LineText)
	{
		LineText->setString(sf::String());
	}
}

void CVrdxDialogueBox::FinishTyping()
{
	VisibleCount = CurrentTextString.Length();

	if (LineText)
	{
		LineText->setString(CurrentTextString.ToSfString());
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

void CVrdxDialogueBox::Clear()
{
	bWaiting = false;
	CurrentTextString = "";
	VisibleCount = 0;
	if (LineText)
	{
		LineText->setString("");
	}

	if (SpeakerText)
	{
		SpeakerText->setString("");
	}
}
