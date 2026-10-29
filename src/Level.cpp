#include "Level.h"

Level::Level()
{
    floor.setSize({800.f, 100.f});
    floor.setPosition({0.f, floorY});
    floor.setFillColor(sf::Color::Green);
}

float Level::getFloorY() const
{
    return floorY;
}

void Level::draw(sf::RenderWindow& window) const
{
    window.draw(floor);
}