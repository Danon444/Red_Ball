#include "../include/Ball.h"
#include "Crate.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float contactTolerance = 0.002f;
constexpr float groundNormalY = -0.5f;

struct CircleContact
{
    sf::Vector2f normal;
    float depth;
};

std::optional<CircleContact> circleContact(sf::Vector2f center, float radius,
                                           const sf::FloatRect& rectangle,
                                           float tolerance = 0.f)
{
    const auto min = rectangle.position;
    const auto max = min + rectangle.size;
    const sf::Vector2f nearest{
        std::clamp(center.x, min.x, max.x),
        std::clamp(center.y, min.y, max.y)
    };
    const auto offset = center - nearest;
    const float distanceSquared = offset.x * offset.x + offset.y * offset.y;
    const float contactRadius = radius + tolerance;
    if (distanceSquared > contactRadius * contactRadius)
    {
        return std::nullopt;
    }

    if (distanceSquared > 0.f)
    {
        const float distance = std::sqrt(distanceSquared);
        return CircleContact{offset / distance, radius - distance};
    }

    // If the center is inside the rectangle, push out through its nearest face.
    const float left = center.x - min.x;
    const float right = max.x - center.x;
    const float top = center.y - min.y;
    const float bottom = max.y - center.y;
    const float nearestFace = std::min({left, right, top, bottom});
    if (nearestFace == top)
    {
        return CircleContact{{0.f, -1.f}, radius + top};
    }
    if (nearestFace == bottom)
    {
        return CircleContact{{0.f, 1.f}, radius + bottom};
    }
    if (nearestFace == left)
    {
        return CircleContact{{-1.f, 0.f}, radius + left};
    }
    return CircleContact{{1.f, 0.f}, radius + right};
}
}

Ball::Ball()
{
    reset();
}

void Ball::setTexture(const sf::Texture& texture)
{
    sprite.emplace(texture);
    auto size = sf::Vector2f(texture.getSize());
    float diameter = radius * 2.f;
    sprite->setOrigin(size / 2.f);
    sprite->setScale({diameter / size.x, diameter / size.y});
    syncVisuals();
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

bool Ball::intersects(const sf::FloatRect& rectangle) const
{
    return circleContact(position + sf::Vector2f{radius, radius}, radius, rectangle).has_value();
}

void Ball::syncVisuals()
{
    if (sprite)
    {
        sprite->setPosition(position + sf::Vector2f{radius, radius});
        sprite->setRotation(sf::degrees(rotationDegrees));
    }
}

bool Ball::isOnGround() const
{
    return onGround;
}

void Ball::bounceFromEnemy(float enemyTop)
{
    position.y = enemyTop - radius * 2.f;
    velocityY = -300.f;
    onGround = false;
    syncVisuals();
}

void Ball::followPlatform(const sf::FloatRect& previousBounds, float shiftX)
{
    const auto center = position + sf::Vector2f{radius, radius};
    const auto support = circleContact(center, radius, previousBounds, contactTolerance);
    if (onGround && support && support->normal.y <= groundNormalY)
    {
        position.x += shiftX;
        return;
    }

    auto currentBounds = previousBounds;
    currentBounds.position.x += shiftX;
    const auto contact = circleContact(center, radius, currentBounds);
    if (contact && contact->depth > 0.f && contact->normal.x * shiftX > 0.f)
    {
        position += contact->normal * contact->depth;
    }
}

bool Ball::handleEvent(const sf::Event& event)
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
            return true;
        }
    }
    return false;
}

void Ball::update(float deltaTime, const std::vector<sf::FloatRect>& surfaces, Crate* crate)
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

    update(deltaTime, surfaces, direction, crate);
}

void Ball::update(float deltaTime, const std::vector<sf::FloatRect>& surfaces, float direction,
                  Crate* crate)
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

    const auto previousPosition = position;
    velocityY += gravity * deltaTime;
    sf::Vector2f velocity{velocityX, velocityY};
    position += velocity * deltaTime;

    // Repeat corrections when a push out of one surface reaches another.
    for (int pass = 0; pass < 4; ++pass)
    {
        onGround = false;
        bool correctedPosition = false;
        auto resolveContact = [&](const sf::FloatRect& surface)
        {
            const auto center = position + sf::Vector2f{radius, radius};
            const auto contact = circleContact(center, radius, surface, contactTolerance);
            if (!contact)
            {
                return;
            }

            if (contact->depth > contactTolerance)
            {
                position += contact->normal * contact->depth;
                correctedPosition = true;
            }

            const auto normal = contact->normal;
            const float speedIntoSurface = velocity.x * normal.x + velocity.y * normal.y;
            if (speedIntoSurface <= 0.01f && normal.y <= groundNormalY)
            {
                onGround = true;
            }

            if (speedIntoSurface < 0.f)
            {
                float response = 1.f;
                if (normal.y >= -groundNormalY)
                {
                    response += ceilingRestitution;
                }
                // Cancel movement into the surface, leaving motion along it.
                velocity -= normal * (speedIntoSurface * response);
            }
        };
        for (const auto& surface : surfaces)
        {
            resolveContact(surface);
        }
        if (crate)
        {
            const auto center = position + sf::Vector2f{radius, radius};
            auto contact = circleContact(center, radius, crate->getBounds(), contactTolerance);
            if (contact && contact->depth > contactTolerance && std::abs(contact->normal.x) > 0.9f &&
                velocity.x * contact->normal.x < 0.f)
            {
                crate->push(-contact->depth / contact->normal.x, surfaces);
                contact = circleContact(center, radius, crate->getBounds(), contactTolerance);
            }
            // Keep lateral speed when the crate moved far enough to leave only a touch.
            if (contact && (contact->depth > contactTolerance || std::abs(contact->normal.x) <= 0.9f))
            {
                resolveContact(crate->getBounds());
            }
        }
        if (!correctedPosition)
        {
            break;
        }
    }
    velocityX = velocity.x;
    velocityY = velocity.y;

    constexpr float radiansToDegrees = 180.f / 3.14159265f;
    float distanceX = position.x - previousPosition.x;
    rotationDegrees = std::fmod(rotationDegrees + distanceX / radius * radiansToDegrees, 360.f);
    syncVisuals();

}    

void Ball::draw(sf::RenderWindow& window) const
{
    if (sprite)
    {
        window.draw(*sprite);
    }
}

