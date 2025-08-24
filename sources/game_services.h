#pragma once
#include "game_camera.h"
#include "game_renderer.h"
#include "physics_world.h"

class GameServices
{
  public:
    GameServices(sf::Vector2f windowSize);

    void Update(sf::Time elapsedTime);
    void Render(sf::RenderTarget &target);

    EntityWorld &ModifyWorld() { return _ecsWorld; }
    const GameCamera &GetCamera() const { return _camera; }

  private:
    void UpdateGameLogic(sf::Time elapsedTime);
    void UpdateSfmlTransforms();

  private:
    sf::Time _totalTime;
    EntityWorld _ecsWorld;
    PhysicsWorld _physicsWorld;
    GameRenderer _renderer;
    GameCamera _camera;
};
