#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Core/Common.h"
#include "Scene/SceneManager.h"

class CVrdxApplication
{
public:
	CVrdxApplication();
	~CVrdxApplication();

	void Run();

private:
	void HandleEvents();
	void Update(const float DeltaTick);
	void Draw();

	sf::RenderWindow Window;
	CVrdxSceneManager SceneManager;
	sf::Clock Clock;
	bool bIsRunning;
};
