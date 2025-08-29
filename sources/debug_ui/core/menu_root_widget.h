#pragma once
#include "base_menu_widget.h"
#include "menu_widget_id.h"

class MenuWidget {
public:
    template <class T, class... Args>
        requires std::is_base_of_v<BaseMenuWidget, T> && std::is_constructible_v<T, Args...>
    std::pair<MenuWidgetId, T*> AddWidget(std::string name, Args&&... args);

    template <class T, class... Args>
        requires std::is_base_of_v<BaseMenuWidget, T> && std::is_constructible_v<T, Args...>
    std::pair<MenuWidgetId, T*> AddWidget(MenuWidgetId parent, std::string name, Args&&... args);

    bool HasWidget(MenuWidgetId id) const;

    void OpenWidget(MenuWidgetId id);
    void CloseWidget(MenuWidgetId id);

    void UpdateWidget(sf::Time elapsedTime);

private:
    static constexpr size_t LimitNumberOfWidgets = 1'000;
    struct WidgetState {
        bool opened { false };
        bool justOpenedFirstTime { false };
        bool justOpened { false };
        bool justClosed { false };
        bool wasOpenedAtLeastOnce { false };
    };
    struct WidgetData {
        WidgetState state;
        MenuWidgetId parent { MenuWidgetId::Invalid };
        std::unique_ptr<BaseMenuWidget> widget;
        std::string name;
    };
    struct WidgetsGroup {
        std::vector<MenuWidgetId> items;
    };

    // Builder with reusable storage
    struct WidgetNameBuilder {
        std::vector<const WidgetData*> widgets;
        std::string name;
    };

    MenuWidgetId AddWidget(MenuWidgetId parent, WidgetData widgetData);

    WidgetData& ModifyWidgetData(MenuWidgetId id);
    const WidgetData& GetWidgetData(MenuWidgetId id) const;
    const WidgetsGroup* FindWidgetGroup(MenuWidgetId id) const;

    void UpdateWidgetsGroup(MenuWidgetId id);
    void ProcessWidgetState(MenuWidgetId id);
    void ChangeWidgetState(MenuWidgetId id, bool openedNow, bool wasOpen);

    void UpdateOpenedWidgets(sf::Time elapsedTime);
    bool ProcessOpenedWidgetState(MenuWidgetId id, sf::Time elapsedTime);

    // Must return null terminated string
    std::string_view MakeFullWidgetName(const WidgetData& data);

    std::vector<WidgetData> _widgets;
    std::map<MenuWidgetId, WidgetsGroup> _indexedGroups;
    std::vector<MenuWidgetId> _openedWidgets;
    WidgetNameBuilder _fullNameBuilder;
};

#include "menu_root_widget.hpp"