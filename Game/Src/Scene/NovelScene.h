#pragma once

#include "Core/Vector.h"
#include "Novel/DialogueLine.h"
#include "Scene/Scene.h"
#include "Ui/DialogueBox.h"
#include "Novel/Background.h"
#include "Novel/CharacterManager.h"

class CVrdxNovelScene : public CVrdxScene
{
public:
	CVrdxNovelScene();
	~CVrdxNovelScene() VRDX_DEFAULT;

	virtual void OnEnter() VRDX_OVERRIDE;
	virtual void OnExit() VRDX_OVERRIDE;
	virtual void HandleEvent(const sf::Event&) VRDX_OVERRIDE;
	virtual void Update(const float DeltaTick) VRDX_OVERRIDE;
	virtual void Draw(sf::RenderWindow&) VRDX_OVERRIDE;

	void SetBackground(const FVrdxString& BackgroundName);
	void ShowCharacter(const FVrdxString& Character, const EVrdxCharacterPosition Position);
	void HideCharacter(const FVrdxString& Character);
	void SetCharacterPose(const FVrdxString& Character, const FVrdxString& Pose);

private:
	void ShowNextLine();
	void EndScenario();

	void SwitchBackground(int32_t BackgroundIndex);

	TVrdxVector<FVrdxDialogueLine> Script;
	int32_t BackgroundIndex;
	int32_t CurrentIndex;
	CVrdxBackground Background;
	CVrdxCharacterManager CharacterManager;
	CVrdxDialogueBox DialogeBox;
};
