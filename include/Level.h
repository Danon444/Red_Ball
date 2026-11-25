#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <optional>
#include <vector>
#include "Crate.h"
#include "Enemy.h"
#include "MovingPlatform.h"

class Ball;

class Level
{
private:
    std::vector<sf::RectangleShape> surfaces;
    std::vector<MovingPlatform> movingPlatforms;
    std::vector<Enemy> enemies;
    std::optional<Crate> crate;
    float width;
    float deathY;
    sf::Vector2f spawn;
    sf::Vector2f checkpointSpawn;
    sf::RectangleShape checkpointPole;
    sf::RectangleShape checkpointFlag;

    void addSurface(sf::Vector2f position, sf::Vector2f size, sf::Color color);
public:
    explicit Level(const std::filesystem::path& path = "assets/levels/level1.json");
    void updateMovingPlatforms(float deltaTime, Ball& player);
    bool updateEnemies(float deltaTime, Ball& player, const sf::FloatRect& previousPlayerBounds);
    void setEnemyTexture(const sf::Texture& texture);
    Crate* getCrate();
    const std::vector<MovingPlatform>& getMovingPlatforms() const;

    std::vector<sf::FloatRect> getSurfaceBounds() const;
    bool hasReachedExit(float playerLeftX) const;
    sf::FloatRect getCheckpointBounds() const;
    sf::Vector2f getCheckpointSpawn() const;
    sf::Vector2f getSpawn() const;
    float getWidth() const;
    float getDeathY() const;
    void activateCheckpoint();
    void resetEnemies();
    void reset();
    void draw(sf::RenderWindow& window) const;
};
