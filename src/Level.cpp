#include "Level.h"

Level::Level()
{
    const sf::Color earth(150, 105, 70);
    // A wide start, then small gaps and a gradual climb.
    addSurface({0.f, 500.f}, {680.f, 400.f}, earth);
    addSurface({780.f, 500.f}, {380.f, 400.f}, earth);
    addSurface({1260.f, 460.f}, {300.f, 440.f}, earth);
    addSurface({1640.f, 390.f}, {280.f, 510.f}, earth);
    addSurface({2000.f, 320.f}, {320.f, 580.f}, earth);
    addSurface({2400.f, 390.f}, {280.f, 24.f}, earth);
    addSurface({2760.f, 460.f}, {360.f, 440.f}, earth);

    // A safe resting place and checkpoint halfway through the route.
    addSurface({3220.f, 500.f}, {580.f, 400.f}, earth);

    // A suspended bridge, another resting place, and the final ascent.
    addSurface({3900.f, 450.f}, {200.f, 24.f}, earth);
    addSurface({4180.f, 380.f}, {200.f, 24.f}, earth);
    addSurface({4460.f, 310.f}, {220.f, 24.f}, earth);
    addSurface({4760.f, 380.f}, {220.f, 24.f}, earth);
    addSurface({5060.f, 450.f}, {320.f, 24.f}, earth);
    addSurface({5480.f, 500.f}, {400.f, 400.f}, earth);
    addSurface({5980.f, 430.f}, {220.f, 470.f}, earth);
    addSurface({6280.f, 350.f}, {220.f, 550.f}, earth);
    addSurface({6580.f, 280.f}, {220.f, 620.f}, earth);
    addSurface({6880.f, 350.f}, {320.f, 24.f}, earth);
    addSurface({7280.f, 420.f}, {520.f, 480.f}, earth);

    checkpointPole.setSize({5.f, 70.f});
    checkpointPole.setPosition({3500.f, 430.f});
    checkpointPole.setFillColor(sf::Color::White);
    checkpointFlag.setSize({36.f, 24.f});
    checkpointFlag.setPosition({3505.f, 430.f});
    reset();

    finishPole.setSize({5.f, 60.f});
    finishPole.setPosition({7550.f, 360.f});
    finishPole.setFillColor(sf::Color::White);

    finishFlag.setSize({35.f, 20.f});
    finishFlag.setPosition({7555.f, 360.f});
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
    sf::Vector2f position = finishPole.getPosition();
    position.x -= 4.f;
    position.y += finishPole.getSize().y - 12.f;
    return {position, {13.f, 12.f}};
}

void Level::draw(sf::RenderWindow& window) const
{
    // Distant scenery is decorative and never participates in collisions.
    for (int i = 0; i < 14; ++i)
    {
        sf::CircleShape hill(220.f, 48);
        hill.setScale({1.7f, 1.f});
        hill.setPosition({static_cast<float>(i) * 600.f - 150.f, 400.f});
        hill.setFillColor(sf::Color(108, 175, 155));
        window.draw(hill);

        sf::CircleShape cloud(28.f);
        cloud.setScale({2.5f, 0.7f});
        cloud.setPosition({static_cast<float>(i) * 600.f + 140.f, 75.f});
        cloud.setFillColor(sf::Color(235, 248, 250));
        window.draw(cloud);
    }

    for (const sf::RectangleShape& surface : surfaces)
    {
        window.draw(surface);
        sf::RectangleShape grass({surface.getSize().x, 8.f});
        grass.setPosition(surface.getPosition());
        grass.setFillColor(sf::Color(90, 155, 75));
        window.draw(grass);
    }

    window.draw(checkpointPole);
    window.draw(checkpointFlag);
    window.draw(finishPole);
    window.draw(finishFlag);
}

sf::FloatRect Level::getCheckpointBounds() const
{
    return {{3480.f, 430.f}, {50.f, 70.f}};
}

sf::Vector2f Level::getCheckpointSpawn() const
{
    return {3440.f, 440.f};
}

float Level::getWidth() const
{
    return 7800.f;
}

float Level::getDeathY() const
{
    return 700.f;
}

void Level::activateCheckpoint()
{
    checkpointActive = true;
    checkpointFlag.setFillColor(sf::Color(80, 240, 110));
}

void Level::reset()
{
    checkpointActive = false;
    checkpointFlag.setFillColor(sf::Color(100, 150, 240));
}
