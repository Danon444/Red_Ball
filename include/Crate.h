#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Crate
{
private:
    sf::RectangleShape body;
    sf::RectangleShape band;
    sf::Vector2f spawn;
    float velocityY = 0.f;

public:
    explicit Crate(sf::FloatRect bounds);
    sf::FloatRect getBounds() const;
    void push(float shiftX, const std::vector<sf::FloatRect>& surfaces);
    void update(float deltaTime, const std::vector<sf::FloatRect>& surfaces);
    void reset();
    void draw(sf::RenderWindow& window) const;
};
