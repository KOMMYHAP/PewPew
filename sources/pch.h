#pragma once
#include "std_headers.h"

#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <imgui-SFML.h>
#include <imgui.h>


using EntityWorld = entt::registry;
using Entity = entt::entity;
using Json = nlohmann::json;