#include "NovelScene.h"

#include <nlohmann/json.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#include "Core/Application.h"
#include "Core/String.h"
#include "Novel/DialogueLine.h"
#include "Ui/ChoiceWidget.h"
#include "Ui/DialogueBox.h"

CVrdxNovelScene::CVrdxNovelScene()
{
	sf::RectangleShape Panel;
	Panel.setSize(sf::Vector2f(1200.f, 200.f));
	Panel.setPosition(sf::Vector2f(40.f, 500.f));
	Panel.setFillColor(sf::Color(20, 20, 20, 220));
	DialogueBox = CVrdxWidgetBase::CreateWidget<CVrdxDialogueBox>(CVrdxApplication::GetRootWidget(), Panel);

	Panel.setSize(sf::Vector2f(1200.f, 200.f));
	Panel.setPosition(sf::Vector2f(40.f, 500.f));
	Panel.setFillColor(sf::Color(20, 20, 20, 220));
	ChoiceWidget = CVrdxWidgetBase::CreateWidget<CVrdxChoiceWidget>(CVrdxApplication::GetRootWidget(), Panel);
	ChoiceWidget->Clear();
}

CVrdxNovelScene::~CVrdxNovelScene()
{
}

void CVrdxNovelScene::OnEnter()
{
	ScriptEngine.SetNovelScene(shared_from_this());
	ChoiceWidget->SetNovelScene(shared_from_this());
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

}

void CVrdxNovelScene::Update(const float DeltaTick)
{
	ScriptEngine.Update(DeltaTick);
	Background.Update(DeltaTick);
	CharacterManager.Update(DeltaTick);

	if (RemainingWaitSeconds > 0.f)
	{
		RemainingWaitSeconds = std::max(0.f, RemainingWaitSeconds - DeltaTick);
	}
}

void CVrdxNovelScene::Draw(sf::RenderWindow& Window) const
{
	Background.Draw(Window);
	CharacterManager.Draw(Window);
}

bool CVrdxNovelScene::CanAdvance() const
{
	return !ChoiceWidget->IsWaiting() && !DialogueBox->IsWaiting() && !(RemainingWaitSeconds > 0);
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
	DialogueBox->SetSpeaker(DialogueLine.Speaker);
	DialogueBox->SetLine(DialogueLine.Text);
}

void CVrdxNovelScene::SetChoices(const TVrdxVector<FVrdxChoiceOption>& ChoiceOptions)
{
	ChoiceWidget->SetChoices(ChoiceOptions);
}

void CVrdxNovelScene::JumpToLabel(const FVrdxString& TargetLabelName)
{
	ScriptEngine.JumpToLabel(TargetLabelName);
}

void CVrdxNovelScene::WaitForSeconds(const float Seconds)
{
	RemainingWaitSeconds = Seconds;
}

FVrdxNovelSceneSaveData CVrdxNovelScene::Save() const
{
	return FVrdxNovelSceneSaveData
		{
			.ScriptPath = ScriptEngine.GetScriptName(),
			.CurrentLine = ScriptEngine.GetCurrentScriptLine() - 1,
			.BackgroundName = Background.GetCurrentAssetName(),
			.CharacterSlots = CharacterManager.GetSaveData(),
		};
}

void CVrdxNovelScene::Load(const FVrdxNovelSceneSaveData& SaveData)
{
	if (!ScriptEngine.LoadScript(SaveData.ScriptPath))
	{
		return;
	}

	CharacterManager.Reset();
	Background.SetBackground(SaveData.BackgroundName);
	for (const auto& CharacterSlot : SaveData.CharacterSlots)
	{
		CharacterManager.ShowCharacter(CharacterSlot.CharacterName, CharacterSlot.Position);
		CharacterManager.SetCharacterPose(CharacterSlot.CharacterName, CharacterSlot.CharacterPose);
	}

	ScriptEngine.JumpToLine(SaveData.CurrentLine);
}

void CVrdxNovelScene::EndScenario()
{
	RequestExit();
}

FVrdxString FVrdxNovelSceneSaveData::ToJson() const
{
	nlohmann::json Json;
	Json["ScriptPath"] = ScriptPath.ToUtf8();
	Json["CurrentLine"] = CurrentLine;
	Json["BackgroundName"] = BackgroundName.ToUtf8();
	for (const auto& Slot : CharacterSlots)
	{
		nlohmann::json SlotData;
		SlotData["CharacterName"] = Slot.CharacterName.ToUtf8();
		SlotData["PoseName"] = Slot.CharacterPose.ToUtf8();
		SlotData["Position"] = (int32_t)Slot.Position;
		Json["Characters"].push_back(SlotData);
	}

	return Json.dump(4);
}

void FVrdxNovelSceneSaveData::FromJson(const FVrdxString& String)
{
	try
	{
		auto Json = nlohmann::json::parse(String.ToUtf8());  // FVrdxString→UTF-8
		ScriptPath = Json["ScriptPath"].get<std::string>();
		CurrentLine = Json["CurrentLine"].get<int32_t>();
		BackgroundName = Json["BackgroundName"].get<std::string>();

		CharacterSlots.Clear();
		for (const auto& Item : Json["Characters"])
		{
			FVrdxCharacterSlotSaveData Slot;
			Slot.CharacterName = Item["CharacterName"].get<std::string>();
			Slot.CharacterPose = Item["PoseName"].get<std::string>();
			Slot.Position = static_cast<EVrdxCharacterPosition>(Item["Position"].get<int32_t>());
			CharacterSlots.Add(Slot);
		}
	}
	catch (const std::exception&)
	{
		//
	}
	
}
