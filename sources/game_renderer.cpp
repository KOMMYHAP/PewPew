#include "game_renderer.h"

#include "components/core_components.h"
#include "components/physics_components.h"
#include "components/sfml_components.h"

GameRenderer::GameRenderer(EntityWorld &ecsWorld, PhysicsWorld & physicsWorld, sf::RenderTarget& renderTarget)
    : _ecsWorld(&ecsWorld)
    , _physicsWorld(&physicsWorld)
    , _renderTarget(&renderTarget)
{
}

void GameRenderer::Draw(const GameCamera &camera)
{
    const sf::FloatRect viewSpace = camera.GetViewSpace();
    auto CameraCullingSystem = [viewSpace, this](Entity entity, const PhysicsBody body, const SfmlDrawableComponent) {
        if (!viewSpace.contains(_physicsWorld->GetPosition(body.id)))
        {
            return;
        }
        _ecsWorld->emplace<IsVisibleInCamera>(entity);
    };
    auto DrawVisibleSystem = [this](const SfmlDrawableComponent drawable) {
        _renderTarget->draw(*drawable.drawable);
    };

    // cleanup culling result from previous frame
    _ecsWorld->clear<IsVisibleInCamera>();

    // apply culling for the new frame
    _ecsWorld->view<const PhysicsBody, const SfmlDrawableComponent>().each(CameraCullingSystem);

    // draw visible
    const auto visibleItems = _ecsWorld->view<const IsVisibleInCamera, const SfmlDrawableComponent>();
    _drawCalls = visibleItems.size_hint();
    visibleItems.each(DrawVisibleSystem);
}
