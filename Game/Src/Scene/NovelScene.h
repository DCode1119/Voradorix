#pragma once

#include "Core/Vector.h"
#include "Novel/Background.h"
#include "Novel/CharacterManager.h"
#include "Novel/DialogueLine.h"
#include "Novel/ScriptEngine.h"
#include "Scene/Scene.h"
#include "Ui/DialogueBox.h"

class CVrdxNovelScene
	: public CVrdxScene
	, public std::enable_shared_from_this<CVrdxNovelScene>
{
public:
	CVrdxNovelScene();
	~CVrdxNovelScene() VRDX_DEFAULT;

	virtual void OnEnter() VRDX_OVERRIDE;
	virtual void OnExit() VRDX_OVERRIDE;
	virtual void HandleEvent(const sf::Event&) VRDX_OVERRIDE;
	virtual void Update(const float DeltaTick) VRDX_OVERRIDE;
	virtual void Draw(sf::RenderWindow&) VRDX_OVERRIDE;

	bool CanAdvance() const;

	void SetBackground(const FVrdxString& BackgroundName);
	void ShowCharacter(const FVrdxString& Character, const EVrdxCharacterPosition Position);
	void HideCharacter(const FVrdxString& Character);
	void SetCharacterPose(const FVrdxString& Character, const FVrdxString& Pose);
	void SetDialogue(const FVrdxDialogueLine& DialogueLine);

	void JumpToLabel(const FVrdxString& TargetLabelName);
	void WaitForSeconds(const float Seconds);

private:
	void EndScenario();


	bool bWaitingInput = false;

	CVrdxBackground Background;
	CVrdxCharacterManager CharacterManager;
	CVrdxDialogueBox DialogueBox;
	CVrdxScriptEngine ScriptEngine;

	float RemainingWaitSeconds = 0.f;
};
