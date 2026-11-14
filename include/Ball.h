#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Ball
{
private:
    sf::CircleShape shape;
    sf::RectangleShape stripe;
    sf::Vector2f spawnPosition{100.f, 100.f};
    sf::Vector2f position;
    float radius = 30.f;
    float rotationDegrees = 0.f;

    float velocityX = 0.f;
    float maxSpeed = 220.f;
    float groundAcceleration = 900.f;
    float airAcceleration = 450.f;
    float braking = 1000.f;
    float velocityY = 0.f;
    float gravity = 900.f;
    float jumpSpeed = 450.f;
    float ceilingRestitution = 0.7f;
    bool onGround = false;

    void syncVisuals();
public:
    Ball();
    void reset();
    void reset(sf::Vector2f newPosition);
    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;
    bool isOnGround() const;
    void handleEvent(const sf::Event& event);
    void update(float deltaTime, const std::vector<sf::FloatRect>& surfaces);
    void update(float deltaTime, const std::vector<sf::FloatRect>& surfaces, float direction);
    void draw(sf::RenderWindow& window) const;
};
