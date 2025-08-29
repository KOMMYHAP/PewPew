#pragma once
#include "menu_root_widget.h"

class DebugUiSystem final {
public:
    explicit DebugUiSystem(sf::RenderWindow & window);
    ~DebugUiSystem();

    void ProcessInput(const sf::Event &event);
    void Update(sf::Time elapsedTime);
    void Draw();

    MenuWidget& ModifyMenu() { return _menuWidget; }

private:
    Ref<sf::RenderWindow> _window;
    MenuWidget _menuWidget;
};