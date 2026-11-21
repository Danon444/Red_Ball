#pragma once

#include <SFML/Graphics.hpp>

class Enemy
{
private:
    sf::RectangleShape body{{55.f, 55.f}};
    sf::Vector2f startPosition;
    float endX;
    float speed;
    float direction = 1.f;
    bool active = true;

public:
    Enemy(sf::Vector2f startPosition, float endX, float speed);
    void setTexture(const sf::Texture& texture);

    void update(float deltaTime);
    sf::FloatRect getBounds() const;
    bool isActive() const;
    void squash();                    // kill
    void reset();
    void draw(sf::RenderWindow& window) const;
};
