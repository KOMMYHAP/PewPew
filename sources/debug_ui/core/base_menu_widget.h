#pragma once

class BaseMenuWidget {
public:
    virtual ~BaseMenuWidget() = default;

    virtual void OnMenuItemOpenedFirstTime() { }
    virtual void OnMenuItemJustOpened() { }

    enum class MenuWidgetAction {
        KeepOpen,
        ShouldClose,
    };
    virtual MenuWidgetAction ProcessMenuItem(sf::Time /*elapsedTime*/) { return MenuWidgetAction::ShouldClose; }

    virtual void OnMenuItemJustClosed() { }
};