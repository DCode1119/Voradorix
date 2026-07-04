#include "Scene/SceneManager.h"

#include <algorithm>

#include "Scene/Scene.h"


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
