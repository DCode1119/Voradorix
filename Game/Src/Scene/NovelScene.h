#pragma once

#include "Core/Vector.h"
#include "Novel/DialogueLine.h"
#include "Scene/Scene.h"
#include "Ui/DialogueBox.h"

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

private:
	void ShowNextLine();
	void EndScenario();

	TVrdxVector<FVrdxDialogueLine> Script;
	int32_t CurrentIndex;
	CVrdxDialogueBox DialogeBox;
};
