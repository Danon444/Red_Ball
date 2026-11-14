#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Level
{
private:
    std::vector<sf::RectangleShape> surfaces;
    sf::RectangleShape finishPole;
    sf::RectangleShape finishFlag;
    sf::RectangleShape checkpointPole;
    sf::RectangleShape checkpointFlag;
    bool checkpointActive = false;

    void addSurface(sf::Vector2f position, sf::Vector2f size, sf::Color color);
public:
    Level();

    std::vector<sf::FloatRect> getSurfaceBounds() const;
    sf::FloatRect getFinishBounds() const;
    sf::FloatRect getCheckpointBounds() const;
    sf::Vector2f getCheckpointSpawn() const;
    float getWidth() const;
    float getDeathY() const;
    void activateCheckpoint();
    void reset();
    void draw(sf::RenderWindow& window) const;
};
