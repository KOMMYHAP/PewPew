#pragma once
#include "game_camera.h"


class GameRenderer {
public:
    GameRenderer(EntityWorld &entities);

    void Draw(const GameCamera& camera, sf::RenderTarget &renderTarget);

    int32_t GetDrawCalls() const { return _drawCalls; }

private:
    Ref<EntityWorld> _world;
    int32_t _drawCalls{0};
};
