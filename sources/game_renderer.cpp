#include "game_renderer.h"


GameRenderer::GameRenderer(const GameMap &gameMap, const EntityWorld &entities)
    : _gameMap(&gameMap),
      _entities(&entities) {
}

void GameRenderer::Draw(sf::RenderTarget &renderTarget) {
    _drawableList.clear();
    _entities->view<const AABBComponent, const DrawableComponent>().each(
        [this](const AABBComponent &aabb, const DrawableComponent &drawable) {
            if (!_gameMap->GetViewSpace().contains(aabb.rect.getCenter())) {
                return;
            }

            _drawableList.push_back(drawable.drawable);
        });

    drawCalls = _drawableList.size();
    renderTarget.setView(_gameMap->GetView());
    for (const sf::Drawable *drawable: _drawableList) {
        renderTarget.draw(*drawable);
    }
}
