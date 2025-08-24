#include "physics_world.h"

#include "components/physics_components.h"

PhysicsWorld::PhysicsWorld(EntityWorld &ecsWorld)
    : _ecsWorld(&ecsWorld)
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0f, 0.0f);
    _physicsWorldId = b2CreateWorld(&worldDef);
}

void PhysicsWorld::Update(sf::Time elapsedTime)
{
    UpdatePhysics(elapsedTime);
    UpdateEcsPhysics();
}

sf::Vector2f PhysicsWorld::GetBoundingBox(b2BodyId id) const
{
    const int shapedCount = b2Body_GetShapeCount(id);
    assert(shapedCount == 1);
    b2ShapeId shapeId{b2_nullShapeId};
    b2Body_GetShapes(id, &shapeId, 1);
    const b2AABB shapeAABB = b2Shape_GetAABB(shapeId);
    const sf::Vector2f lowerBound{shapeAABB.lowerBound.x, shapeAABB.lowerBound.y};
    const sf::Vector2f upperBound{shapeAABB.upperBound.x, shapeAABB.upperBound.y};
    return upperBound - lowerBound;
}

sf::Vector2f PhysicsWorld::GetPosition(b2BodyId id) const
{
    const b2Vec2 position = b2Body_GetPosition(id);
    return {position.x, position.y};
}

void PhysicsWorld::UpdatePhysics(sf::Time elapsedTime)
{
    static constexpr sf::Time PhysicsStepTime = sf::milliseconds(10);
    static constexpr int PhysicsSubStepsCount = 4;
    sf::Time physicsElapsedTime = elapsedTime - PhysicsStepTime + _physicAccumulatedErrorTime;
    int physicsStepsCount = 0;
    while (physicsElapsedTime > PhysicsStepTime)
    {
        b2World_Step(_physicsWorldId, PhysicsStepTime.asSeconds(), PhysicsSubStepsCount);
        physicsElapsedTime -= PhysicsStepTime;
        physicsStepsCount += 1;
    }
    _physicAccumulatedErrorTime = physicsElapsedTime;
    std::println("Physics steps = {}", physicsStepsCount);
}

void PhysicsWorld::UpdateEcsPhysics()
{
    auto AddKinematicPhysicsObjectSystem = [this](Entity entity, const KinematicPhysicsObjectPrototype &kinematicPrototype) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.position = b2Vec2{kinematicPrototype.position.x, kinematicPrototype.position.y};
        bodyDef.type = b2_kinematicBody;

        b2BodyId bodyId = b2CreateBody(_physicsWorldId, &bodyDef);

        const b2Polygon shapeBoundingBox = b2MakeBox(kinematicPrototype.boundingBox.x / 2.0f, kinematicPrototype.boundingBox.y / 2.0f);
        const b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(bodyId, &shapeDef, &shapeBoundingBox);

        _ecsWorld->emplace<PhysicsBody>(entity, bodyId);
    };

    _ecsWorld->view<const KinematicPhysicsObjectPrototype>().each(AddKinematicPhysicsObjectSystem);
    _ecsWorld->clear<KinematicPhysicsObjectPrototype>();
}