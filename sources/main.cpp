#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

int main() {
    sf::RenderWindow window{sf::VideoMode({800u, 600u}), "CMake SFML Project"};
    window.setFramerateLimit(144);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();
        window.display();
    }
    return 0;
}
