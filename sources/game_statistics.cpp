#include "game_statistics.h"

static constexpr int32_t FRAME_HISTORY_LIMIT = 100;

GameStatistics::GameStatistics()
    : _fpsCounter{FRAME_HISTORY_LIMIT}
    , _physicsTimeCounter{FRAME_HISTORY_LIMIT}
    , _physicsStepsCounter{FRAME_HISTORY_LIMIT}
{
}

void GameStatistics::Update(sf::Time elapsedTime)
{
    _fpsCounter.AddSample(elapsedTime.asSeconds());
    _elapsedGameTime += elapsedTime;
    _elapsedFramesCount++;
}

void GameStatistics::LogPhysics(sf::Time elapsedTime, int32_t steps, int32_t bodies)
{
    _physicsTimeCounter.AddSample(elapsedTime.asSeconds());
    _physicsStepsCounter.AddSample(steps);
    _bodiesCounter = bodies;
}

sf::Time GameStatistics::GetFrameTime() const
{
    const float fps = _fpsCounter.CalcMedian();
    return sf::seconds(fps);
}

sf::Time GameStatistics::GetPhysicsTime() const
{
    const float physicsTime = _physicsTimeCounter.CalcMedian();
    return sf::seconds(physicsTime);
}

int32_t GameStatistics::GetPhysicsSteps() const
{
    return _physicsStepsCounter.CalcAverage();
}

int32_t GameStatistics::GetPhysicsBodies() const
{
    return _bodiesCounter;
}
