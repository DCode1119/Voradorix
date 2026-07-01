#pragma once

#include "Scene/Scene.h"

class CVrdxTestScene : public CVrdxScene
{
public:

	virtual void OnEnter() override;
	virtual void OnExit() override;
	virtual void HandleEvent(const sf::Event&) override;
	virtual void Update(const float DeltaTick) override;
	virtual void Draw(sf::RenderWindow&) override;
};
