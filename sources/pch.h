#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <gsl/pointers>

template<class T>
using Ref = gsl::strict_not_null<T>;