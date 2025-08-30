#pragma once
#include "game_statistics.h"

class PhysicsWorld
{
public:
    PhysicsWorld(EntityWorld &ecsWorld, GameStatistics& statistics);

    void Update(sf::Time elapsedTime);

    sf::Vector2f GetBoundingBox(b2BodyId id) const;
    sf::Vector2f GetPosition(b2BodyId id) const;
    int32_t GetBodysCount() const;

private:
    void UpdatePhysics(sf::Time elapsedTime);
    void UpdateEcsPhysics();

    sf::Time _physicAccumulatedErrorTime;
    b2WorldId _physicsWorldId{b2_nullWorldId};
    Ref<EntityWorld> _ecsWorld;
    Ref<GameStatistics> _statistics;
};
