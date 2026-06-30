#include "NovelScene.h"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Window/Mouse.hpp"

CVrdxNovelScene::CVrdxNovelScene()
{
	FVrdxDialogueLine DialogueLine =  { "Voradorix", "Voradorix의 세계에 오신 것을 환영합니다." };
	Script.Add(DialogueLine);
	CurrentIndex = 0;
}

void CVrdxNovelScene::OnEnter()
{
	CurrentIndex = 0;
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

	#if 0
	struct MouseButtonPressed
	{
		Mouse::Button button{}; //!< Code of the button that has been pressed
		Vector2i      position; //!< Position of the mouse pointer, relative to the top left of the owner window
	};
	#endif
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
			if (DialogeBox.IsFinished())
			{
				ShowNextLine();
			}
			else
			{
				DialogeBox.FinishTyping();
			}
		}
	}
}

void CVrdxNovelScene::Update(const float DeltaTick)
{
	DialogeBox.Update(DeltaTick);
}

void CVrdxNovelScene::Draw(sf::RenderWindow& Window)
{
	Window.clear(sf::Color::Black);
	DialogeBox.Draw(Window);
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
