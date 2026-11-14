#include "Game.h"

#include <algorithm>

Game::Game()
    : window(sf::VideoMode({1100, 700}), "Red Ball"),
      font("assets/fonts/arial.ttf"),
      victoryText(font, "LEVEL COMPLETE!\nEnter - restart", 36),
      statusText(font, "", 18),
      camera(sf::FloatRect({0.f, 0.f}, {1100.f, 700.f}))
{
    window.setKeyRepeatEnabled(false); // запрещает повторное нажатие при зажимании клавиши
    window.setFramerateLimit(120);

    victoryText.setFillColor(sf::Color::Yellow);
    victoryText.setOrigin(victoryText.getLocalBounds().getCenter());
    victoryText.setPosition({550.f, 350.f});
    statusText.setPosition({16.f, 12.f});
    statusText.setFillColor(sf::Color(30, 45, 55));
    updateCamera();
}

void Game::run()
{
    clock.restart();

    while (window.isOpen())
    {
        float deltaTime = std::min(clock.restart().asSeconds(), 0.05f);

        processEvents();

        if (!window.isOpen())
        {
            break;
        }

        // Small physics steps keep jumps stable during brief frame delays.
        while (deltaTime > 0.f)
        {
            float step = std::min(deltaTime, 1.f / 120.f);
            update(step);
            deltaTime -= step;
        }
        updateCamera();
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
                    level.reset();
                    checkpointReached = false;
                    respawnPosition = {100.f, 100.f};
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

    if (player.getPosition().y > level.getDeathY() ||
        player.getPosition().x < -60.f || player.getPosition().x > level.getWidth())
    {
        player.reset(respawnPosition);
        return;
    }

    if (!checkpointReached && player.isOnGround() &&
        player.getBounds().findIntersection(level.getCheckpointBounds()))
    {
        checkpointReached = true;
        respawnPosition = level.getCheckpointSpawn();
        level.activateCheckpoint();
    }

    if (player.isOnGround() &&
        player.getBounds().findIntersection(level.getFinishBounds()))
    {
        state = State::Won;
    }
}

void Game::render()
{
    window.clear(sf::Color(135, 206, 235));
    window.setView(camera);
    level.draw(window);
    player.draw(window);

    window.setView(window.getDefaultView());
    int progress = static_cast<int>(std::clamp(player.getPosition().x / 7550.f, 0.f, 1.f) * 100.f);
    std::string message = "A/D or arrows: move | W/Up/Space: jump\nRoute: " + std::to_string(progress) + "%";
    if (checkpointReached)
    {
        message += " | Checkpoint saved";
    }
    else
    {
        message += " | Reach the blue flag to save progress";
    }
    statusText.setString(message);
    window.draw(statusText);

    if (state == State::Won)
    {
        sf::RectangleShape shade({1100.f, 700.f});
        shade.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(shade);
        window.draw(victoryText);
    }

    window.display();
}

void Game::updateCamera()
{
    float centerX = player.getPosition().x + 30.f;
    centerX = std::clamp(centerX, 550.f, level.getWidth() - 550.f);
    camera.setCenter({centerX, 350.f});
}
