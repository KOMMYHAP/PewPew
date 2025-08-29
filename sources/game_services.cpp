#include "game_services.h"

#include "components/core_components.h"
#include "components/physics_components.h"
#include "components/sfml_components.h"
#include "debug_ui/menu/summary_widget.h"

GameServices::GameServices(sf::RenderWindow &renderWindow)
    : _renderTarget{&renderWindow}
    , _physicsWorld{_ecsWorld, _statistics}
    , _renderer{_ecsWorld, _physicsWorld, *_renderTarget}
    , _camera{_ecsWorld, sf::Vector2f{0.0f, 0.0f}, sf::Vector2f{25.0f, 25.0f}}
    , _debugUiSystem{renderWindow}
{
    _debugUiSystem.ModifyMenu().AddWidget<GameStatisticsWidget>("Statistics", _statistics);
}

void GameServices::Update(sf::Time elapsedTime)
{
    _statistics.Update(elapsedTime);
    _debugUiSystem.Update(elapsedTime);
    _physicsWorld.Update(elapsedTime);
    UpdateGameLogic(elapsedTime);
    UpdateSfmlTransforms();
}

void GameServices::Draw()
{
    _renderer.Draw(_camera);
    _debugUiSystem.Draw();
}

void GameServices::UpdateGameLogic(sf::Time /*elapsedTime*/)
{
    const auto inputEvents = _ecsWorld.storage<InputEventComponent>().each();

    auto WatchTargetRotationSystem = [](const PhysicsBody bodyFrom, const WatchTargetComponent &postitionTo, RotationComponent &rotation) {
        b2Vec2 positionFrom = b2Body_GetPosition(bodyFrom.id);
        const float fromX = positionFrom.x;
        const float fromY = positionFrom.y;
        const float fromUpX = fromX;
        const float fromUpY = fromY + 1.0f;
        const sf::Vector2f from{fromUpX - fromX, fromUpY - fromY};

        const float toStartX = fromX;
        const float toStartY = fromY;
        const float toEndX = postitionTo.position.x;
        const float toEndY = postitionTo.position.y;
        const sf::Vector2f to{toEndX - toStartX, toEndY - toStartY};
        if (to == sf::Vector2f{})
        {
            return;
        }

        const sf::Angle rotationAngle = from.angleTo(to);
        rotation.angle = rotationAngle;
    };

    auto MoveSystem = [](const PhysicsBody &body, const MoveDirectionComponent direction, const MoveSpeedComponent speed) {
        const sf::Vector2f velocity = direction.delta * speed.value;
        b2Body_SetLinearVelocity(body.id, b2Vec2{velocity.x, velocity.y});
    };

    auto MoveControlSystem = [&inputEvents](MoveControlComponent &move) {
        for (auto &&[_, input] : inputEvents)
        {
            const sf::Event &event = input.event;
            if (const sf::Event::KeyPressed *keyPressed = event.getIf<sf::Event::KeyPressed>(); keyPressed)
            {
                const sf::Keyboard::Key code = keyPressed->code;
                move.activeDirections[MoveControlComponent::Left] |= code == sf::Keyboard::Key::A;
                move.activeDirections[MoveControlComponent::Right] |= code == sf::Keyboard::Key::D;
                move.activeDirections[MoveControlComponent::Up] |= code == sf::Keyboard::Key::W;
                move.activeDirections[MoveControlComponent::Down] |= code == sf::Keyboard::Key::S;
            }
            if (const sf::Event::KeyReleased *keyReleased = event.getIf<sf::Event::KeyReleased>(); keyReleased)
            {
                const sf::Keyboard::Key code = keyReleased->code;
                move.activeDirections[MoveControlComponent::Left] &= code != sf::Keyboard::Key::A;
                move.activeDirections[MoveControlComponent::Right] &= code != sf::Keyboard::Key::D;
                move.activeDirections[MoveControlComponent::Up] &= code != sf::Keyboard::Key::W;
                move.activeDirections[MoveControlComponent::Down] &= code != sf::Keyboard::Key::S;
            }
        }
    };
    auto ApplyMoveControlSystem = [](const MoveControlComponent control, MoveDirectionComponent &vel) {
        sf::Vector2f direction{};
        if (control.activeDirections[MoveControlComponent::Left])
        {
            direction.x = -1.0f;
        }
        if (control.activeDirections[MoveControlComponent::Right])
        {
            direction.x = 1.0f;
        }
        if (control.activeDirections[MoveControlComponent::Up])
        {
            direction.y = -1.0f;
        }
        if (control.activeDirections[MoveControlComponent::Down])
        {
            direction.y = 1.0f;
        }
        vel.delta = direction;
    };

    _ecsWorld.view<MoveControlComponent>().each(MoveControlSystem);
    _ecsWorld.clear<InputEventComponent>();
    _ecsWorld.view<const MoveControlComponent, MoveDirectionComponent>().each(ApplyMoveControlSystem);
    _ecsWorld.view<const PhysicsBody, const MoveDirectionComponent, const MoveSpeedComponent>().each(MoveSystem);
    _ecsWorld.view<const PhysicsBody, const WatchTargetComponent, RotationComponent>().each(WatchTargetRotationSystem);
}

