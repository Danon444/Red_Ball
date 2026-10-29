#include "Game.h"

Game::Game()
    : window(sf::VideoMode({800, 600}), "Red Ball"),
      font("assets/fonts/arial.ttf"),
      victoryText(font, "LEVEL COMPLETE!\nEnter - restart", 36)
{
    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(120);

    victoryText.setFillColor(sf::Color::Yellow);
    victoryText.setOrigin(victoryText.getLocalBounds().getCenter());
    victoryText.setPosition({400.f, 300.f});
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
            return;
        }

        if (state == State::Won)
        {
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Enter)
                {
                    player.reset();
                    state = State::Playing;
                }
            }
        }
        else
        {
            player.handleEvent(*event);
        }
    }
}

void Game::update(float deltaTime)
{
    if (state == State::Won)
    {
        return;
    }

    std::vector<sf::FloatRect> surfaces = level.getSurfaceBounds();
    player.update(deltaTime, surfaces);

    float windowBottom = static_cast<float>(window.getSize().y);

    if (player.getPosition().y > windowBottom)
    {
        player.reset();
        return;
    }

    if (player.isOnGround() &&
        player.getBounds().findIntersection(level.getFinishBounds()))
    {
        state = State::Won;
    }
}

void Game::render()
{
    window.clear();
    level.draw(window);
    player.draw(window);

    if (state == State::Won)
    {
        sf::RectangleShape shade({800.f, 600.f});
        shade.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(shade);
        window.draw(victoryText);
    }

    window.display();
}
