// Copyright DCode. All Rights Reserved.
#pragma once

// C++ Standard Library
#include <functional>

// Third-party Library
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

// Project Headers
#include "Core/Common.h"
#include "Ui/WidgetBase.h"


class CVrdxApplication : public CVrdxWidgetBase
{
public:
	using VRDX_Initializer = std::function<void(TVrdxWeakPtr<CVrdxWidgetBase>& Parent)>;

	// Dedicated 
	CVrdxApplication(const sf::Vector2f& Size);

	// WidgetBase inherited.
	CVrdxApplication(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

	//Initializer should be called after generation.
	void Initialize(VRDX_Initializer&& InCallback);

	// Main loop
	void Run();

private:
	void HandleEvents();

	sf::RenderWindow Window;
	sf::Clock Clock;
	bool bIsRunning = true;
	bool bInitialized = false;

	//bool bCtrl = false;
};
