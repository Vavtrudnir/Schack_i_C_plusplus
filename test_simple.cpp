#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    std::cout << "Testing SFML..." << std::endl;
    
    try {
        sf::RenderWindow window(sf::VideoMode({200, 200}), "Test");
        std::cout << "Window created successfully!" << std::endl;
        
        sf::CircleShape shape(50.f);
        shape.setFillColor(sf::Color::Green);
        
        while (window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }
            
            window.clear();
            window.draw(shape);
            window.display();
            
            // Just run for a short time
            static int count = 0;
            count++;
            if (count > 100) {
                window.close();
            }
        }
        
        std::cout << "Test completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
