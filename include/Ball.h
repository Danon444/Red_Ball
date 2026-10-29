#pragma once

#include <SFML/Graphics.hpp>

class Ball
{
private:
    sf::CircleShape shape;

    float speed = 200.f;
    float velocityY = 0.f;
    float gravity = 900.f;
    float jumpSpeed = 450.f;
    bool onGround = false;
public:
    Ball();
    void handleEvent(const sf::Event& event);
    void update(float deltaTime, float floorY);
    void draw(sf::RenderWindow& window) const;
};