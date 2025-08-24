#pragma once

class GameCamera
{
  public:
    GameCamera(EntityWorld &world, sf::Vector2f position, sf::Vector2f cameraSize);

    sf::FloatRect GetViewSpace() const;
    const sf::View &GetView() const { return *_view; }

  private:
    const sf::View *_view;
    Ref<EntityWorld> _world;
};
