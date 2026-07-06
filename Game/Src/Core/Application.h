#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "Core/Common.h"
#include "Scene/SceneManager.h"
#include "Ui/WidgetBase.h"

class CVrdxApplication : public CVrdxWidgetBase
{
public:
	CVrdxApplication(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	~CVrdxApplication();

	static TVrdxWeakPtr<CVrdxWidgetBase> GetRootWidget();

	void Run();

	//virtual bool HandleEvent(const sf::Event& Event) override { return true; }

	void OnPostCreate() override;
	void OnPreDestroy() override;

private:
	void HandleEvents();
	virtual void Update(const float DeltaTick) override;
	virtual void Draw(sf::RenderWindow& RenderWindow) const override;

	sf::RenderWindow Window;
	CVrdxSceneManager SceneManager;
	sf::Clock Clock;
	bool bIsRunning;

	static TVrdxSharedPtr<CVrdxWidgetBase> Instance;

	bool bCtrl = false;
};
