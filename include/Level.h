#pragma once

#include <SFML/Graphics.hpp>
#include <cstddef>
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
    struct Star
    {
        sf::FloatRect bounds;
        std::optional<std::size_t> guardIndex;
        bool collected = false;
    };

    std::vector<sf::RectangleShape> surfaces;
    std::vector<MovingPlatform> movingPlatforms;
    std::vector<Enemy> enemies;
    std::vector<Star> stars;
    std::optional<Crate> crate;
    float width;
    float deathY;
    sf::Vector2f spawn;
    sf::Vector2f checkpointSpawn;
    sf::RectangleShape checkpointPole;
    sf::RectangleShape checkpointFlag;

    void addSurface(sf::Vector2f position, sf::Vector2f size, sf::Color color);
    bool isStarLocked(const Star& star) const;
public:
    explicit Level(const std::filesystem::path& path = "assets/levels/level1.json");
    void updateMovingPlatforms(float deltaTime, Ball& player);
    bool updateEnemies(float deltaTime, Ball& player, const sf::FloatRect& previousPlayerBounds);
    void collectStars(const Ball& player);
    std::size_t getStarCount() const;
    std::size_t getCollectedStars() const;
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
    void draw(sf::RenderWindow& window, const sf::Texture& starTexture) const;
};
