#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <vector>
#include "MovingPlatform.h"

class Ball;

class Level
{
private:
    std::vector<sf::RectangleShape> surfaces;
    std::vector<MovingPlatform> movingPlatforms;
    float width;
    float deathY;
    sf::Vector2f spawn;
    sf::Vector2f checkpointSpawn;
    sf::RectangleShape finishPole;
    sf::RectangleShape finishFlag;
    sf::RectangleShape checkpointPole;
    sf::RectangleShape checkpointFlag;

    void addSurface(sf::Vector2f position, sf::Vector2f size, sf::Color color);
public:
    explicit Level(const std::filesystem::path& path = "assets/levels/level1.json");
    void updateMovingPlatforms(float deltaTime, Ball& player);
    const std::vector<MovingPlatform>& getMovingPlatforms() const;

    std::vector<sf::FloatRect> getSurfaceBounds() const;
    sf::FloatRect getFinishBounds() const;
    sf::FloatRect getCheckpointBounds() const;
    sf::Vector2f getCheckpointSpawn() const;
    sf::Vector2f getSpawn() const;
    float getFinishX() const;
    float getWidth() const;
    float getDeathY() const;
    void activateCheckpoint();
    void reset();
    void draw(sf::RenderWindow& window) const;
};
