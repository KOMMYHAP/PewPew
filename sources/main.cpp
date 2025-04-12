#include <random>
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

using EntityWorld = entt::registry;

struct PositionComponent {
    float x;
    float y;
};

struct VelocityComponent {
    float dx;
    float dy;
};

struct InitialVelocityComponent {
    float dx;
    float dy;
};

struct SpriteComponent {
    sf::CircleShape shape;
};

struct DrawableComponent {
    const sf::Drawable *drawable;
};

struct AABBComponent {
    sf::FloatRect rect;
};

void Update(EntityWorld &world, sf::Vector2f mapSize, sf::Time gameElapsedTime, sf::Time frameElapsedTime) {
    const float elapsedSeconds = frameElapsedTime.asSeconds();
    const float animationKey = std::sinf(gameElapsedTime.asSeconds());

    auto MoveSystem = [elapsedSeconds](AABBComponent &aabb, const VelocityComponent vel) {
        aabb.rect.position.x += vel.dx * elapsedSeconds;
        aabb.rect.position.y += vel.dy * elapsedSeconds;
    };

    auto MakeImpulseForStoppedEntitySystem =
            [](VelocityComponent &vel, const InitialVelocityComponent initialVelocity) {
        if (sf::Vector2f{vel.dx, vel.dy}.lengthSquared() <= 1.f) {
            vel.dx = initialVelocity.dx;
            vel.dy = initialVelocity.dy;
        }
    };

    auto UpdateSpritePositionSystem = [](const PositionComponent position, SpriteComponent &sprite) {
        sprite.shape.setPosition({position.x, position.y});
    };

    const auto colorOffset = static_cast<uint8_t>(std::lerp(0.0f, 255.0f, animationKey * 0.01f));
    auto UpdateSpriteColorSystem = [colorOffset](SpriteComponent &sprite) {
        sf::Color color = sprite.shape.getFillColor();
        color.r += colorOffset;
        color.g += colorOffset;
        color.b += colorOffset;
        sprite.shape.setFillColor(color);
    };
    auto MapCollisionSystem = [mapSize](AABBComponent &aabb, VelocityComponent &vel) {
        const sf::Vector2f offsetToPosition = aabb.rect.size / -2.0f;
        const sf::Vector2f center = aabb.rect.getCenter();

        std::optional<sf::Vector2f> wallNormal;
        if (center.x <= 0.0f) {
            wallNormal = {-1.0f, 0.0f};
        } else if (center.x >= mapSize.x) {
            wallNormal = {1.0f, 0.0f};
        } else if (center.y <= 0.0f) {
            wallNormal = {0.0f, 1.0f};
        } else if (center.y >= mapSize.y) {
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

        const float x = std::min(mapSize.x, std::max(0.0f, center.x));
        const float y = std::min(mapSize.y, std::max(0.0f, center.y));
        aabb.rect.position.x = offsetToPosition.x + x;
        aabb.rect.position.y = offsetToPosition.y + y;
    };
    auto UpdatePositionSystem = [](PositionComponent &position, const AABBComponent &aabb) {
        position.x = aabb.rect.position.x;
        position.y = aabb.rect.position.y;
    };

    world.view<VelocityComponent, const InitialVelocityComponent>().each(MakeImpulseForStoppedEntitySystem);
    world.view<AABBComponent, const VelocityComponent>().each(MoveSystem);
    world.view<AABBComponent, VelocityComponent>().each(MapCollisionSystem);
    world.view<PositionComponent, const AABBComponent>().each(UpdatePositionSystem);
    world.view<const PositionComponent, SpriteComponent>().each(UpdateSpritePositionSystem);
    world.view<SpriteComponent>().each(UpdateSpriteColorSystem);
}

void Render(EntityWorld &world, sf::RenderTarget &renderTarget) {
    auto DrawSystem = [&renderTarget](const DrawableComponent &data) {
        renderTarget.draw(*data.drawable);
    };

    world.view<const DrawableComponent>().each(DrawSystem);
}

int main() {
    const uint32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator{seed};

    const sf::Vector2f windowSize{800.0f, 600.0f};
    EntityWorld world;

    std::uniform_real_distribution positionXDistribution{0.0f, windowSize.x};
    std::uniform_real_distribution positionYDistribution{0.0f, windowSize.y};
    std::uniform_real_distribution velocityDistribution{-200.0f, 200.0f};
    std::uniform_int_distribution colorDistribution{0, 255};
    std::uniform_real_distribution radiusDistribution{3.0f, 10.0f};
    for (int i = 0; i < 1000; ++i) {
        const auto entity = world.create();
        const PositionComponent position = world.emplace<PositionComponent>(
            entity, positionXDistribution(generator), positionYDistribution(generator));
        const VelocityComponent velocity = world.emplace<VelocityComponent>(
            entity, velocityDistribution(generator), velocityDistribution(generator));
        world.emplace<InitialVelocityComponent>(entity, velocity.dx, velocity.dy);

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

    sf::RenderWindow window{sf::VideoMode(static_cast<sf::Vector2u>(windowSize)), "PewPew"};
    window.setFramerateLimit(60);

    sf::Clock gameClock;
    sf::Clock frameClock;
    while (window.isOpen()) {
        const sf::Time frameTime = frameClock.restart();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        Update(world, windowSize, gameClock.getElapsedTime(), frameTime);

        window.clear();
        Render(world, window);
        window.display();
    }
    return 0;
}
