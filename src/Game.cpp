#include "Game.h"

Game::Game() : window(sf::VideoMode({800, 600}), "Red Ball")
{
    window.setKeyRepeatEnabled(false);
}

void Game::run()
{
    clock.restart();

    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        processEvents();

        if (!window.isOpen())
        {
            break;
        }

        update(deltaTime);
        render();
    }
}

void Game::processEvents()
{
    while (const auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

        player.handleEvent(*event);
    }
}

void Game::update(float deltaTime)
{
    player.update(deltaTime, level.getFloorY());
}

void Game::render()
{
    window.clear();
    level.draw(window);
    player.draw(window);
    window.display();
}