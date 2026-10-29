#pragma once

#include <SFML/Graphics.hpp>

class Level
{
private:
    float floorY = 500.f;
    sf::RectangleShape floor;
public:
    Level();

    float getFloorY() const;
    void draw(sf::RenderWindow& window) const;
};