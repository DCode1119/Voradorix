#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>

int main()
{
    // 800x600 크기의 창 생성
    sf::RenderWindow Window(sf::VideoMode({800, 600}), "SFML Game");

    // FPS 제한 - 창모드는 60으로 설정
    Window.setFramerateLimit(60);

    // 초록색 원 생성
    sf::CircleShape Shape(100.f);
    Shape.setFillColor(sf::Color::Green);
    Shape.setPosition({300.f, 200.f});

    sf::Clock Clock;
    int32_t FrameCount = 0;

    std::cout << "SFML " << SFML_VERSION_MAJOR << "." << SFML_VERSION_MINOR
              << "." << SFML_VERSION_PATCH << " window opened successfully!"
              << std::endl;

    while (Window.isOpen())
    {
        // 이벤트 처리
        while (const auto OptionalEvent = Window.pollEvent())
        {
            const auto& Event = OptionalEvent.value();
            if (Event.is<sf::Event::Closed>())
            {
                Window.close();
            }
            else if (const auto* KeyPressed = Event.getIf<sf::Event::KeyPressed>())
            {
                if (KeyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    Window.close();
                }
            }
        }

        // 원을 회전 (애니메이션)
        static float Angle = 0.f;
        Angle += 0.5f;
        Shape.setRotation(sf::degrees(Angle));

        // 그리기
        Window.clear(sf::Color::Black);
        Window.draw(Shape);
        Window.display();

        // FPS 출력
        FrameCount++;
        if (Clock.getElapsedTime().asSeconds() >= 1.f)
        {
            std::cout << "FPS: " << FrameCount << std::endl;
            FrameCount = 0;
            Clock.restart();
        }
    }

    return 0;
}
