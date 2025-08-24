#pragma once
#include "game_camera.h"
#include "physics_world.h"

class GameRenderer
{
  public:
    GameRenderer(EntityWorld &ecsWorld, PhysicsWorld & physicsWorld);

    void Draw(const GameCamera &camera, sf::RenderTarget &renderTarget);

    int32_t GetDrawCalls() const { return _drawCalls; }

  private:
    Ref<EntityWorld> _ecsWorld;
    Ref<PhysicsWorld> _physicsWorld;
    int32_t _drawCalls{0};
};
