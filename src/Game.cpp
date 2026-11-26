#include "Game.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace
{
constexpr std::array levelFiles{
    "assets/levels/level1.json",
    "assets/levels/level2.json",
    "assets/levels/level3.json"
};
constexpr auto mainMenuLabel = L"Главное меню";
constexpr auto retryLabel = L"Начать заново";
constexpr auto progressFile = "progress.json";

std::size_t loadProgress()
{
    std::ifstream input(progressFile);
    const auto data = nlohmann::json::parse(input, nullptr, false);
    const auto count = data.find("completedLevels");
    if (count == data.end() || !count->is_number_unsigned())
    {
        return 0;
    }

    if (*count > levelFiles.size())
    {
        return levelFiles.size();
    }

    return count->get<std::size_t>();
}

bool saveProgress(std::size_t completedLevels)
{
    constexpr auto temporaryFile = "progress.json.tmp";
    std::ofstream output(temporaryFile);
    output << nlohmann::json{{"completedLevels", completedLevels}}.dump(2) << '\n';
    output.close();
    if (!output)
    {
        return false;
    }

    std::error_code error;
    std::filesystem::rename(temporaryFile, progressFile, error);
    return !error;
}
}

Game::Game()
    : window(sf::VideoMode({1100, 700}), "Red Ball"),
      // Use the visible part of each PNG so the art reaches the ground.
      ballTexture("assets/sprites/ball.png", false, {{23, 46}, {1183, 1187}}),
      enemyTexture("assets/sprites/enemy.png", false, {{87, 117}, {1081, 1044}}),
      camera(sf::FloatRect({0.f, 0.f}, {1100.f, 700.f})),
      completedLevels(loadProgress())
{
    window.setKeyRepeatEnabled(false); // запрещает автоповтор событий при удержании клавиши
    window.setFramerateLimit(120);
    ballTexture.setSmooth(true);
    enemyTexture.setSmooth(true);
    (void)ballTexture.generateMipmap();
    (void)enemyTexture.generateMipmap();
    player.setTexture(ballTexture);
    openMainMenu();
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
        while (deltaTime > 0.f && state == State::Playing)
        {
            float step = std::min(deltaTime, 1.f / 120.f);
            update(step);
            deltaTime -= step;
        }
        if (level)
        {
            updateCamera();
        }
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

        const auto* key = event->getIf<sf::Event::KeyPressed>();
        if (key && key->code == sf::Keyboard::Key::Escape && state != State::MainMenu)
        {
            openMainMenu();
            continue;
        }

        if (state == State::Playing)
        {
            player.handleEvent(*event);
        }
        else if (const auto choice = ui.handleEvent(*event, window))
        {
            activateMenuOption(*choice);
            if (!window.isOpen())
            {
                return;
            }
        }
    }
}

void Game::openMainMenu()
{
    level.reset();
    state = State::MainMenu;
    ui.showMenu(L"Red Ball", {{L"Играть"}, {L"Выход"}});
}

void Game::openLevelSelect()
{
    state = State::LevelSelect;
    std::vector<GameUI::Option> options;
    for (std::size_t i = 0; i < levelFiles.size(); ++i)
    {
        std::wstring label = L"Уровень " + std::to_wstring(i + 1);
        if (i < completedLevels)
        {
            label += L" — пройден";
        }
        else if (i > completedLevels)
        {
            label += L" — закрыт";
        }
        options.push_back({label, i <= completedLevels});
    }
    options.push_back({L"Назад"});
    ui.showMenu(L"Выбор уровня", std::move(options));
}

void Game::startLevel(std::size_t index)
{
    level.emplace(levelFiles.at(index));
    level->setEnemyTexture(enemyTexture);
    levelIndex = index;
    restartLevel();
}

void Game::activateMenuOption(std::size_t index)
{
    switch (state)
    {
        case State::MainMenu:
            if (index == 0)
            {
                openLevelSelect();
            }
            else
            {
                window.close();
            }
            break;

        case State::LevelSelect:
            if (index == levelFiles.size())
            {
                openMainMenu();
            }
            else
            {
                startLevel(index);
            }
            break;

        case State::Lost:
            if (checkpointReached && index == 0)
            {
                player.reset(respawnPosition);
                state = State::Playing;
                updateCamera();
            }
            else if (index == (checkpointReached ? 1u : 0u))
            {
                restartLevel();
            }
            else
            {
                openMainMenu();
            }
            break;

        case State::Won:
            if (index == 1)
            {
                openMainMenu();
            }
            else if (levelIndex + 1 < levelFiles.size())
            {
                startLevel(levelIndex + 1);
            }
            else
            {
                restartLevel();
            }
            break;

        case State::Playing:
            break;
    }
}

