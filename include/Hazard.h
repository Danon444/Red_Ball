#pragma once

#include <SFML/Graphics.hpp>

class Ball;

class Hazard
{
private:
    sf::FloatRect bounds;
    sf::VertexArray spikes{sf::PrimitiveType::Triangles};

public:
    explicit Hazard(sf::FloatRect bounds);
    bool touches(const Ball& player) const;
    void draw(sf::RenderWindow& window) const;
};
