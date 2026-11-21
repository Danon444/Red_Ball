#include "Enemy.h"

#include <algorithm>

Enemy::Enemy(sf::Vector2f startPosition, float endX, float speed)
    : startPosition(startPosition), endX(endX), speed(speed)
{
    body.setFillColor(sf::Color(75, 48, 108));
    reset();
}

void Enemy::setTexture(const sf::Texture& texture)
{
    body.setTexture(&texture, true);
    body.setFillColor(sf::Color::White);
}

void Enemy::update(float deltaTime)
{
    auto position = body.getPosition();
    position.x += direction * speed * deltaTime;
    if (position.x >= endX)
    {
        direction = -1.f;
    }
    else if (position.x <= startPosition.x)
    {
        direction = 1.f;
    }
    position.x = std::clamp(position.x, startPosition.x, endX);
    body.setPosition(position);
}

sf::FloatRect Enemy::getBounds() const
{
    return body.getGlobalBounds();
}

bool Enemy::isActive() const
{
    return active;
}

void Enemy::squash()
{
    active = false;
}

void Enemy::reset()
{
    body.setPosition(startPosition);
    direction = 1.f;
    active = true;
}

void Enemy::draw(sf::RenderWindow& window) const
{
    if (!active)
    {
        return;
    }

    window.draw(body);
}
