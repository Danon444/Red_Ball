#pragma once

#include <SFML/Graphics.hpp>

class MovingPlatform
{
private:
    sf::RectangleShape body;
    sf::RectangleShape stripe;
    sf::Vector2f startPosition;
    float endX;
    float period;
    float elapsed = 0.f;

public:
    MovingPlatform(sf::FloatRect bounds, float endX, float period);

    sf::FloatRect getBounds() const;
    float update(float deltaTime);
    void reset();
    void draw(sf::RenderWindow& window) const;
};