void GameServices::UpdateSfmlTransforms()
{
    auto ApplyPhysicsSystem = [](const PhysicsBody body, const SfmlTransformableComponent &transform) {
        const b2Vec2 position = b2Body_GetPosition(body.id);
        transform.transform->setPosition(sf::Vector2f{position.x, position.y});
    };
    auto ApplyRotationSystem = [](const RotationComponent &rotation, const SfmlTransformableComponent &transform) {
        transform.transform->setRotation(rotation.angle);
    };
    auto ApplyScaleSystem = [](const ScaleComponent &scale, const SfmlTransformableComponent &transform) {
        transform.transform->setScale(scale.factor);
    };
    auto ApplyBoundingBoxForCircleSystem = [this](const PhysicsBody &body, CircleShapeComponent &circle) {
        const float radius = _physicsWorld.GetBoundingBox(body.id).length() / 2.0f;
        circle.shape.setRadius(radius);
    };
    auto ApplyBoundingBoxForRectangleSystem = [this](const PhysicsBody &body, RectangleShapeComponent &rectangle) {
        rectangle.shape.setSize(_physicsWorld.GetBoundingBox(body.id));
    };
    auto ApplyFillColorForCircleSystem = [](const FillColorComponent &color, CircleShapeComponent &rectangle) {
        rectangle.shape.setFillColor(color.color);
    };
    auto ApplyFillColorForRectangleSystem = [](const FillColorComponent &color, RectangleShapeComponent &rectangle) {
        rectangle.shape.setFillColor(color.color);
    };
    auto UpdateOriginSystem = [this](const PhysicsBody &body, const SfmlTransformableComponent &transform) {
        transform.transform->setOrigin(_physicsWorld.GetBoundingBox(body.id) / 2.0f);
    };
    auto UpdateCameraPositionSystem = [this](const PhysicsBody &body, SfmlViewComponent &view) {
        view.view.setCenter(_physicsWorld.GetPosition(body.id));
    };
    auto UpdateCameraSizeSystem = [this](const PhysicsBody &body, SfmlViewComponent &view) {
        view.view.setSize(_physicsWorld.GetBoundingBox(body.id));
    };

    _ecsWorld.view<const PhysicsBody, const SfmlTransformableComponent>().each(ApplyPhysicsSystem);
    _ecsWorld.view<const PhysicsBody, CircleShapeComponent>().each(ApplyBoundingBoxForCircleSystem);
    _ecsWorld.view<const PhysicsBody, RectangleShapeComponent>().each(ApplyBoundingBoxForRectangleSystem);
    _ecsWorld.view<const RotationComponent, const SfmlTransformableComponent>().each(ApplyRotationSystem);
    _ecsWorld.view<const ScaleComponent, const SfmlTransformableComponent>().each(ApplyScaleSystem);
    _ecsWorld.view<const FillColorComponent, CircleShapeComponent>().each(ApplyFillColorForCircleSystem);
    _ecsWorld.view<const FillColorComponent, RectangleShapeComponent>().each(ApplyFillColorForRectangleSystem);
    _ecsWorld.view<const InvalidatedBoundingBoxTag, const PhysicsBody, SfmlTransformableComponent>().each(UpdateOriginSystem);
    _ecsWorld.view<const PhysicsBody, SfmlViewComponent>().each(UpdateCameraPositionSystem);
    _ecsWorld.view<const InvalidatedBoundingBoxTag, const PhysicsBody, SfmlViewComponent>().each(UpdateCameraSizeSystem);

    _ecsWorld.clear<InvalidatedBoundingBoxTag>();
}
