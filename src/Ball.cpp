#include "../include/Ball.h"

#include <algorithm>

Ball::Ball()
{
    shape.setRadius(30.f);
    shape.setFillColor(sf::Color::Red);
    reset();
}

void Ball::reset()
{
    shape.setPosition(spawnPosition);
    velocityX = 0.f;
    velocityY = 0.f;
    onGround = false;
}

sf::Vector2f Ball::getPosition() const
{
    return shape.getPosition();
}

sf::FloatRect Ball::getBounds() const
{
    return shape.getGlobalBounds();
}

bool Ball::isOnGround() const
{
    return onGround;
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

void Ball::update(float deltaTime, const std::vector<sf::FloatRect>& surfaces)
{
    float direction = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        direction -= 1.f;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        direction += 1.f;
    }

    if (direction != 0.f)
    {
        float acceleration = onGround ? groundAcceleration : airAcceleration;
        velocityX += direction * acceleration * deltaTime;
        velocityX = std::clamp(velocityX, -maxSpeed, maxSpeed);
    }
    else if (onGround)
    {
        // Approach zero without changing direction while braking.
        float speedLoss = braking * deltaTime;
        if (velocityX > 0.f)
        {
            velocityX = std::max(0.f, velocityX - speedLoss);
        }
        else
        {
            velocityX = std::min(0.f, velocityX + speedLoss);
        }
    }

    float moveX = velocityX * deltaTime;

    float diameter = shape.getRadius() * 2.f;
    sf::Vector2f previousPosition = shape.getPosition();
    float nextX = previousPosition.x + moveX;

    // Resolve horizontal movement before applying vertical movement.
    for (const sf::FloatRect& surface : surfaces)
    {
        float surfaceRight = surface.position.x + surface.size.x;
        float surfaceBottom = surface.position.y + surface.size.y;
        bool overlapsVertically = previousPosition.y < surfaceBottom &&
                                  previousPosition.y + diameter > surface.position.y;

        if (!overlapsVertically)
        {
            continue;
        }

        if (moveX > 0.f && previousPosition.x + diameter <= surface.position.x &&
            nextX + diameter >= surface.position.x)
        {
            nextX = surface.position.x - diameter;
            velocityX = 0.f;
        }
        else if (moveX < 0.f && previousPosition.x >= surfaceRight && nextX <= surfaceRight)
        {
            nextX = surfaceRight;
            velocityX = 0.f;
        }
    }

    shape.setPosition({nextX, previousPosition.y});

    float previousTop = previousPosition.y;
    float previousBottom = shape.getPosition().y + diameter;

    velocityY += gravity * deltaTime;
    shape.move({0.f, velocityY * deltaTime});

    float ballLeft = shape.getPosition().x;
    float ballRight = ballLeft + diameter;
    float ballBottom = shape.getPosition().y + diameter;
    float ballTop = shape.getPosition().y;

    onGround = false;

    // Choose the highest surface crossed during this fall.
    float landingTop = ballBottom;
    bool hitCeiling = false;
    float ceilingBottom = ballTop;

    for (const sf::FloatRect& surface : surfaces)
    {
        float surfaceLeft = surface.position.x;
        float surfaceRight = surfaceLeft + surface.size.x;
        float surfaceTop = surface.position.y;
        float surfaceBottom = surfaceTop + surface.size.y;

        bool overlapsHorizontally = ballRight > surfaceLeft && ballLeft < surfaceRight;
        bool crossesTop = previousBottom <= surfaceTop && ballBottom >= surfaceTop;

        if (velocityY >= 0.f && overlapsHorizontally && crossesTop)
        {
            if (!onGround || surfaceTop < landingTop)
            {
                landingTop = surfaceTop;
            }
            onGround = true;
        }

        bool crossesBottom = previousTop >= surfaceBottom && ballTop <= surfaceBottom;

        if (velocityY < 0.f && overlapsHorizontally && crossesBottom)
        {
            if (!hitCeiling || surfaceBottom > ceilingBottom)
            {
                ceilingBottom = surfaceBottom;
            }
            hitCeiling = true;
        }
    }

    if (onGround)
    {
        shape.setPosition({ballLeft, landingTop - diameter});
        velocityY = 0.f;
    }
    else if (hitCeiling)
    {
        shape.setPosition({ballLeft, ceilingBottom});
        velocityY = -velocityY * ceilingRestitution;
    }

}    

void Ball::draw(sf::RenderWindow& window) const
{
    window.draw(shape);
}

