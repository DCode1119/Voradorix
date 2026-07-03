#include "NovelScene.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#include "Core/String.h"
#include "Novel/ChoiceManager.h"
#include "Novel/DialogueLine.h"

CVrdxNovelScene::CVrdxNovelScene()
{
}

void CVrdxNovelScene::OnEnter()
{
	ScriptEngine.SetNovelScene(shared_from_this());
	ChoiceManager.SetNovelScene(shared_from_this());
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
	// Dialogue typing state
	if (const bool bIsWaiting = DialogueBox.IsWaiting())
	{
		DialogueBox.HandleEvent(Event);
	}

	// Choice wait state
	if (const bool bIsWaiting = ChoiceManager.IsWaiting())
	{
		ChoiceManager.HandleEvent(Event);
	}
}

void CVrdxNovelScene::Update(const float DeltaTick)
{
	ScriptEngine.Update(DeltaTick);
	Background.Update(DeltaTick);
	CharacterManager.Update(DeltaTick);
	DialogueBox.Update(DeltaTick);
	ChoiceManager.Update(DeltaTick);

	if (RemainingWaitSeconds > 0.f)
	{
		RemainingWaitSeconds = std::max(0.f, RemainingWaitSeconds - DeltaTick);
	}
}

void CVrdxNovelScene::Draw(sf::RenderWindow& Window)
{
	Window.clear(sf::Color::Black);

	Background.Draw(Window);
	CharacterManager.Draw(Window);
	DialogueBox.Draw(Window);
	ChoiceManager.Draw(Window);
}

bool CVrdxNovelScene::CanAdvance() const
{
	return !ChoiceManager.IsWaiting() && !DialogueBox.IsWaiting() && !(RemainingWaitSeconds > 0);
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

void CVrdxNovelScene::SetChoices(const TVrdxVector<FVrdxChoiceOption>& ChoiceOptions)
{
	ChoiceManager.SetChoices(ChoiceOptions);
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
