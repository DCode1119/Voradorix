// Copyright DCode. All Rights Reserved.
#pragma once

// Third-party Library
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

// Project Headers
#include "Core/Common.h"
#include "Ui/WidgetBase.h"

class CVrdxApplication : public CVrdxWidgetBase
{
public:
	CVrdxApplication(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	~CVrdxApplication();

	void Run();

	void OnPostCreate() override;

protected:
	void Save() const;
	void Load();

private:
	void HandleEvents();
	virtual void Draw(sf::RenderWindow& RenderWindow) const override;

	sf::RenderWindow Window;
	sf::Clock Clock;
	bool bIsRunning;

	bool bCtrl = false;
};
