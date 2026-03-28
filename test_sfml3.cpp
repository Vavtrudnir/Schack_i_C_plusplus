#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode({640, 720}), "Test");
    sf::Event event;
    while (const std::optional e = window.pollEvent()) {
        if (e->is<sf::Event::Closed>()) {
            window.close();
        } else if (const auto* mp = e->getIf<sf::Event::MouseButtonPressed>()) {
            if (mp->button == sf::Mouse::Button::Left) {
                std::cout << mp->position.x << "," << mp->position.y << "\n";
            }
        }
    }
    
    sf::RectangleShape shape({10.f, 10.f});
    shape.setPosition({5.f, 5.f});
    return 0;
}
