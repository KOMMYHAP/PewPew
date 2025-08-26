#pragma once
#include "menu_root_widget.h"

template <class T, class... Args>
    requires std::is_base_of_v<BaseMenuWidget, T> && std::is_constructible_v<T, Args...>
std::pair<MenuWidgetId, T*> MenuWidget::AddWidget(std::string name, Args&&... args)
{
    return AddWidget<T>(MenuWidgetId::Root, std::move(name), std::forward<Args>(args)...);
}

template <class T, class... Args>
    requires std::is_base_of_v<BaseMenuWidget, T> && std::is_constructible_v<T, Args...>
std::pair<MenuWidgetId, T*> MenuWidget::AddWidget(MenuWidgetId parent, std::string name, Args&&... args)
{
    WidgetData data;
    data.parent = parent;
    data.widget = std::make_unique<T>(std::forward<Args>(args)...);
    data.name = std::move(name);
    T* rawWidget = static_cast<T*>(data.widget.get());
    const MenuWidgetId id = AddWidget(parent, std::move(data));
    if (id == MenuWidgetId::Invalid) {
        rawWidget = nullptr;
    }
    return std::make_pair(id, rawWidget);
}
