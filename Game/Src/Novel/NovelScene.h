// Copyright DCode. All Rights Reserved.
#pragma once

// Third-party Library
#include <nlohmann/json.hpp>

// Project Headers
#include "Core/Vector.h"
#include "Novel/ScriptEngine.h"

struct FVrdxString;
struct FVrdxDialogueLine;
struct FVrdxChoiceOption;

class CVrdxBackground;
class CVrdxCharacterManager;
class CVrdxChoiceWidget;
class CVrdxDialogueBox;

struct FVrdxNovelSceneSaveData
{
	FVrdxString ScriptPath;
	int32_t CurrentLine = 0;
	FVrdxString BackgroundName;
	TVrdxVector<FVrdxCharacterSlotSaveData> CharacterSlots;

	FVrdxString ToJson() const;
	void FromJson(const FVrdxString& String);
};

class CVrdxNovelScene : public CVrdxWidgetBase
{
public:
	CVrdxNovelScene(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

	virtual void OnPostCreate() override;
	virtual void OnPreDestroy() override;
	virtual void Update(const float DeltaTick) VRDX_OVERRIDE;
	virtual bool OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode);

	bool CanAdvance() const;

	void SetBackground(const FVrdxString& BackgroundName);
	void ShowCharacter(const FVrdxString& Character, const EVrdxCharacterPosition Position);
	void HideCharacter(const FVrdxString& Character);
	void SetCharacterPose(const FVrdxString& Character, const FVrdxString& Pose);
	void SetDialogue(const FVrdxDialogueLine& DialogueLine);
	void SetChoices(const TVrdxVector<FVrdxChoiceOption>& ChoiceOptions);

	void JumpToLabel(const FVrdxString& TargetLabelName);
	void WaitForSeconds(const float Seconds);

	FVrdxNovelSceneSaveData Save() const;
	void Save(const FVrdxString& Filename) const;
	void Load(const FVrdxNovelSceneSaveData& SaveData);
	void Load(const FVrdxString& Filename);

private:
	void EndScenario();

	TVrdxSharedPtr<CVrdxBackground> Background;
	TVrdxSharedPtr<CVrdxCharacterManager> CharacterManager;
	TVrdxSharedPtr<CVrdxDialogueBox> DialogueBox;
	TVrdxSharedPtr<CVrdxChoiceWidget> ChoiceWidget;

	CVrdxScriptEngine ScriptEngine;

	float RemainingWaitSeconds = 0.f;
};
