#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <gsl/pointers>

template<class T>
requires !std::is_const_v<T>
using Ref = gsl::strict_not_null<T*>;

template<class T>
using ConstRef = gsl::strict_not_null<std::add_const_t<T>*>;