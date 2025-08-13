#include "game_map.h"


GameMap::GameMap(const sf::FloatRect &boundaries)
    : _boundaries(boundaries) {
}

sf::FloatRect GameMap::GetViewSpace() const {
    return sf::FloatRect{_view.getCenter() - _view.getSize() / 2.0f, _view.getSize()};
}


