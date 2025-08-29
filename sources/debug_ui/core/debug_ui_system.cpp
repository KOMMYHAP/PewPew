#include "debug_ui_system.h"

DebugUiSystem::DebugUiSystem(sf::RenderWindow & window)
    : _window(&window)
{
    const bool imguiInited = ImGui::SFML::Init(*_window);
    assert(imguiInited && "ImGui::SFML::Init failed!");
}

DebugUiSystem::~DebugUiSystem()
{
    ImGui::SFML::Shutdown();
}

void DebugUiSystem::ProcessInput(const sf::Event &event)
{
    ImGui::SFML::ProcessEvent(*_window, event);
}

void DebugUiSystem::Update(sf::Time elapsedTime)
{
    ImGui::SFML::Update(*_window, elapsedTime);
    _menuWidget.UpdateWidget(elapsedTime);
}

void DebugUiSystem::Draw()
{
    ImGui::SFML::Render(*_window);
}