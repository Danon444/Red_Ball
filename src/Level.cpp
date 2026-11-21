#include "Level.h"
#include "Ball.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace
{
using nlohmann::json;

float readNumber(const json& object, const char* key)
{
    const auto& value = object.at(key);
    if (!value.is_number())
    {
        throw std::runtime_error(std::string(key) + " must be a number");
    }
    float number = value.get<float>();
    if (!std::isfinite(number))
    {
        throw std::runtime_error(std::string(key) + " is outside the supported range");
    }
    return number;
}

float readPositive(const json& object, const char* key)
{
    float number = readNumber(object, key);
    if (number <= 0.f)
    {
        throw std::runtime_error(std::string(key) + " must be greater than zero");
    }
    return number;
}

sf::Vector2f readPoint(const json& object)
{
    return {readNumber(object, "x"), readNumber(object, "y")};
}

sf::FloatRect readSurface(const json& object)
{
    return {readPoint(object), {readPositive(object, "width"), readPositive(object, "height")}};
}
}

Level::Level(const std::filesystem::path& path)
{
    sf::Vector2f checkpointBase;
    try
    {
        std::ifstream input(path);
        if (!input)
        {
            throw std::runtime_error("could not open file");
        }

        const auto data = json::parse(input);
        width = readPositive(data, "width");
        deathY = readPositive(data, "deathY");
        spawn = readPoint(data.at("spawn"));
        const auto& checkpoint = data.at("checkpoint");
        checkpointBase = readPoint(checkpoint.at("base"));
        checkpointSpawn = readPoint(checkpoint.at("spawn"));
        if (spawn.x >= width)
        {
            throw std::runtime_error("spawn.x must be to the left of the level exit");
        }

        const sf::Color earth(150, 105, 70);
        for (const auto& surface : data.at("surfaces").get_ref<const json::array_t&>())
        {
            auto bounds = readSurface(surface);
            addSurface(bounds.position, bounds.size, earth);
        }

        if (data.contains("movingPlatforms"))
        {
            for (const auto& surface : data.at("movingPlatforms").get_ref<const json::array_t&>())
            {
                movingPlatforms.emplace_back(readSurface(surface), readNumber(surface, "endX"),
                                             readPositive(surface, "period"));
            }
        }

        if (data.contains("enemies"))
        {
            for (const auto& enemy : data.at("enemies").get_ref<const json::array_t&>())
            {
                auto position = readPoint(enemy);
                float endX = readNumber(enemy, "endX");
                if (endX <= position.x)
                {
                    throw std::runtime_error("Enemy endX must be to the right of its start x");
                }
                enemies.emplace_back(position, endX, readPositive(enemy, "speed"));
            }
        }
    }
    catch (const std::exception& error)
    {
        throw std::runtime_error("Cannot load level " + path.string() + ": " + error.what());
    }

    checkpointPole.setSize({5.f, 70.f});
    checkpointPole.setPosition(checkpointBase - sf::Vector2f{0.f, checkpointPole.getSize().y});
    checkpointPole.setFillColor(sf::Color::White);
    checkpointFlag.setSize({36.f, 24.f});
    checkpointFlag.setPosition(checkpointPole.getPosition() + sf::Vector2f{checkpointPole.getSize().x, 0.f});
    reset();
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
    bounds.reserve(surfaces.size() + movingPlatforms.size());

    for (const sf::RectangleShape& surface : surfaces)
    {
        bounds.push_back(surface.getGlobalBounds());
    }

    for (const auto& platform : movingPlatforms)
    {
        bounds.push_back(platform.getBounds());
    }

    return bounds;
}

bool Level::hasReachedExit(float playerLeftX) const
{
    return playerLeftX >= width;
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

    for (const auto& platform : movingPlatforms)
    {
        platform.draw(window);
    }

    for (const auto& enemy : enemies)
    {
        enemy.draw(window);
    }

    window.draw(checkpointPole);
    window.draw(checkpointFlag);
}

sf::FloatRect Level::getCheckpointBounds() const
{
    const auto pole = checkpointPole.getPosition();
    return {{pole.x - 20.f, pole.y}, {50.f, checkpointPole.getSize().y}};
}

sf::Vector2f Level::getCheckpointSpawn() const
{
    return checkpointSpawn;
}

sf::Vector2f Level::getSpawn() const
{
    return spawn;
}

float Level::getWidth() const
{
    return width;
}

float Level::getDeathY() const
{
    return deathY;
}

void Level::activateCheckpoint()
{
    checkpointFlag.setFillColor(sf::Color(80, 240, 110));
}

void Level::setEnemyTexture(const sf::Texture& texture)
{
    for (auto& enemy : enemies)
    {
        enemy.setTexture(texture);
    }
}

void Level::resetEnemies()
{
    for (auto& enemy : enemies)
    {
        enemy.reset();
    }
}

void Level::reset()
{
    for (auto& platform : movingPlatforms)
    {
        platform.reset();
    }
    resetEnemies();
    checkpointFlag.setFillColor(sf::Color(100, 150, 240));
}

const std::vector<MovingPlatform>& Level::getMovingPlatforms() const
{
    return movingPlatforms;
}

void Level::updateMovingPlatforms(float deltaTime, Ball& player)
{
    for (auto& platform : movingPlatforms)
    {
        auto previousBounds = platform.getBounds();
        float shiftX = platform.update(deltaTime);
        player.followPlatform(previousBounds, shiftX);
    }
}

bool Level::updateEnemies(float deltaTime, Ball& player, const sf::FloatRect& previousPlayerBounds)
{
    for (auto& enemy : enemies)
    {
        if (!enemy.isActive())
        {
            continue;
        }

        enemy.update(deltaTime);
        auto playerBounds = player.getBounds();
        auto enemyBounds = enemy.getBounds();
        if (!player.intersects(enemyBounds))
        {
            continue;
        }

        const auto center = playerBounds.getCenter();
        const float enemyTop = enemyBounds.position.y;
        const float closestX = std::clamp(center.x, enemyBounds.position.x,
                                          enemyBounds.position.x + enemyBounds.size.x);
        const bool falling = playerBounds.position.y > previousPlayerBounds.position.y;
        const bool aboveEnemy = enemyTop - center.y > std::abs(center.x - closestX);
        if (falling && aboveEnemy)
        {
            enemy.squash();
            player.bounceFromEnemy(enemyTop);
            return false;
        }

        return true;
    }

    return false;
}
