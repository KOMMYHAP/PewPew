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

struct SpriteComponent {
    sf::CircleShape shape;
};

struct DrawableComponent {
    const sf::Drawable *drawable;
};

void Update(EntityWorld &world, sf::Time elapsedTime, int32_t frameNumber) {
    const float elapsedSeconds = elapsedTime.asSeconds();
    const float animationKey = std::sinf(static_cast<float>(frameNumber) * elapsedSeconds);
    world.view<PositionComponent, const VelocityComponent>()
            .each([elapsedSeconds, animationKey](PositionComponent &pos, const VelocityComponent vel) {
                pos.x += vel.dx * elapsedSeconds * animationKey;
                pos.y += vel.dy * elapsedSeconds * animationKey;
            });
    world.view<const PositionComponent, SpriteComponent>()
            .each([](const PositionComponent position, SpriteComponent &sprite) {
                sprite.shape.setPosition({position.x, position.y});
            });
}

void Render(EntityWorld &world, sf::RenderTarget &renderTarget) {
    world.view<const DrawableComponent>()
            .each([&renderTarget](const DrawableComponent &data) {
                renderTarget.draw(*data.drawable);
            });
}

int main() {
    const uint32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator{seed};

    const sf::Vector2u windowSize{800, 600};
    EntityWorld world;

    std::uniform_real_distribution positionDistribution{0.0f, static_cast<float>(windowSize.y)};
    std::uniform_real_distribution velocityDistribution{-100.0f, 100.0f};
    std::uniform_int_distribution colorDistribution{0, 255};
    std::uniform_real_distribution radiusDistribution{10.0f, 50.0f};
    for (auto i = 0u; i < 10u; ++i) {
        const auto entity = world.create();
        world.emplace<PositionComponent>(entity, positionDistribution(generator), positionDistribution(generator));
        world.emplace<VelocityComponent>(entity, velocityDistribution(generator), velocityDistribution(generator));

        SpriteComponent &sprite = world.emplace<SpriteComponent>(entity);
        sf::Color color = sf::Color::White;
        color.r = colorDistribution(generator);
        color.g = colorDistribution(generator);
        color.b = colorDistribution(generator);
        sprite.shape.setFillColor(color);
        sprite.shape.setRadius(radiusDistribution(generator));

        world.emplace<DrawableComponent>(entity, static_cast<const sf::Drawable*>(&sprite.shape));
    }

    sf::RenderWindow window{sf::VideoMode({800u, 600u}), "PewPew"};
    window.setFramerateLimit(60);

    sf::Clock frameClock;
    int32_t frameNumber{0};
    while (window.isOpen()) {
        frameNumber += 1;
        const sf::Time elapsedTime = frameClock.restart();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        Update(world, elapsedTime, frameNumber);

        window.clear();
        Render(world, window);
        window.display();
    }
    return 0;
}
