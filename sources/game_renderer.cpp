#include "game_renderer.h"

#include "components/core_components.h"
#include "components/sfml_components.h"

GameRenderer::GameRenderer(EntityWorld &entities)
    : _world(&entities) {
}

void GameRenderer::Draw(const GameCamera &camera, sf::RenderTarget &renderTarget) {
    const sf::FloatRect viewSpace = camera.GetViewSpace();
    auto CameraCullingSystem = [viewSpace, this](Entity entity, const PositionComponent position) {
        if (!viewSpace.contains(position.position)) {
            return;
        }
        _world->emplace<IsVisibleInCamera>(entity);
    };
    auto DrawVisibleSystem = [&renderTarget](const SfmlDrawableComponent drawable) {
        renderTarget.draw(*drawable.drawable);
    };

    // cleanup culling result from previous frame
    _world->clear<IsVisibleInCamera>();

    // apply culling for the new frame
    _world->view<const PositionComponent>().each(CameraCullingSystem);

    // draw visible
    const auto visibleItems = _world->view<const IsVisibleInCamera, const SfmlDrawableComponent>();
    _drawCalls = visibleItems.size_hint();
    visibleItems.each(DrawVisibleSystem);
}
