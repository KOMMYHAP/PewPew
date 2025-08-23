#include "game_services.h"

#include "components/sfml_components.h"
#include "components/core_components.h"

namespace {
    constexpr sf::Vector2f MapSize{10000.0f, 10000.0f};
}

GameServices::GameServices(sf::Vector2f windowSize)
    : _mapRect{MapSize / -2.0f, MapSize}
    , _renderer{_world}
    , _camera{_world, windowSize}
{
}

void GameServices::Update(sf::Time elapsedTime) {
    std::println("FPS = {:.2f}", 1.0f / elapsedTime.asSeconds());

    _totalTime += elapsedTime;
    UpdateGameLogic(elapsedTime);
    UpdateSfmlTransforms();

    std::fflush(stdout);
}

void GameServices::Render(sf::RenderTarget &target) {
    _renderer.Draw(_camera, target);
}

void GameServices::UpdateGameLogic(sf::Time elapsedTime) {
    const float elapsedSeconds = elapsedTime.asSeconds();
    const auto inputEvents = _world.storage<InputEventComponent>().each();

    auto MoveSystem = [elapsedSeconds](PositionComponent &position, const VelocityComponent vel) {
        position.position += vel.delta * elapsedSeconds;
    };

    auto MakeImpulseForStoppedEntitySystem =
            [](VelocityComponent &vel, const InitialSpeedComponent initialVelocity) {
        if (vel.delta.lengthSquared() <= 1.f) {
            vel.delta = initialVelocity.delta;
        }
    };

    const sf::FloatRect mapBoundaries = _mapRect;
    auto MapCollisionSystem = [mapBoundaries](PositionComponent &pos, VelocityComponent &vel,
                                              const BoundingBoxComponent &bbox) {
        const sf::Vector2f center = pos.position;
        if (mapBoundaries.contains(center)) {
            return;
        }

        const float minX = mapBoundaries.position.x;
        const float minY = mapBoundaries.position.y;
        const float maxX = mapBoundaries.position.x + mapBoundaries.size.x;
        const float maxY = mapBoundaries.position.y + mapBoundaries.size.y;

        std::optional<sf::Vector2f> wallNormal;
        if (center.x <= minX) {
            wallNormal = {-1.0f, 0.0f};
        } else if (center.x >= maxX) {
            wallNormal = {1.0f, 0.0f};
        } else if (center.y <= minY) {
            wallNormal = {0.0f, 1.0f};
        } else if (center.y >= maxY) {
            wallNormal = {0.0f, -1.0f};
        }
        if (wallNormal.has_value()) {
            const sf::Vector2f newVel =
                    vel.delta -
                    2.0f * wallNormal->dot(vel.delta) * *wallNormal;
            static constexpr float Attenuation = 0.9f;
            vel.delta = newVel * Attenuation;
        }

        const float x = std::min(maxX, std::max(minX, center.x));
        const float y = std::min(maxY, std::max(minY, center.y));
        pos.position = sf::Vector2f{x, y};
    };
    auto MoveControlSystem = [&inputEvents](MoveControlComponent &move) {
        for (auto &&[_, input]: inputEvents) {
            const sf::Event &event = input.event;
            if (const sf::Event::KeyPressed *keyPressed = event.getIf<sf::Event::KeyPressed>(); keyPressed) {
                const sf::Keyboard::Key code = keyPressed->code;
                move.activeDirections[MoveControlComponent::Left] |= code == sf::Keyboard::Key::A;
                move.activeDirections[MoveControlComponent::Right] |= code == sf::Keyboard::Key::D;
                move.activeDirections[MoveControlComponent::Up] |= code == sf::Keyboard::Key::W;
                move.activeDirections[MoveControlComponent::Down] |= code == sf::Keyboard::Key::S;
            }
            if (const sf::Event::KeyReleased *keyReleased = event.getIf<sf::Event::KeyReleased>(); keyReleased) {
                const sf::Keyboard::Key code = keyReleased->code;
                move.activeDirections[MoveControlComponent::Left] &= code != sf::Keyboard::Key::A;
                move.activeDirections[MoveControlComponent::Right] &= code != sf::Keyboard::Key::D;
                move.activeDirections[MoveControlComponent::Up] &= code != sf::Keyboard::Key::W;
                move.activeDirections[MoveControlComponent::Down] &= code != sf::Keyboard::Key::S;
            }
        }
    };
    auto ApplyMoveControlSystem = [](const MoveControlComponent control, const MoveSpeedComponent speed,
                                     VelocityComponent &vel) {
        vel = {};
        if (control.activeDirections[MoveControlComponent::Left]) {
            vel.delta.x -= speed.value.x;
        }
        if (control.activeDirections[MoveControlComponent::Right]) {
            vel.delta.x += speed.value.x;
        }
        if (control.activeDirections[MoveControlComponent::Up]) {
            vel.delta.y -= speed.value.y;
        }
        if (control.activeDirections[MoveControlComponent::Down]) {
            vel.delta.y += speed.value.y;
        }
    };

    _world.view<MoveControlComponent>().each(MoveControlSystem);
    _world.clear<InputEventComponent>();
    _world.view<const MoveControlComponent, const MoveSpeedComponent, VelocityComponent>().each(ApplyMoveControlSystem);

    _world.view<VelocityComponent, const InitialSpeedComponent>().each(MakeImpulseForStoppedEntitySystem);
    _world.view<PositionComponent, const VelocityComponent>().each(MoveSystem);
    _world.view<PositionComponent, VelocityComponent, BoundingBoxComponent>().each(MapCollisionSystem);
}

