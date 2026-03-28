#include <iostream>
#include <SFML/Graphics.hpp>

int main() {
    std::cout << "Testing minimal SFML..." << std::endl;
    
    try {
        // Test creating window without showing it
        sf::VideoMode mode = sf::VideoMode::getDesktopMode();
        std::cout << "Desktop mode: " << mode.size.x << "x" << mode.size.y << std::endl;
        
        sf::RenderWindow window(sf::VideoMode({100, 100}), "Test", sf::Style::None);
        std::cout << "Window created successfully!" << std::endl;
        
        window.close();
        std::cout << "SFML test PASSED!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "SFML test FAILED: " << e.what() << std::endl;
        return 1;
    }
}
