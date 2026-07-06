// Copyright DCode. All Rights Reserved.

#include "Ui/Application.h"

// Project Headers
#include "Ui/WidgetBase.h"

CVrdxApplication::CVrdxApplication(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
{
	try
	{
		Window.create(sf::VideoMode(sf::Vector2u(InShape.getSize())), "Voradorix");
		Window.setVerticalSyncEnabled(true);
		Window.setFramerateLimit(60);
	}
	catch (std::exception&)
	{
		// Window creation failed; todo: make it notified.
		bIsRunning = false;
	}

	// bCtrl = false;
}


CVrdxApplication::CVrdxApplication(const sf::Vector2f& Size)
	: CVrdxApplication(TVrdxWeakPtr<CVrdxWidgetBase>{}, sf::RectangleShape(Size))
{

}

void CVrdxApplication::Run()
{
	if (!bInitialized)
	{
		// notify error
		return;
	}

	while (bIsRunning && Window.isOpen())
	{
		Window.clear(sf::Color::Black);

		float DeltaTick = Clock.restart().asSeconds();
		HandleEvents();
		Update(DeltaTick);
		Draw(Window);

		Window.display();
	}
}

void CVrdxApplication::Initialize(VRDX_Initializer&& Callback)
{
	if (bInitialized)
	{
		// Display double initialization and do nothing.
		return;
	}

	if (Callback)
	{
		TVrdxWeakPtr<CVrdxWidgetBase> WeakThis = shared_from_this();
		Callback(WeakThis);
	}

	bInitialized = true;
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

		CVrdxWidgetBase::HandleEvent(Event);

#if 0
		if (const auto* Key = Event.getIf<sf::Event::KeyPressed>())
		{
			switch (Key->scancode)
			{
				case sf::Keyboard::Scan::LControl: bCtrl = true; break;
				case sf::Keyboard::Scan::S: if (bCtrl) { Save(); } break;
				case sf::Keyboard::Scan::L: if (bCtrl) { Load(); } break;
			}
		}

		if (const auto* Key = Event.getIf<sf::Event::KeyReleased>())
		{
			if (Key->scancode == sf::Keyboard::Scan::LControl)
			{
				bCtrl = false;
			}
		}
#endif
	}
}
