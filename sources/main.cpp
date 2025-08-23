#include "game_services.h"
#include "components/core_components.h"
#include "components/sfml_components.h"

int main() {
    static constexpr sf::Vector2f WindowSize{800.0f, 600.0f};
    GameServices gameServices{WindowSize};
    EntityWorld &world = gameServices.ModifyWorld();

    RectangleShapeComponent* spritePlayer =nullptr;
    {
        const auto player = world.create();
        RectangleShapeComponent &sprite = world.emplace<RectangleShapeComponent>(player);
        spritePlayer = &sprite;

        world.emplace<SfmlDrawableComponent>(player, static_cast<sf::Drawable *>(&sprite.shape));
        world.emplace<SfmlTransformableComponent>(player, static_cast<sf::Transformable *>(&sprite.shape));

        world.emplace<PositionComponent>(player, sf::Vector2f{450.0f, 350.0f});
        world.emplace<FillColorComponent>(player, sf::Color::White);
        world.emplace<BoundingBoxComponent>(player, sf::Vector2f{50.0f, 50.0f});
        world.emplace<InvalidatedBoundingBoxTag>(player);
        world.emplace<MoveControlComponent>(player, MoveControlComponent{});
        world.emplace<MoveSpeedComponent>(player, sf::Vector2f{50.0f, 50.0f});
        world.emplace<VelocityComponent>(player, sf::Vector2f{0.0f, 0.0f});
    }

    {
        const auto checkObject = world.create();
        RectangleShapeComponent &sprite = world.emplace<RectangleShapeComponent>(checkObject);
        world.emplace<SfmlDrawableComponent>(checkObject, static_cast<sf::Drawable *>(&sprite.shape));
        world.emplace<SfmlTransformableComponent>(checkObject, static_cast<sf::Transformable *>(&sprite.shape));

        world.emplace<BoundingBoxComponent>(checkObject, sf::Vector2f{20.0f, 50.0f});
        world.emplace<InvalidatedBoundingBoxTag>(checkObject);
        world.emplace<FillColorComponent>(checkObject, sf::Color::Blue);
        world.emplace<PositionComponent>(checkObject, sf::Vector2f{500.f, 200.f});
    }

    sf::RenderWindow window{sf::VideoMode(static_cast<sf::Vector2u>(WindowSize)), "PewPew"};
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Clock frameClock;
    while (window.isOpen()) {
        const sf::Time frameTime = frameClock.restart();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            const auto entity = world.create();
            world.emplace<InputEventComponent>(entity, *std::move(event));
        }

        gameServices.Update(frameTime);

        {
            sf::Vector2i mousePosScreenSpace = sf::Mouse::getPosition(window);
            sf::Vector2f mousePosWorldSpace = window.mapPixelToCoords(mousePosScreenSpace, view);
            std::println("({},{})", mousePosWorldSpace.x, mousePosWorldSpace.y);
            float x1 = mousePosWorldSpace.x;
            float y1 = mousePosWorldSpace.y;
            float x2 = 1000.f;
            float y2 = 500.f;
            float sqrtMouse =  sqrt((x1 * x1) + (y1 * y1));
            float sqrtPlayer = sqrt((x2 * x2) + (y2 * y2));
            float cos = (((x1 * x2) + (y1 * y2))/(sqrtMouse * sqrtPlayer));
            float rotationAnglef = acos(cos);
            const sf::Angle rotationAngle = sf::radians(rotationAnglef);
            spritePlayer->shape.setRotation(rotationAngle);
        }

        window.clear();
        gameServices.Render(window);
        window.display();
    }
    return 0;
}
