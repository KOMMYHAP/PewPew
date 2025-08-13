#include <random>
#include <print>

#include "game_map.h"
#include "components.h"
#include "game_renderer.h"


void Update(EntityWorld &world, Entity camera, GameMap &map, sf::Time gameElapsedTime,
            sf::Time frameElapsedTime) {
    const float elapsedSeconds = frameElapsedTime.asSeconds();
    const float animationKey = std::sinf(gameElapsedTime.asSeconds());
    const auto inputEvents = world.storage<InputEventComponent>().each();

    bool ChangeColorBalls = false;

    auto DampingSystem = [elapsedSeconds](VelocityComponent& vel) {
        constexpr float dampingPerSecond = 0.8f;
        const float damping = std::pow(dampingPerSecond, elapsedSeconds);
        vel.dx *= damping;
        vel.dy *= damping;
        };

    auto MoveSystem = [elapsedSeconds](PositionComponent& position, const VelocityComponent vel) {
        position.x += vel.dx * elapsedSeconds;
        position.y += vel.dy * elapsedSeconds;
        };

    auto MakeImpulseForStoppedEntitySystem =
            [](VelocityComponent &vel, const InitialSpeedComponent initialVelocity) {
        if (sf::Vector2f{vel.dx, vel.dy}.lengthSquared() <= 1.f) {
            vel.dx = initialVelocity.dx;
            vel.dy = initialVelocity.dy;
        }
    };

    auto UpdateSpritePositionSystem = [](const PositionComponent position, SpriteComponent &sprite) {
        sprite.shape.setPosition({position.x, position.y});
    };

    const auto colorOffset = static_cast<uint8_t>(std::lerp(0.0f, 255.0f, animationKey * 0.01f));
    auto UpdateSpriteColorSystem = [colorOffset](SpriteComponent& sprite) {
        sf::Color color = sprite.shape.getFillColor();
        color.r += colorOffset;
        color.g += colorOffset;
        color.b += colorOffset;
        sprite.shape.setFillColor(color);
    };

    const sf::FloatRect mapBoundaries = map.GetBoundaries();
    auto MapCollisionSystem = [mapBoundaries
            ](PositionComponent &pos, VelocityComponent &vel, AABBComponent &aabb) {
        const sf::Vector2f center = aabb.rect.getCenter();
        if (mapBoundaries.contains(center)) {
            return;
        }

        const sf::Vector2f offsetToPosition = aabb.rect.size / -2.0f;
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
                    sf::Vector2f{vel.dx, vel.dy} -
                    2.0f * wallNormal->dot({vel.dx, vel.dy}) * *wallNormal;
            static constexpr float Attenuation = 0.9f;
            vel.dx = newVel.x * Attenuation;
            vel.dy = newVel.y * Attenuation;
        }

        const float x = std::min(maxX, std::max(minX, center.x));
        const float y = std::min(maxY, std::max(minY, center.y));
        pos.x = offsetToPosition.x + x;
        pos.y = offsetToPosition.y + y;
        aabb.rect.position = {pos.x, pos.y};
    };
    auto UpdateAABBSystem = [](AABBComponent &aabb, PositionComponent position) {
        aabb.rect.position = {position.x, position.y};
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
            vel.dx -= speed.value.x;
        }
        if (control.activeDirections[MoveControlComponent::Right]) {
            vel.dx += speed.value.x;
        }
        if (control.activeDirections[MoveControlComponent::Up]) {
            vel.dy -= speed.value.y;
        }
        if (control.activeDirections[MoveControlComponent::Down]) {
            vel.dy += speed.value.y;
        }
    };

    world.view<MoveControlComponent>().each(MoveControlSystem);
    world.clear<InputEventComponent>();
    world.view<const MoveControlComponent, const MoveSpeedComponent, VelocityComponent>().each(ApplyMoveControlSystem);

    world.view<VelocityComponent, const InitialSpeedComponent>().each(MakeImpulseForStoppedEntitySystem);
    world.view<PositionComponent, const VelocityComponent>().each(MoveSystem);
    world.view<AABBComponent, const PositionComponent>().each(UpdateAABBSystem);
    world.view<PositionComponent, VelocityComponent, AABBComponent>().each(MapCollisionSystem);
    world.view<const PositionComponent, SpriteComponent>().each(UpdateSpritePositionSystem);
    world.view<SpriteComponent>().each(UpdateSpriteColorSystem);

    if (const PositionComponent *position = world.try_get<PositionComponent>(camera)) {
        const sf::Vector2f newCenter = sf::Vector2f{position->x, position->y} + map.GetViewSpace().size / 2.0f;
        map.ModifyView().setCenter(newCenter);
    }
}


