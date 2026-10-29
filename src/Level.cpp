#include "Level.h"

Level::Level()
{
    addSurface({0.f, 500.f}, {800.f, 100.f}, sf::Color::Green);

    sf::Color platformColor(100, 160, 240);
    addSurface({320.f, 410.f}, {180.f, 20.f}, platformColor);
    addSurface({560.f, 320.f}, {160.f, 20.f}, platformColor);
    addSurface({320.f, 230.f}, {180.f, 20.f}, platformColor);

    finishPole.setSize({5.f, 60.f});
    finishPole.setPosition({435.f, 170.f});
    finishPole.setFillColor(sf::Color::White);

    finishFlag.setSize({35.f, 20.f});
    finishFlag.setPosition({440.f, 170.f});
    finishFlag.setFillColor(sf::Color::Yellow);
}

void Level::addSurface(sf::Vector2f position, sf::Vector2f size, sf::Color color)
{
    sf::RectangleShape surface(size);
    surface.setPosition(position);
    surface.setFillColor(color);
    surfaces.push_back(surface);
}

std::vector<sf::FloatRect> Level::getSurfaceBounds() const
{
    std::vector<sf::FloatRect> bounds;

    for (const sf::RectangleShape& surface : surfaces)
    {
        bounds.push_back(surface.getGlobalBounds());
    }

    return bounds;
}

sf::FloatRect Level::getFinishBounds() const
{
    // A small trigger at the base of the pole, above the last platform.
    sf::Vector2f position = finishPole.getPosition();
    position.x -= 4.f;
    position.y += finishPole.getSize().y - 12.f;
    return {position, {13.f, 12.f}};
}

void Level::draw(sf::RenderWindow& window) const
{
    for (const sf::RectangleShape& surface : surfaces)
    {
        window.draw(surface);
    }

    window.draw(finishPole);
    window.draw(finishFlag);
}