void GameServices::UpdateSfmlTransforms() {
    auto ApplyPositionSystem = [](const PositionComponent position, const SfmlTransformableComponent &transform) {
        transform.transform->setPosition(position.position);
    };
    auto ApplyRotationSystem = [](const RotationComponent &rotation, const SfmlTransformableComponent &transform) {
        transform.transform->setRotation(sf::radians(rotation.radians));
    };
    auto ApplyScaleSystem = [](const ScaleComponent &scale, const SfmlTransformableComponent &transform) {
        transform.transform->setScale(scale.factor);
    };
    auto ApplyBoundingBoxForCircleSystem = [](const BoundingBoxComponent &bbox, CircleShapeComponent &circle) {
        const float radius = bbox.size.length() / 2.0f;
        circle.shape.setRadius(radius);
    };
    auto ApplyBoundingBoxForRectangleSystem = [](const BoundingBoxComponent &bbox, RectangleShapeComponent &rectangle) {
        rectangle.shape.setSize(bbox.size);
    };
    auto ApplyFillColorForCircleSystem = [](const FillColorComponent &color, CircleShapeComponent &rectangle) {
        rectangle.shape.setFillColor(color.color);
    };
    auto ApplyFillColorForRectangleSystem = [](const FillColorComponent &color, RectangleShapeComponent &rectangle) {
        rectangle.shape.setFillColor(color.color);
    };
    auto UpdateOriginSystem = [](const BoundingBoxComponent &bbox, const SfmlTransformableComponent &transform) {
        transform.transform->setOrigin(bbox.size / 2.0f);
    };
    auto UpdateCameraPositionSystem = [](const PositionComponent &position, SfmlViewComponent &view) {
        view.view.setCenter(position.position);
    };
    auto UpdateCameraSizeSystem = [](const BoundingBoxComponent &bbox, SfmlViewComponent &view) {
        view.view.setSize(bbox.size);
    };

    _world.view<const PositionComponent, const SfmlTransformableComponent>().each(ApplyPositionSystem);
    _world.view<const RotationComponent, const SfmlTransformableComponent>().each(ApplyRotationSystem);
    _world.view<const ScaleComponent, const SfmlTransformableComponent>().each(ApplyScaleSystem);
    _world.view<const BoundingBoxComponent, CircleShapeComponent>().each(ApplyBoundingBoxForCircleSystem);
    _world.view<const BoundingBoxComponent, RectangleShapeComponent>().each(ApplyBoundingBoxForRectangleSystem);
    _world.view<const FillColorComponent, CircleShapeComponent>().each(ApplyFillColorForCircleSystem);
    _world.view<const FillColorComponent, RectangleShapeComponent>().each(ApplyFillColorForRectangleSystem);
    _world.view<const InvalidatedBoundingBoxTag, const BoundingBoxComponent, SfmlTransformableComponent>().each(
        UpdateOriginSystem);
    _world.view<const PositionComponent, SfmlViewComponent>().each(UpdateCameraPositionSystem);
    _world.view<const InvalidatedBoundingBoxTag, const BoundingBoxComponent, SfmlViewComponent>().each(
        UpdateCameraSizeSystem);

    _world.clear<InvalidatedBoundingBoxTag>();
}
