#pragma once


using EntityWorld = entt::registry;
using Entity = entt::entity;

struct RectangleShapeComponent {
    sf::RectangleShape shape;
};


struct PositionComponent {
    float x;
    float y;
};

struct VelocityComponent {
    float dx;
    float dy;
};

struct InitialSpeedComponent {
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

struct MoveControlComponent {
    enum { Left = 0, Right = 1, Up = 2, Down = 3 };

    std::array<bool, 4> activeDirections{};
};

struct MoveSpeedComponent {
    sf::Vector2f value;
};

struct InputEventComponent {
    sf::Event event;
};
