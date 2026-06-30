#pragma once

#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"

#include "../Core/Common.h"

class CVrdxScene
{
public:
	virtual ~CVrdxScene() VRDX_DEFAULT;

	virtual void OnEnter() VRDX_PURE_VIRTUAL;
	virtual void OnExit() VRDX_PURE_VIRTUAL;
	virtual void HandleEvent(const sf::Event&) VRDX_PURE_VIRTUAL;
	virtual void Update(const float DeltaTick) VRDX_PURE_VIRTUAL;
	virtual void Draw(sf::RenderWindow&) VRDX_PURE_VIRTUAL;

	bool WantsExit() const { return bWantExit; }

protected:
	void RequestExit() { bWantExit = true; }

private:
	bool bWantExit = false;
};
