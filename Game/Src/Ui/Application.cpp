// Copyright DCode. All Rights Reserved.

#include "Ui/Application.h"

// C++ Standard Library
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

// Project Headers
#include "Novel/NovelScene.h"

CVrdxApplication::CVrdxApplication(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxWidgetBase(ParentWidget, InShape)
	, bCtrl(false)
{
	Window.create(sf::VideoMode(sf::Vector2u(InShape.getSize())), "Voradorix");
	Window.setVerticalSyncEnabled(true);
	Window.setFramerateLimit(60);
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
		Draw(Window);
	}
}

void CVrdxApplication::OnPostCreate()
{
	auto Panel = Shape;
	Panel.setFillColor(sf::Color::Transparent);
	Panel.setOutlineColor(sf::Color::Transparent);
	CVrdxWidgetBase::CreateWidget<CVrdxNovelScene>(shared_from_this(), Panel);

	bIsRunning = true;
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
	}
}

void CVrdxApplication::Draw(sf::RenderWindow& RenderWindow) const
{
	RenderWindow.clear(sf::Color::Black);
	CVrdxWidgetBase::Draw(RenderWindow);
	RenderWindow.display();
}

void CVrdxApplication::Save() const
{
	TVrdxSharedPtr<const CVrdxNovelScene> NovelScene;
	for (const auto Child : Children)
	{
		NovelScene = dynamic_pointer_cast<const CVrdxNovelScene>(Child);
		if (NovelScene)
		{
			break;
		}
	}

	if (!NovelScene)
	{
		return;
	}

	FVrdxNovelSceneSaveData SaveData = NovelScene->Save();
	FVrdxString String = SaveData.ToJson();

	//Write into "Saves/Save0.dat" from String.
	std::filesystem::create_directories("Saves");
	std::ofstream File("Saves/Save0.dat");
	if (File.is_open())
	{
		File << String.ToUtf8();
	}
}

void CVrdxApplication::Load()
{
	TVrdxSharedPtr<CVrdxNovelScene> NovelScene;
	for (auto Child : Children)
	{
		NovelScene = dynamic_pointer_cast<CVrdxNovelScene>(Child);
		if (NovelScene)
		{
			break;
		}
	}

	if (!NovelScene)
	{
		return;
	}

	//Read from "Saves/Save0.dat" into String.
	FVrdxString String;
	std::ifstream File("Saves/Save0.dat");
	if (File.is_open())
	{
		std::stringstream Buffer;
		Buffer << File.rdbuf();
		String = Buffer.str();

		FVrdxNovelSceneSaveData SaveData;
		SaveData.FromJson(String);

		NovelScene->Load(SaveData);
	}
}
