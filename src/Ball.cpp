#include "../include/Ball.h"

#include <algorithm>
#include <cmath>

Ball::Ball()
{
    shape.setRadius(radius);
    shape.setOrigin({radius, radius});
    shape.setFillColor(sf::Color::Red);

    stripe.setSize({radius * 1.4f, 6.f});
    stripe.setOrigin(stripe.getSize() / 2.f);
    stripe.setFillColor(sf::Color(255, 220, 150));
    reset();
}

void Ball::reset()
{
    position = spawnPosition;
    rotationDegrees = 0.f;
    velocityX = 0.f;
    velocityY = 0.f;
    onGround = false;
    syncVisuals();
}

sf::Vector2f Ball::getPosition() const
{
    return position;
}

void Ball::reset(sf::Vector2f newPosition)
{
    reset();
    position = newPosition;
    syncVisuals();
}

sf::FloatRect Ball::getBounds() const
{
    return {position, {radius * 2.f, radius * 2.f}};
}

void Ball::syncVisuals()
{
    sf::Vector2f center = position + sf::Vector2f{radius, radius};
    shape.setPosition(center);
    stripe.setPosition(center);
    shape.setRotation(sf::degrees(rotationDegrees));
    stripe.setRotation(sf::degrees(rotationDegrees));
}

bool Ball::isOnGround() const
{
    return onGround;
}

void Ball::followPlatform(const sf::FloatRect& previousBounds, float shiftX)
{
    auto ball = getBounds();
    float left = previousBounds.position.x;
    float right = left + previousBounds.size.x;
    float top = previousBounds.position.y;
    float ballRight = position.x + ball.size.x;
    float ballBottom = position.y + ball.size.y;

    if (onGround && std::abs(ballBottom - top) < 0.01f && position.x < right && ballRight > left)
    {
        position.x += shiftX;
    }
    else if (ballBottom > top && position.y < top + previousBounds.size.y)
    {
        // A moving side can reach the ball even when the player is standing still.
        if (position.x >= right && position.x < right + shiftX)
        {
            position.x = right + shiftX;
        }
        else if (ballRight <= left && ballRight > left + shiftX)
        {
            position.x = left + shiftX - ball.size.x;
        }
    }
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

    update(deltaTime, surfaces, direction);
}

void Ball::update(float deltaTime, const std::vector<sf::FloatRect>& surfaces, float direction)
{
    if (direction != 0.f)
    {
        float acceleration;

        if (onGround)
        {
            acceleration = groundAcceleration;
        }
        else
        {
            acceleration = airAcceleration;
        }
        velocityX += direction * acceleration * deltaTime;
        velocityX = std::clamp(velocityX, -maxSpeed, maxSpeed);
    }
    else if (onGround)
    {
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

    float diameter = radius * 2.f;
    sf::Vector2f previousPosition = position;
    float nextX = previousPosition.x + moveX;

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

    position.x = nextX;

    float previousTop = previousPosition.y;
    float previousBottom = previousPosition.y + diameter;

    velocityY += gravity * deltaTime;
    position.y += velocityY * deltaTime;

    float ballLeft = position.x;
    float ballRight = ballLeft + diameter;
    float ballBottom = position.y + diameter;
    float ballTop = position.y;

    onGround = false;

    float landingTop = ballBottom;
    bool hitCeiling = false;
    float ceilingBottom = ballTop;

    for (const sf::FloatRect& surface : surfaces)
    {
        float surfaceLeft = surface.position.x;
        float surfaceRight = surfaceLeft + surface.size.x;
        float surfaceTop = surface.position.y;
        float surfaceBottom = surfaceTop + surface.size.y;

        bool overlapsHorizontally;
        if (ballRight > surfaceLeft && ballLeft < surfaceRight)
        {
            overlapsHorizontally = true;
        }
        else
        {
            overlapsHorizontally = false;
        }

        bool crossesTop;
        if (previousBottom <= surfaceTop && ballBottom >= surfaceTop)
        {
            crossesTop = true;
        }
        else
        {
            crossesTop = false;
        }

        if (velocityY >= 0.f && overlapsHorizontally && crossesTop)
        {
            if (!onGround || surfaceTop < landingTop)
            {
                landingTop = surfaceTop;
            }
            onGround = true;
        }

        bool crossesBottom;
        if (previousTop >= surfaceBottom && ballTop <= surfaceBottom)
        {
            crossesBottom = true;
        }
        else
        {
            crossesBottom = false;
        }

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
        position.y = landingTop - diameter;
        velocityY = 0.f;
    }
    else if (hitCeiling)
    {
        position.y = ceilingBottom;
        velocityY = -velocityY * ceilingRestitution;
    }

    constexpr float radiansToDegrees = 180.f / 3.14159265f;
    float distanceX = position.x - previousPosition.x;
    rotationDegrees = std::fmod(rotationDegrees + distanceX / radius * radiansToDegrees, 360.f);
    syncVisuals();

}    

void Ball::draw(sf::RenderWindow& window) const
{
    window.draw(shape);
    window.draw(stripe);
}

