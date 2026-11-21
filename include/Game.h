#pragma once

#include <SFML/Graphics.hpp>
#include "Ball.h"
#include "Level.h"

class Game
{
private:
    enum class State
    {
        Playing,
        Won
    };

    State state = State::Playing;
    sf::RenderWindow window;
    sf::Texture ballTexture;
    sf::Texture enemyTexture;
    Ball player;

    Level level;

    sf::Clock clock;
    sf::Font font;
    sf::Text victoryText;
    sf::Text statusText;
    sf::View camera;
    sf::Vector2f respawnPosition;
    bool checkpointReached = false;

    void processEvents();
    void update(float deltaTime);
    void render();
    void updateCamera();

public:
    Game();
    void run();
};
