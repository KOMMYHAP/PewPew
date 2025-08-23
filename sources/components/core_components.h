#pragma once

struct PositionComponent {
    sf::Vector2f position;
};

struct RotationComponent {
    float radians;
};

struct ScaleComponent {
    sf::Vector2f factor;
};

struct VelocityComponent {
    sf::Vector2f delta;
};

struct InitialSpeedComponent {
    sf::Vector2f delta;
};

struct BoundingBoxComponent {
    sf::Vector2f size;
};

struct InvalidatedBoundingBoxTag {

};

struct MoveControlComponent {
    enum { Left = 0, Right = 1, Up = 2, Down = 3 };

    std::array<bool, 4> activeDirections{};
};

struct MoveSpeedComponent {
    sf::Vector2f value;
};

struct IsVisibleInCamera {

};
