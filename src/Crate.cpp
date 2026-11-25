#include "Crate.h"

#include <algorithm>

Crate::Crate(sf::FloatRect bounds)
    : body(bounds.size),
      band({bounds.size.x, 8.f}),
      spawn(bounds.position)
{
    body.setFillColor(sf::Color(184, 118, 62));
    band.setFillColor(sf::Color(110, 65, 39));
    reset();
}

sf::FloatRect Crate::getBounds() const
{
    return body.getGlobalBounds();
}

void Crate::push(float shiftX, const std::vector<sf::FloatRect>& surfaces)
{
    auto position = body.getPosition();
    const auto size = body.getSize();
    float nextX = position.x + shiftX;
    for (const auto& surface : surfaces)
    {
        const auto end = surface.position + surface.size;
        if (position.y >= end.y || position.y + size.y <= surface.position.y)
        {
            continue;
        }

        if (shiftX > 0.f && position.x + size.x <= surface.position.x)
        {
            nextX = std::min(nextX, surface.position.x - size.x);
        }
        else if (shiftX < 0.f && position.x >= end.x)
        {
            nextX = std::max(nextX, end.x);
        }
    }
    body.setPosition({nextX, position.y});
}

void Crate::update(float deltaTime, const std::vector<sf::FloatRect>& surfaces)
{
    auto position = body.getPosition();
    const auto size = body.getSize();
    const float previousBottom = position.y + size.y;
    constexpr float gravity = 900.f;
    velocityY += gravity * deltaTime;
    float nextY = position.y + velocityY * deltaTime;

    for (const auto& surface : surfaces)
    {
        const float top = surface.position.y;
        const bool spansSurface = position.x < surface.position.x + surface.size.x &&
                                  position.x + size.x > surface.position.x;
        if (spansSurface && previousBottom <= top && nextY + size.y >= top)
        {
            nextY = top - size.y;
            velocityY = 0.f;
        }
    }
    body.setPosition({position.x, nextY});
}

void Crate::reset()
{
    body.setPosition(spawn);
    velocityY = 0.f;
}

void Crate::draw(sf::RenderWindow& window) const
{
    window.draw(body);
    sf::RectangleShape plank = band;
    plank.setPosition(body.getPosition());
    window.draw(plank);
    plank.move({0.f, body.getSize().y - plank.getSize().y});
    window.draw(plank);
}
