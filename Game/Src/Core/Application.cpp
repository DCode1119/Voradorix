#include "Application.h"

#include "Scene/TestScene.h"
#include "Scene/NovelScene.h"

CVrdxApplication::CVrdxApplication()
{
	Window.create(sf::VideoMode({ 1280, 720 }), "Voradorix");
	Window.setVerticalSyncEnabled(true);
	Window.setFramerateLimit(60);
	bIsRunning = true;
	SceneManager.Push(MakeVrdxUnique<CVrdxTestScene>());
	SceneManager.Push(MakeVrdxUnique<CVrdxNovelScene>());
}

CVrdxApplication::~CVrdxApplication()
{

}

void CVrdxApplication::Run()
{
	while (bIsRunning && Window.isOpen())
	{
		float DeltaTick = Clock.restart().asSeconds();
		HandleEvents();
		Update(DeltaTick);
		Draw();

		if (SceneManager.IsEmpty())
		{
			bIsRunning = false;
		}
	}
}

void CVrdxApplication::HandleEvents()
{
	while (const auto OptionalEvent = Window.pollEvent())
	{
		const auto& Event = OptionalEvent.value();
		if (Event.is<sf::Event::Closed>())
		{
			bIsRunning = false;
		}

		SceneManager.HandleEvent(Event);
	}
}

void CVrdxApplication::Update(const float DeltaTick)
{
	SceneManager.Update(DeltaTick);
}

void CVrdxApplication::Draw()
{
	Window.clear(sf::Color::Black);
	SceneManager.Draw(Window);
	Window.display();
}
