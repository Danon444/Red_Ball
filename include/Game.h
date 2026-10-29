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
    Ball player;

    Level level;

    sf::Clock clock;
    sf::Font font;
    sf::Text victoryText;

    void processEvents();
    void update(float deltaTime);
    void render();

public:
    Game();
    void run();
};
