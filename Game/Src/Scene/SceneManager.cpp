#include "Scene/SceneManager.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Scene/Scene.h"
#include "NovelScene.h"


void CVrdxSceneManager::Push(TVrdxSharedPtr<CVrdxScene> Scene)
{
	Scene->OnEnter();
	SceneStack.Add(VrdxMove(Scene));
}

void CVrdxSceneManager::Pop()
{
	if (!SceneStack.IsEmpty())
	{
		SceneStack.Last()->OnExit();
		SceneStack.Pop();
	}
}

void CVrdxSceneManager::Switch(TVrdxSharedPtr<CVrdxScene> Scene)
{
	Pop();

	Push(VrdxMove(Scene));
}

void CVrdxSceneManager::HandleEvent(const sf::Event& Event)
{
	if (!SceneStack.IsEmpty())
	{
		SceneStack.Last()->HandleEvent(Event);
	}

	if (!SceneStack.IsEmpty() && SceneStack.Last()->WantsExit())
	{
		Pop();
	}
}

void CVrdxSceneManager::Update(const float DeltaTick)
{
	if (!SceneStack.IsEmpty())
	{
		SceneStack.Last()->Update(DeltaTick);
	}
}

void CVrdxSceneManager::Draw(sf::RenderWindow& Window) const
{
	if (!SceneStack.IsEmpty())
	{
		SceneStack.Last()->Draw(Window);
	}
}

bool CVrdxSceneManager::IsEmpty() const
{
	return SceneStack.IsEmpty();
}

int32_t CVrdxSceneManager::GetCount() const
{
	return SceneStack.Num();
}

void CVrdxSceneManager::Save() const
{
	if (const auto NovelScene = dynamic_cast<const CVrdxNovelScene*>(SceneStack.Last().get()))
	{
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
}

void CVrdxSceneManager::Load()
{
	if (auto NovelScene = dynamic_cast<CVrdxNovelScene*>(SceneStack.Last().get()))
	{
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
}
