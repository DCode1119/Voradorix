#pragma once

#include "Core/Vector.h"
#include "Novel/Background.h"
#include "Novel/CharacterManager.h"
#include "Novel/ScriptEngine.h"
#include "Scene/Scene.h"

struct FVrdxString;
struct FVrdxDialogueLine;
struct FVrdxChoiceOption;
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

class CVrdxNovelScene
	: public CVrdxScene
	, public std::enable_shared_from_this<CVrdxNovelScene>
{
public:
	CVrdxNovelScene();
	~CVrdxNovelScene();

	virtual void OnEnter() VRDX_OVERRIDE;
	virtual void OnExit() VRDX_OVERRIDE;
	virtual void HandleEvent(const sf::Event&) VRDX_OVERRIDE;
	virtual void Update(const float DeltaTick) VRDX_OVERRIDE;
	virtual void Draw(sf::RenderWindow&) const VRDX_OVERRIDE;

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
	void Load(const FVrdxNovelSceneSaveData& SaveData);

private:
	void EndScenario();

	CVrdxBackground Background;
	CVrdxCharacterManager CharacterManager;
	TVrdxSharedPtr<CVrdxDialogueBox> DialogueBox;
	CVrdxScriptEngine ScriptEngine;
	//CVrdxChoiceWidget ChoiceManager;
	TVrdxSharedPtr<CVrdxChoiceWidget> ChoiceWidget;

	float RemainingWaitSeconds = 0.f;
};
