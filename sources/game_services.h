#pragma once
#include "game_camera.h"
#include "game_renderer.h"

class GameServices
{
  public:
    GameServices(sf::Vector2f windowSize);

    void Update(sf::Time elapsedTime);
    void Render(sf::RenderTarget &target);

    EntityWorld &ModifyWorld() { return _world; }
    const GameCamera &GetCamera() const { return _camera; }

  private:
    void UpdateGameLogic(sf::Time elapsedTime);
    void UpdateSfmlTransforms();

  private:
    sf::FloatRect _mapRect;
    sf::Time _totalTime;
    EntityWorld _world;
    GameRenderer _renderer;
    GameCamera _camera;
};
