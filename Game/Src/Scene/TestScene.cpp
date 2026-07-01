#include "Scene/TestScene.h"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"

void CVrdxTestScene::OnEnter()
{

}

void CVrdxTestScene::OnExit()
{

}

void CVrdxTestScene::HandleEvent(const sf::Event& Event)
{
	if (const auto* KeyPressed = Event.getIf<sf::Event::KeyPressed>())
	{
		if (KeyPressed->scancode == sf::Keyboard::Scancode::Escape)
		{
			RequestExit();
		}
	}
}

void CVrdxTestScene::Update(const float DeltaTick)
{

}

void CVrdxTestScene::Draw(sf::RenderWindow& Window)
{
	sf::Vector2u WindowSize = Window.getSize();
	
	constexpr float ShapeRadius = 100.f;
	sf::CircleShape Shape(ShapeRadius);

	Shape.setOrigin({ ShapeRadius, ShapeRadius });
	Shape.setFillColor(sf::Color::Blue);
	Shape.setPosition({ WindowSize.x / 2.f, WindowSize.y / 2.f});
	Window.draw(Shape);
}
