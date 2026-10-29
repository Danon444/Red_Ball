#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Level
{
private:
    std::vector<sf::RectangleShape> surfaces;
    sf::RectangleShape finishPole;
    sf::RectangleShape finishFlag;

    void addSurface(sf::Vector2f position, sf::Vector2f size, sf::Color color);
public:
    Level();

    std::vector<sf::FloatRect> getSurfaceBounds() const;
    sf::FloatRect getFinishBounds() const;
    void draw(sf::RenderWindow& window) const;
};
