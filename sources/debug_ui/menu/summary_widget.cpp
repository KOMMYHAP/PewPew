#include "summary_widget.h"


GameStatisticsWidget::GameStatisticsWidget(const GameStatistics &statistics)
    : _statistics(&statistics)
{
}

BaseMenuWidget::MenuWidgetAction GameStatisticsWidget::ProcessMenuItem(sf::Time /*time*/)
{
    const sf::Time frameTime = _statistics->GetFrameTime();
    ImGui::Text("FPS: %.2f (%d ms)", 1.0f / frameTime.asSeconds(), frameTime.asMilliseconds());
    ImGui::Text("Frame: %d", _statistics->GetElapsedFramesCount());
    ImGui::Text("Game Time: %.2f seconds", _statistics->GetElapsedGameTime().asSeconds());
    ImGui::Text("Physics Time: %d ms (%d steps)", _statistics->GetPhysicsTime().asMilliseconds(), _statistics->GetPhysicsSteps());
    ImGui::Text("Physics Bodies: %d", _statistics->GetPhysicsBodies());
    return MenuWidgetAction::KeepOpen;
}
