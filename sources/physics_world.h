#pragma once

class PhysicsWorld
{
public:
    PhysicsWorld(EntityWorld &ecsWorld);

    void Update(sf::Time elapsedTime);

    sf::Vector2f GetBoundingBox(b2BodyId id) const;
    sf::Vector2f GetPosition(b2BodyId id) const;

private:

    void UpdatePhysics(sf::Time elapsedTime);
    void UpdateEcsPhysics();

    sf::Time _physicAccumulatedErrorTime;
    b2WorldId _physicsWorldId{b2_nullWorldId};
    Ref<EntityWorld> _ecsWorld;
};