void Game::restartLevel()
{
    level->reset();
    checkpointReached = false;
    respawnPosition = level->getSpawn();
    player.reset(respawnPosition);
    state = State::Playing;
    updateCamera();
}

void Game::update(float deltaTime)
{
    auto previousPlayerBounds = player.getBounds();
    level->updateMovingPlatforms(deltaTime, player);
    std::vector<sf::FloatRect> surfaces = level->getSurfaceBounds();
    auto* crate = level->getCrate();
    if (crate)
    {
        crate->update(deltaTime, surfaces);
        if (crate->getBounds().position.y > level->getDeathY())
        {
            crate->reset();
        }
    }
    player.update(deltaTime, surfaces, crate);
    bool hitEnemy = level->updateEnemies(deltaTime, player, previousPlayerBounds);

    if (hitEnemy || player.getPosition().y > level->getDeathY() ||
        player.getPosition().x < -60.f)
    {
        state = State::Lost;
        if (checkpointReached)
        {
            ui.showMenu(L"Поражение", {{L"Продолжить с чекпоинта"}, {retryLabel}, {mainMenuLabel}});
        }
        else
        {
            ui.showMenu(L"Поражение", {{retryLabel}, {mainMenuLabel}});
        }
        return;
    }

    if (!checkpointReached && player.isOnGround() &&
        player.intersects(level->getCheckpointBounds()))
    {
        checkpointReached = true;
        respawnPosition = level->getCheckpointSpawn();
        level->activateCheckpoint();
    }

    if (level->hasReachedExit(player.getPosition().x))
    {
        const auto savedLevels = loadProgress();
        completedLevels = std::max({completedLevels, savedLevels, levelIndex + 1});
        state = State::Won;
        std::wstring title = L"Уровень " + std::to_wstring(levelIndex + 1) + L" пройден";
        if (completedLevels > savedLevels && !saveProgress(completedLevels))
        {
            title += L"\nНе удалось сохранить прогресс";
        }
        const auto firstOption = levelIndex + 1 < levelFiles.size() ? L"Следующий уровень" : L"Сыграть ещё раз";
        ui.showMenu(title, {{firstOption}, {mainMenuLabel}});
    }
}

void Game::render()
{
    if (state == State::MainMenu || state == State::LevelSelect)
    {
        window.setView(window.getDefaultView());
        window.clear(sf::Color(19, 39, 48));
        ui.drawMenu(window, false);
        window.display();
        return;
    }

    window.clear(sf::Color(135, 206, 235));
    window.setView(camera);
    level->draw(window);
    player.draw(window);

    window.setView(window.getDefaultView());
    float startX = level->getSpawn().x;
    float routeLength = level->getWidth() - startX;
    float routeFraction = (player.getPosition().x - startX) / routeLength;
    int progress = static_cast<int>(std::clamp(routeFraction, 0.f, 1.f) * 100.f);
    std::wstring message = L"A/D или стрелки — движение   W/↑/пробел — прыжок   Esc — меню\nУровень " +
                           std::to_wstring(levelIndex + 1) + L" — " + std::to_wstring(progress) + L"%";
    if (checkpointReached)
    {
        message += L"   Чекпоинт активирован";
    }
    else if (level->getCrate())
    {
        message += L"   Подкати ящик к уступу";
    }
    else
    {
        message += L"   Доберись до синего флага";
    }
    ui.drawHud(window, message);

    if (state == State::Lost || state == State::Won)
    {
        ui.drawMenu(window, true);
    }

    window.display();
}

void Game::updateCamera()
{
    float centerX = player.getPosition().x + 30.f;
    float margin = std::min(camera.getSize().x / 2.f, level->getWidth() / 2.f);
    centerX = std::clamp(centerX, margin, level->getWidth() - margin);
    camera.setCenter({centerX, 350.f});
}
