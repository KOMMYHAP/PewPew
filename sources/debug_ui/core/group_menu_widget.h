#pragma once
#include "base_menu_widget.h"

class GroupMenuWidget final : public BaseMenuWidget {
public:
    MenuWidgetAction ProcessMenuItem(sf::Time /*elapsedTime*/) override { return MenuWidgetAction::KeepOpen; }
};