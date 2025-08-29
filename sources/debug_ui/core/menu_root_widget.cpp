#include "menu_root_widget.h"

bool MenuWidget::HasWidget(MenuWidgetId id) const
{
    if (id == MenuWidgetId::Root) {
        return true;
    }
    return std::to_underlying(id) < _widgets.size();
}

void MenuWidget::OpenWidget(MenuWidgetId id)
{
    if (!HasWidget(id)) {
        assert(false && "Invalid widget id!");
        return;
    }
    ChangeWidgetState(id, true, GetWidgetData(id).state.opened);
}

void MenuWidget::CloseWidget(MenuWidgetId id)
{
    if (!HasWidget(id)) {
        assert(false && "Invalid widget id!");
        return;
    }
    ChangeWidgetState(id, false, GetWidgetData(id).state.opened);
}

void MenuWidget::UpdateWidget(sf::Time elapsedTime)
{
    if (ImGui::BeginMainMenuBar()) {
        if (const WidgetsGroup* rootGroup = FindWidgetGroup(MenuWidgetId::Root)) {
            for (const MenuWidgetId id : rootGroup->items) {
                UpdateWidgetsGroup(id);
            }
        }
        ImGui::EndMainMenuBar();
    }

    UpdateOpenedWidgets(elapsedTime);
}

MenuWidgetId MenuWidget::AddWidget(MenuWidgetId parent, WidgetData widgetData)
{
    if (_widgets.size() >= LimitNumberOfWidgets) {
        assert(false && "Limit of widgets reached!");
        return MenuWidgetId::Invalid;
    }
    if (!HasWidget(parent)) {
        assert(false && "Invalid parent id!");
        return MenuWidgetId::Invalid;
    }
    const auto childId = static_cast<MenuWidgetId>(_widgets.size());
    _widgets.emplace_back(std::move(widgetData));
    _indexedGroups[parent].items.emplace_back(childId);
    return childId;
}

MenuWidget::WidgetData& MenuWidget::ModifyWidgetData(MenuWidgetId id)
{
    const size_t index = static_cast<size_t>(id);
    assert(index < _widgets.size() && "Invalid index!");
    return _widgets[index];
}

const MenuWidget::WidgetData& MenuWidget::GetWidgetData(MenuWidgetId id) const
{
    const size_t index = static_cast<size_t>(id);
    assert(index < _widgets.size() && "Invalid index!");
    return _widgets[index];
}

const MenuWidget::WidgetsGroup* MenuWidget::FindWidgetGroup(MenuWidgetId id) const
{
    const auto groupIt = _indexedGroups.find(id);
    if (groupIt == _indexedGroups.end()) {
        return nullptr;
    }
    const WidgetsGroup& group = groupIt->second;
    if (group.items.empty()) {
        return nullptr;
    }
    return &group;
}

void MenuWidget::UpdateWidgetsGroup(MenuWidgetId id)
{
    const WidgetsGroup* group = FindWidgetGroup(id);
    if (!group) {
        ProcessWidgetState(id);
        return;
    }

    for (const MenuWidgetId childId : group->items) {
        const WidgetData& widgetData = GetWidgetData(id);
        if (!ImGui::BeginMenu(widgetData.name.c_str())) {
            continue;
        }
        UpdateWidgetsGroup(childId);
        ImGui::EndMenu();
    }
}

void MenuWidget::ProcessWidgetState(MenuWidgetId id)
{
    const WidgetData& widgetData = GetWidgetData(id);
    const WidgetState& widgetState = widgetData.state;

    const bool wasOpen = widgetState.opened;
    bool opened = widgetState.opened;
    const bool openedByImgui = ImGui::MenuItem(widgetData.name.c_str(), nullptr, &opened);
    if (!openedByImgui) {
        return;
    }

    ChangeWidgetState(id, opened, wasOpen);
}

void MenuWidget::ChangeWidgetState(MenuWidgetId id, bool openedNow, bool wasOpen)
{
    WidgetData& widgetData = ModifyWidgetData(id);
    WidgetState& widgetState = widgetData.state;

    if (openedNow) {
        widgetState.opened = true;
        if (!widgetState.wasOpenedAtLeastOnce) {
            widgetState.wasOpenedAtLeastOnce = true;
            widgetState.justOpenedFirstTime = true;
        }
        widgetState.justOpened = !wasOpen;
        if (widgetState.justOpened) {
            _openedWidgets.emplace_back(id);
        }
    } else {
        widgetState.opened = false;
        widgetState.justClosed = wasOpen && !widgetState.opened;
    }
}

void MenuWidget::UpdateOpenedWidgets(sf::Time elapsedTime)
{
    auto it = std::ranges::remove_if(_openedWidgets, [this, elapsedTime](const MenuWidgetId id) {
        const bool shouldClose = ProcessOpenedWidgetState(id, elapsedTime);
        return shouldClose;
    });
    _openedWidgets.erase(it.begin(), it.end());
}

bool MenuWidget::ProcessOpenedWidgetState(const MenuWidgetId id, const sf::Time elapsedTime)
{
    WidgetData& widgetData = ModifyWidgetData(id);
    BaseMenuWidget& widget = *widgetData.widget;
    WidgetState& widgetState = widgetData.state;

    if (widgetState.justOpenedFirstTime) {
        widgetState.justOpenedFirstTime = false;
        widget.OnMenuItemOpenedFirstTime();
    }
    if (widgetState.justOpened) {
        widgetState.justOpened = false;
        widget.OnMenuItemJustOpened();
    }
    if (widgetState.opened) {
        bool isOpened { true };
        const std::string_view widgetName = MakeFullWidgetName(widgetData);
        ImGui::Begin(widgetName.data(), &isOpened);
        BaseMenuWidget::MenuWidgetAction action = BaseMenuWidget::MenuWidgetAction::ShouldClose;
        if (isOpened) {
            action = widget.ProcessMenuItem(elapsedTime);
        }
        ImGui::End();
        widgetState.justClosed = action == BaseMenuWidget::MenuWidgetAction::ShouldClose;
    }
    if (widgetState.justClosed) {
        widgetState.justClosed = false;
        widgetState.opened = false;
        widget.OnMenuItemJustClosed();
        return true;
    }
    return false;
}

std::string_view MenuWidget::MakeFullWidgetName(const WidgetData& data)
{
    _fullNameBuilder.widgets.clear();
    _fullNameBuilder.name.clear();

    MenuWidgetId id = data.parent;
    _fullNameBuilder.widgets.push_back(&data);
    while (id != MenuWidgetId::Root) {
        const WidgetData& parent = GetWidgetData(id);
        _fullNameBuilder.widgets.push_back(&parent);
        id = parent.parent;
    }

    bool needSeparator = false;
    for (const WidgetData* widget : _fullNameBuilder.widgets | std::views::reverse) {
        if (needSeparator) {
            _fullNameBuilder.name += '.';
        }
        needSeparator = true;
        _fullNameBuilder.name += widget->name;
    }
    return _fullNameBuilder.name;
}
