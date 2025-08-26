#pragma once

#include "utils/sample_counter.h"

class GameStatistics
{
public:
    GameStatistics();

    void Update(sf::Time elapsedTime);
    void LogPhysics(sf::Time elapsedTime, int32_t steps);

    sf::Time GetFrameTime() const;
    sf::Time GetElapsedGameTime() const { return _elapsedGameTime; }
    int32_t GetElapsedFramesCount() const { return _elapsedFramesCount; }

    sf::Time GetPhysicsTime() const;
    int32_t GetPhysicsSteps() const;

private:
    FloatSampleCounter _fpsCounter;
    FloatSampleCounter _physicsTimeCounter;
    Int32SampleCounter _physicsStepsCounter;
    sf::Time _elapsedGameTime;
    int32_t _elapsedFramesCount{0};
};
