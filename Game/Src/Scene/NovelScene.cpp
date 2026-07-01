#include "NovelScene.h"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Window/Mouse.hpp"

CVrdxNovelScene::CVrdxNovelScene()
{
	FVrdxDialogueLine DialogueLine =  { "???", "Welcome to the Chamber of Creation." };
	Script.Add(DialogueLine);
	CurrentIndex = 0;

	//First initialize.
	BackgroundIndex = -1;
	SwitchBackground(0);
	ShowCharacter("Laura", EVrdxCharacterPosition::Center);
}

void CVrdxNovelScene::OnEnter()
{
	CurrentIndex = 0;
	BackgroundIndex = 0;
	ShowNextLine();
}

void CVrdxNovelScene::OnExit()
{
	//
}

void CVrdxNovelScene::HandleEvent(const sf::Event& Event)
{
	static TVrdxVector<sf::Keyboard::Scan> SkipKeys =
	{
		sf::Keyboard::Scan::Enter,
		sf::Keyboard::Scan::Space,
	};

	bool bProceed = false;

	if (const auto* KeyPressed = Event.getIf<sf::Event::KeyPressed>())
	{
		if (SkipKeys.Contains(KeyPressed->scancode))
		{
			bProceed = true;
		}
	}

	else if (const auto* MousePressed = Event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (MousePressed->button  == sf::Mouse::Button::Left)
		{
			bProceed = true;
		}
	}

	if (bProceed)
	{
		if (CurrentIndex < Script.Num())
		{
			if (DialogeBox.IsTyping())
			{
				DialogeBox.FinishTyping();
			}
			else
			{
				SwitchBackground((BackgroundIndex == 0) ? 1 : 0);
				CurrentIndex = 0;
				DialogeBox.SetLine(Script[CurrentIndex].Text);
			}
		}
		else
		{
			
		}
	}
}

void CVrdxNovelScene::Update(const float DeltaTick)
{
	Background.Update(DeltaTick);
	CharacterManager.Update(DeltaTick);
	DialogeBox.Update(DeltaTick);
}

void CVrdxNovelScene::Draw(sf::RenderWindow& Window)
{
	Window.clear(sf::Color::Black);

	Background.Draw(Window);
	CharacterManager.Draw(Window);
	DialogeBox.Draw(Window);
}

void CVrdxNovelScene::SetBackground(const FVrdxString& BackgroundName)
{
	Background.SetBackground(BackgroundName);
}

void CVrdxNovelScene::ShowCharacter(const FVrdxString& Character, const EVrdxCharacterPosition Position)
{
	CharacterManager.ShowCharacter(Character, Position);
}

void CVrdxNovelScene::HideCharacter(const FVrdxString& Character)
{
	CharacterManager.HideCharacter(Character);
}

void CVrdxNovelScene::SetCharacterPose(const FVrdxString& Character, const FVrdxString& Pose)
{
	CharacterManager.SetCharacterPose(Character, Pose);
}

void CVrdxNovelScene::ShowNextLine()
{
	if (CurrentIndex < Script.Num())
	{
		DialogeBox.SetSpeaker(Script[CurrentIndex].Speaker);
		DialogeBox.SetLine(Script[CurrentIndex].Text);
	}
	else
	{
		EndScenario();
	}
}

void CVrdxNovelScene::EndScenario()
{
	RequestExit();
}

void CVrdxNovelScene::SwitchBackground(int32_t InBackgroundIndex)
{
	if (InBackgroundIndex == BackgroundIndex)
	{
		return;
	}

	switch (InBackgroundIndex)
	{
		case 0:	SetBackground("WhiteRoom");	break;
		case 1:	SetBackground("WhiteRoom_SunSet");	break;
	}

	BackgroundIndex = InBackgroundIndex;
}
