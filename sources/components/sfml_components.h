#pragma once


struct RectangleShapeComponent {
    sf::RectangleShape shape;
};


struct CircleShapeComponent {
    sf::CircleShape shape;
};

struct FillColorComponent {
    sf::Color color;
};

struct SfmlDrawableComponent {
    sf::Drawable *drawable;
};

struct SfmlTransformableComponent {
    sf::Transformable *transform;
};

struct SfmlViewComponent {
    sf::View view;
};

struct InputEventComponent {
    sf::Event event;
};
