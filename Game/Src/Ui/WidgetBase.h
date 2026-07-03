#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "Core/Common.h"

class CVrdxBaseWidget
{
public:
	virtual ~CVrdxBaseWidget() VRDX_DEFAULT;

	virtual void HandleEvent(const sf::Event& Event) VRDX_PURE_VIRTUAL;
	virtual void Update(const float DeltaTick) VRDX_PURE_VIRTUAL;
	virtual void Draw(sf::RenderWindow& Window) const VRDX_PURE_VIRTUAL;
};
