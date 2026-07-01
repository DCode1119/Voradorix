#include "NovelScene.h"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

CVrdxNovelScene::CVrdxNovelScene()
{
}

void CVrdxNovelScene::OnEnter()
{
	ScriptEngine.SetNovelScene(shared_from_this());
	if (!ScriptEngine.LoadScript("Assets/Scripts/TestScript.txt"))
	{
		return;
	}
}

void CVrdxNovelScene::OnExit()
{
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
		if (DialogueBox.IsTyping())
		{
			// Complete typing, wait for user input.
			DialogueBox.FinishTyping();
			bWaitingInput = true;
		}
		else
		{
			// Go next script.
			bWaitingInput = false;
		}
	}
}

void CVrdxNovelScene::Update(const float DeltaTick)
{
	ScriptEngine.Update(DeltaTick);
	Background.Update(DeltaTick);
	CharacterManager.Update(DeltaTick);
	DialogueBox.Update(DeltaTick);

	if (RemainingWaitSeconds > 0.f)
	{
		RemainingWaitSeconds = std::max(0.f, RemainingWaitSeconds - DeltaTick);
	}

	bWaitingInput = DialogueBox.IsFinished();
}

void CVrdxNovelScene::Draw(sf::RenderWindow& Window)
{
	Window.clear(sf::Color::Black);

	Background.Draw(Window);
	CharacterManager.Draw(Window);
	DialogueBox.Draw(Window);
}

bool CVrdxNovelScene::CanAdvance() const
{
	return !DialogueBox.IsTyping() && !(RemainingWaitSeconds > 0) && !bWaitingInput;
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

void CVrdxNovelScene::SetDialogue(const FVrdxDialogueLine& DialogueLine)
{
	DialogueBox.SetSpeaker(DialogueLine.Speaker);
	DialogueBox.SetLine(DialogueLine.Text);
}

void CVrdxNovelScene::JumpToLabel(const FVrdxString& TargetLabelName)
{
	ScriptEngine.JumpToLabel(TargetLabelName);
}

void CVrdxNovelScene::WaitForSeconds(const float Seconds)
{
	RemainingWaitSeconds = Seconds;
}

void CVrdxNovelScene::EndScenario()
{
	RequestExit();
}
