#pragma once

#include <cstdint>
#include <memory>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Core/Common.h"
#include "Core/Vector.h"

class CVrdxScene;

class CVrdxSceneManager
{
public:
	CVrdxSceneManager() VRDX_DEFAULT;
	~CVrdxSceneManager() VRDX_DEFAULT;

	CVrdxSceneManager(const CVrdxSceneManager&) VRDX_NO_COPY;
	CVrdxSceneManager& operator=(const CVrdxSceneManager&) VRDX_NO_COPY;

	void Push(TVrdxSharedPtr<CVrdxScene> Scene);
	void Pop();
	void Switch(TVrdxSharedPtr<CVrdxScene> Scene);

	void HandleEvent(const sf::Event& Event);
	void Update(const float DeltaTick);

	void Draw(sf::RenderWindow& Window);

	VRDX_NO_DISCARD bool IsEmpty() const;
	VRDX_NO_DISCARD int32_t GetCount() const;

private:
	TVrdxVector<TVrdxSharedPtr<CVrdxScene>> SceneStack;
};
