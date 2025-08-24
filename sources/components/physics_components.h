#pragma once

struct KinematicPhysicsObjectPrototype
{
    sf::Vector2f position;
    sf::Vector2f boundingBox;
};

struct PhysicsBody
{
    b2BodyId id;
};
