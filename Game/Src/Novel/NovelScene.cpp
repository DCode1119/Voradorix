// Copyright DCode. All Rights Reserved.

#include "Novel/NovelScene.h"

// C++ Standard Library
#include <filesystem>
#include <fstream>
#include <sstream>

// Third-party Library
#include <nlohmann/json.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

// Project Headers
#include "Core/String.h"
#include "Novel/Background.h"
#include "Novel/CharacterManager.h"
#include "Novel/ChoiceWidget.h"
#include "Novel/DialogueBox.h"
#include "Novel/DialogueLine.h"
#include "Ui/Application.h"


CVrdxNovelScene::CVrdxNovelScene(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
{

}

void CVrdxNovelScene::OnPostCreate()
{
	TVrdxSharedPtr<CVrdxNovelScene> SharedThis = dynamic_pointer_cast<CVrdxNovelScene>(shared_from_this());
	if (!SharedThis)
	{
		// Critical issue. logger will be inserted here in short future.
		return;
	}

	ScriptEngine.SetNovelScene(SharedThis);
	if (!ScriptEngine.LoadScript("Assets/Scripts/TestScript.txt"))
	{
		return;
	}

	sf::RectangleShape Panel;
	Panel.setSize(sf::Vector2f(1200.f, 200.f));
	Panel.setPosition(sf::Vector2f(40.f, 500.f));
	Panel.setFillColor(sf::Color(20, 20, 20, 220));
	DialogueBox = CVrdxWidgetBase::CreateWidget<CVrdxDialogueBox>(SharedThis, Panel);

	Panel.setSize(sf::Vector2f(1200.f, 200.f));
	Panel.setPosition(sf::Vector2f(40.f, 500.f));
	Panel.setFillColor(sf::Color(20, 20, 20, 220));
	ChoiceWidget = CVrdxWidgetBase::CreateWidget<CVrdxChoiceWidget>(SharedThis, Panel);
	ChoiceWidget->SetNovelScene(SharedThis);
	ChoiceWidget->Clear();

	Panel.setSize({ 1280, 720 });
	Panel.setPosition({ 0,0 });
	Panel.setFillColor(sf::Color::Transparent);
	Panel.setOutlineColor(sf::Color::Transparent);
	CharacterManager= CVrdxWidgetBase::CreateWidget<CVrdxCharacterManager>(SharedThis, Panel);

	Panel.setSize({ 1280, 720 });
	Panel.setPosition({ 0,0 });
	Panel.setFillColor(sf::Color::Transparent);
	Panel.setOutlineColor(sf::Color::Transparent);
	Background = CVrdxWidgetBase::CreateWidget<CVrdxBackground>(SharedThis, Panel);

	//Order widgets to display.
	const TVrdxSharedPtr<CVrdxWidgetBase> LayoutOrder[] =
	{
		Background,
		CharacterManager,
		DialogueBox,
		ChoiceWidget,
	};

	for (auto Widget : LayoutOrder)
	{
		Widget->BringToFront();
	}
}


void CVrdxNovelScene::OnPreDestroy()
{
	// Release all references
	Background = nullptr;
	ChoiceWidget = nullptr;
	DialogueBox = nullptr;
}

void CVrdxNovelScene::Update(const float DeltaTick)
{
	ScriptEngine.Update(DeltaTick);

	if (RemainingWaitSeconds > 0.f)
	{
		RemainingWaitSeconds = std::max(0.f, RemainingWaitSeconds - DeltaTick);
	}

	// Propagate update tick.
	CVrdxWidgetBase::Update(DeltaTick);
}


bool CVrdxNovelScene::OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode)
{
	if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl))
	{
		return false;
	}

	switch (ScanCode)
	{
	case sf::Keyboard::Scan::S: Save("Save0.dat"); return true; break;
	case sf::Keyboard::Scan::L: Load("Save0.dat"); return true; break;
	}

	return false;
}

bool CVrdxNovelScene::CanAdvance() const
{
	return !ChoiceWidget->IsWaiting() && !DialogueBox->IsWaiting() && !(RemainingWaitSeconds > 0);
}

void CVrdxNovelScene::SetBackground(const FVrdxString& BackgroundName)
{
	Background->SetBackground(BackgroundName);
}

void CVrdxNovelScene::ShowCharacter(const FVrdxString& Character, const EVrdxCharacterPosition Position)
{
	CharacterManager->ShowCharacter(Character, Position);
}

void CVrdxNovelScene::HideCharacter(const FVrdxString& Character)
{
	CharacterManager->HideCharacter(Character);
}

void CVrdxNovelScene::SetCharacterPose(const FVrdxString& Character, const FVrdxString& Pose)
{
	CharacterManager->SetCharacterPose(Character, Pose);
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
			.BackgroundName = Background->GetCurrentAssetName(),
			.CharacterSlots = CharacterManager->GetSaveData(),
		};
}


void CVrdxNovelScene::Save(const FVrdxString& Filename) const
{
	FVrdxNovelSceneSaveData SaveData = Save();
	FVrdxString String = SaveData.ToJson();

	//Write into "Saves/Save0.dat" from String.
	std::filesystem::create_directories("Saves");
	std::ofstream File("Saves/" + Filename.ToUtf8());
	if (File.is_open())
	{
		File << String.ToUtf8();
	}
}

void CVrdxNovelScene::Load(const FVrdxNovelSceneSaveData& SaveData)
{
	if (!ScriptEngine.LoadScript(SaveData.ScriptPath))
	{
		return;
	}

	CharacterManager->Reset();
	Background->SetBackground(SaveData.BackgroundName);
	for (const auto& CharacterSlot : SaveData.CharacterSlots)
	{
		CharacterManager->ShowCharacter(CharacterSlot.CharacterName, CharacterSlot.Position);
		CharacterManager->SetCharacterPose(CharacterSlot.CharacterName, CharacterSlot.CharacterPose);
	}

	ScriptEngine.JumpToLine(SaveData.CurrentLine);
}


void CVrdxNovelScene::Load(const FVrdxString& Filename)
{
	//Read from "Saves/Save0.dat" into String.
	FVrdxString String;
	std::ifstream File("Saves/" + Filename.ToUtf8());
	if (File.is_open())
	{
		std::stringstream Buffer;
		Buffer << File.rdbuf();
		String = Buffer.str();

		FVrdxNovelSceneSaveData SaveData;
		SaveData.FromJson(String);

		Load(SaveData);
	}
}


void CVrdxNovelScene::ResetScriptEngine()
{
	if (!ScriptEngine.LoadScript("Assets/Scripts/TestScript.txt"))
	{
		return;
	}
}

void CVrdxNovelScene::EndScenario()
{
	//RequestExit();
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
