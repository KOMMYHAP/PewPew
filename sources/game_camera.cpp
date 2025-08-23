#include "game_camera.h"

#include "components/core_components.h"
#include "components/sfml_components.h"

GameCamera::GameCamera(EntityWorld &world, sf::Vector2f windowSize)
    : _world(&world)
{
    const Entity camera = _world->create();
    _world->emplace<PositionComponent>(camera, windowSize / 2.0f);
    _world->emplace<BoundingBoxComponent>(camera, windowSize);
    _world->emplace<InvalidatedBoundingBoxTag>(camera);
    SfmlViewComponent& view = _world->emplace<SfmlViewComponent>(camera);
    _view = &view.view;
}


sf::FloatRect GameCamera::GetViewSpace() const {
    return sf::FloatRect{_view->getCenter() - _view->getSize() / 2.0f, _view->getSize()};
}
