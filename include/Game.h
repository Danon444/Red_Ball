#pragma once

#include <SFML/Graphics.hpp>
#include "Ball.h"
#include "Level.h"

class Game
{
private:
    sf::RenderWindow window;
    Ball player;

    Level level;

    sf::Clock clock;

    void processEvents();
    void update(float deltaTime);
    void render();

public:
    Game();
    void run();
};