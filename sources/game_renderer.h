#pragma once
#include "game_map.h"
#include "components.h"


class GameRenderer {
public:
    GameRenderer(const GameMap &gameMap, const EntityWorld &entities);

    void Draw(sf::RenderTarget &renderTarget);

    int32_t GetDrawCalls() const { return drawCalls; }

private:
    Ref<const GameMap *> _gameMap;
    Ref<const EntityWorld *> _entities;
    std::vector<const sf::Drawable *> _drawableList;
    int32_t drawCalls{0};
};
