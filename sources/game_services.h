#pragma once
#include "debug_ui_system.h"
#include "game_camera.h"
#include "game_renderer.h"
#include "game_statistics.h"
#include "physics_world.h"

class GameServices
{
  public:
    GameServices(sf::RenderWindow& renderWindow);

    void Update(sf::Time elapsedTime);
    void Draw();

    EntityWorld &ModifyWorld() { return _ecsWorld; }
    const GameCamera &GetCamera() const { return _camera; }
    DebugUiSystem &ModifyDebugUi() { return _debugUiSystem; }
    GameStatistics &ModifyStatistics() { return _statistics; }

  private:
    void UpdateGameLogic(sf::Time elapsedTime);
    void UpdateSfmlTransforms();

  private:
    Ref<sf::RenderTarget> _renderTarget;
    EntityWorld _ecsWorld;
    GameStatistics _statistics;
    PhysicsWorld _physicsWorld;
    GameRenderer _renderer;
    GameCamera _camera;
    DebugUiSystem _debugUiSystem;
};
