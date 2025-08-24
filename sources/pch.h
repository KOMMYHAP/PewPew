#pragma once
#include <random>
#include <print>

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <gsl/pointers>
#include "box2d/box2d.h"

template<class T>
requires !std::is_const_v<T>
using Ref = gsl::strict_not_null<T*>;

template<class T>
using ConstRef = gsl::strict_not_null<std::add_const_t<T>*>;

using EntityWorld = entt::registry;
using Entity = entt::entity;