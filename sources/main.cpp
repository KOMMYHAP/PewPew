#include "components/core_components.h"
#include "components/physics_components.h"
#include "components/sfml_components.h"
#include "game_services.h"

int main()
{
    static constexpr sf::Vector2f WindowSize{800.0f, 600.0f};

    sf::RenderWindow window{sf::VideoMode(static_cast<sf::Vector2u>(WindowSize)), "PewPew"};
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    GameServices gameServices{window};
    EntityWorld &world = gameServices.ModifyWorld();

    WatchTargetComponent *mouse = nullptr;
    {
        const auto player = world.create();
        RectangleShapeComponent &sprite = world.emplace<RectangleShapeComponent>(player);
        world.emplace<SfmlDrawableComponent>(player, static_cast<sf::Drawable *>(&sprite.shape));
        world.emplace<SfmlTransformableComponent>(player, static_cast<sf::Transformable *>(&sprite.shape));
        world.emplace<FillColorComponent>(player, sf::Color::White);
        world.emplace<InvalidatedBoundingBoxTag>(player);
        world.emplace<MoveControlComponent>(player, MoveControlComponent{});
        world.emplace<MoveSpeedComponent>(player, 5.0f);
        world.emplace<VelocityComponent>(player, sf::Vector2f{0.0f, 0.0f});
        WatchTargetComponent &watchTargetComponent = world.emplace<WatchTargetComponent>(player, sf::Vector2f{0.0f, 0.0f});
        mouse = &watchTargetComponent;
        world.emplace<RotationComponent>(player, sf::degrees(0));
        world.emplace<KinematicPhysicsObjectPrototype>(player, sf::Vector2f{0.0f, 0.0f}, sf::Vector2f{1.0f, 1.0f});
    }
    {
        const auto checkObject = world.create();
        RectangleShapeComponent &sprite = world.emplace<RectangleShapeComponent>(checkObject);
        world.emplace<SfmlDrawableComponent>(checkObject, static_cast<sf::Drawable *>(&sprite.shape));
        world.emplace<SfmlTransformableComponent>(checkObject, static_cast<sf::Transformable *>(&sprite.shape));

        world.emplace<InvalidatedBoundingBoxTag>(checkObject);
        world.emplace<FillColorComponent>(checkObject, sf::Color::Blue);
        world.emplace<KinematicPhysicsObjectPrototype>(checkObject, sf::Vector2f{15.0f, 20.0f}, sf::Vector2f{1.0f, 5.0f});
    }

    sf::Clock frameClock;
    while (window.isOpen())
    {
        const sf::Time frameTime = frameClock.restart();

        while (std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            gameServices.ModifyDebugUi().ProcessInput(*event);

            const auto entity = world.create();
            world.emplace<InputEventComponent>(entity, *std::move(event));
        }

        gameServices.Update(frameTime);
        {
            sf::Vector2i mousePosScreenSpace = sf::Mouse::getPosition(window);
            sf::Vector2f mousePosWorldSpace = window.mapPixelToCoords(mousePosScreenSpace, gameServices.GetCamera().GetView());
            mouse->position = mousePosWorldSpace;
            std::println("({},{})", mousePosWorldSpace.x, mousePosWorldSpace.y);
        }

        window.clear();
        window.setView(gameServices.GetCamera().GetView());
        gameServices.Draw();
        window.display();

        std::fflush(stdout);
    }
    return 0;
}
