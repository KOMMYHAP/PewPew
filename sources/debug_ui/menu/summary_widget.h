#pragma once

#include "debug_ui/core/base_menu_widget.h"
#include "game_statistics.h"

class GameStatisticsWidget final : public BaseMenuWidget {
public:
    GameStatisticsWidget(const GameStatistics& statistics);

    MenuWidgetAction ProcessMenuItem(sf::Time) override;

private:
    ConstRef<GameStatistics> _statistics;
};