int main() {
    static constexpr sf::Vector2f WindowSize{1920.0f, 1080.0f};
    static constexpr sf::Vector2f MapSize{10000.0f, 10000.0f};
    GameMap map{{MapSize / -2.0f, MapSize}};
    sf::View &view = map.ModifyView();
    view.setCenter({0.0f, 0.0f});
    view.setSize(WindowSize);

    EntityWorld world;
    GameRenderer renderer{map, world};

    const uint32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator{seed};

    std::uniform_real_distribution positionXDistribution{
        map.GetBoundaries().position.x, map.GetBoundaries().position.x + map.GetBoundaries().size.x
    };
    std::uniform_real_distribution positionYDistribution{
        map.GetBoundaries().position.y, map.GetBoundaries().position.y + map.GetBoundaries().size.y
    };
    std::uniform_real_distribution velocityDistribution{-200.0f, 200.0f};
    std::uniform_int_distribution colorDistribution{0, 155};
    std::uniform_real_distribution radiusDistribution{3.0f, 10.0f};


    // Setup spheres
    for (int i = 0; i < 10000; ++i) {
        const auto entity = world.create();
        const PositionComponent position = world.emplace<PositionComponent>(
            entity, positionXDistribution(generator), positionYDistribution(generator));
        const VelocityComponent velocity = world.emplace<VelocityComponent>(
            entity, velocityDistribution(generator), velocityDistribution(generator));
        world.emplace<InitialSpeedComponent>(entity, velocity.dx, velocity.dy);

        SpriteComponent &sprite = world.emplace<SpriteComponent>(entity);
        sf::Color color = sf::Color::White;
        color.r = colorDistribution(generator);
        color.g = colorDistribution(generator);
        color.b = colorDistribution(generator);
        sprite.shape.setPointCount(10);
        sprite.shape.setFillColor(color);
        sprite.shape.setRadius(radiusDistribution(generator));

        world.emplace<DrawableComponent>(entity, static_cast<const sf::Drawable *>(&sprite.shape));

        const sf::FloatRect aabb{{position.x, position.y}, {sprite.shape.getRadius(), sprite.shape.getRadius()}};
        world.emplace<AABBComponent>(entity, aabb);
    }


    // Setup camera
    const auto camera = world.create();
    world.emplace<MoveControlComponent>(camera, MoveControlComponent{});
    world.emplace<MoveSpeedComponent>(camera, sf::Vector2f{500.0f, 500.0f});
    world.emplace<VelocityComponent>(camera, 0.0f, 0.0f);
    world.emplace<PositionComponent>(camera, view.getCenter().x, view.getCenter().y);
    world.emplace<AABBComponent>(camera, sf::FloatRect{view.getCenter() - view.getSize() / 2.0f, view.getSize()});

    sf::RenderWindow window{sf::VideoMode(static_cast<sf::Vector2u>(WindowSize)), "PewPew"};
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Clock frameClock;
    uint64_t gameTime{0};
    while (window.isOpen()) {
        const sf::Time frameTime = frameClock.restart();
        std::println("FPS = {:.2f}", 1.0f / frameTime.asSeconds());
        gameTime += frameTime.asMicroseconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            const auto entity = world.create();
            world.emplace<InputEventComponent>(entity, *std::move(event));
        }

        Update(world, camera, map, sf::microseconds(gameTime), frameTime);

        window.clear();
        renderer.Draw(window);
        window.display();

        std::fflush(stdout);
    }
    return 0;
}
