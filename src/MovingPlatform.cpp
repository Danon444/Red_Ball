#include "MovingPlatform.h"

#include <cmath>

MovingPlatform::MovingPlatform(sf::FloatRect bounds, float endX, float period)
    : body(bounds.size),
      stripe({bounds.size.x, 8.f}),
      startPosition(bounds.position),
      endX(endX),
      period(period)
{
    body.setFillColor(sf::Color(36, 115, 141));
    stripe.setFillColor(sf::Color(255, 202, 84));
    reset();
}

sf::FloatRect MovingPlatform::getBounds() const
{
    return body.getGlobalBounds();
}

float MovingPlatform::update(float deltaTime)
{
    constexpr float fullTurn = 6.2831853f;
    float previousX = body.getPosition().x;
    elapsed = std::fmod(elapsed + deltaTime, period);

    // The factor goes from 0 to 1 and back over a full cycle.
    float phase = elapsed / period;
    float travel = (1.f - std::cos(fullTurn * phase)) / 2.f;
    float x = startPosition.x + (endX - startPosition.x) * travel;
    body.setPosition({x, startPosition.y});
    stripe.setPosition(body.getPosition());
    return x - previousX;
}

void MovingPlatform::reset()
{
    elapsed = 0.f;
    body.setPosition(startPosition);
    stripe.setPosition(startPosition);
}

void MovingPlatform::draw(sf::RenderWindow& window) const
{
    window.draw(body);
    window.draw(stripe);
}
