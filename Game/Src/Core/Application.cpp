#include "Core/Application.h"

#include "Scene/NovelScene.h"
#include "Scene/TestScene.h"

TVrdxSharedPtr<CVrdxWidgetBase> CVrdxApplication::Instance;

CVrdxApplication::CVrdxApplication(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
{
	Window.create(sf::VideoMode(sf::Vector2u(InShape.getSize())), "Voradorix");
	Window.setVerticalSyncEnabled(true);
	Window.setFramerateLimit(60);
}

CVrdxApplication::~CVrdxApplication()
{
	
}

TVrdxWeakPtr<CVrdxWidgetBase> CVrdxApplication::GetRootWidget()
{
	return Instance;
}

void CVrdxApplication::Run()
{
	while (bIsRunning && Window.isOpen())
	{
		float DeltaTick = Clock.restart().asSeconds();
		HandleEvents();
		Update(DeltaTick);
		Draw(Window);

		if (SceneManager.IsEmpty())
		{
			bIsRunning = false;
		}
	}
}

void CVrdxApplication::OnPostCreate()
{
	Instance = shared_from_this();
	SceneManager.Push(MakeVrdxShared<CVrdxNovelScene>());
	bIsRunning = true;
}

void CVrdxApplication::OnPreDestroy()
{
	while (!SceneManager.IsEmpty())
	{
		SceneManager.Pop();
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
		CVrdxWidgetBase::HandleEvent(Event);
	}
}

void CVrdxApplication::Update(const float DeltaTick)
{
	SceneManager.Update(DeltaTick);
	CVrdxWidgetBase::Update(DeltaTick);
}

void CVrdxApplication::Draw(sf::RenderWindow& RenderWindow) const
{
	RenderWindow.clear(sf::Color::Black);
	SceneManager.Draw(RenderWindow);
	CVrdxWidgetBase::Draw(RenderWindow);
	RenderWindow.display();
}
