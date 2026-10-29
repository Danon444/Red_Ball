#include "../include/Ball.h"

Ball::Ball()
{
    shape.setRadius(30.f);
    shape.setFillColor(sf::Color::Red);
    shape.setPosition({100.f, 100.f});
}

void Ball::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        bool jumpPressed =
            key->code == sf::Keyboard::Key::Space ||
            key->code == sf::Keyboard::Key::W ||
            key->code == sf::Keyboard::Key::Up;
        if (jumpPressed && onGround)
        {
            velocityY = -jumpSpeed;
            onGround = false;
        }
    }
}

void Ball::update(float deltaTime, float floorY)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        shape.move({-speed * deltaTime, 0.f});
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        shape.move({speed * deltaTime, 0.f});
    }

    velocityY += gravity * deltaTime;
    shape.move({0.f, velocityY * deltaTime});

    float diameter = shape.getRadius() * 2.f;

    onGround = false;

    if (shape.getPosition().y + diameter >= floorY)
    {
        shape.setPosition({shape.getPosition().x, floorY - diameter});
        velocityY = 0.f;
        onGround = true;
    }

}    

void Ball::draw(sf::RenderWindow& window) const
{
    window.draw(shape);
}

