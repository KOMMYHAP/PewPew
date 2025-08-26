#pragma once
#include "game_camera.h"
#include "physics_world.h"

class GameRenderer
{
  public:
    GameRenderer(EntityWorld &ecsWorld, PhysicsWorld & physicsWorld, sf::RenderTarget& renderTarget);

    void Draw(const GameCamera &camera);

    int32_t GetDrawCalls() const { return _drawCalls; }

  private:
    Ref<EntityWorld> _ecsWorld;
    Ref<PhysicsWorld> _physicsWorld;
    Ref<sf::RenderTarget> _renderTarget;
    int32_t _drawCalls{0};
};
