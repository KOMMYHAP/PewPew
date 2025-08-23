#pragma once

class GameCamera {
public:
    GameCamera(EntityWorld& world, sf::Vector2f windowSize);

    sf::FloatRect GetViewSpace() const;
    const sf::View& GetView() const { return *_view; }

private:
    const sf::View* _view;
    Ref<EntityWorld> _world;
};
