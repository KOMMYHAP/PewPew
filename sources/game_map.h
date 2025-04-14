#pragma once

class GameMap {
public:
    explicit GameMap(const sf::FloatRect &boundaries);

    const sf::FloatRect &GetBoundaries() const { return _boundaries; }

    sf::View &ModifyView() { return _view; }
    const sf::View &GetView() const { return _view; }

    sf::FloatRect GetViewSpace() const;

private:
    sf::View _view;
    sf::FloatRect _boundaries;
};
