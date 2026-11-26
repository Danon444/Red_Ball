#include "Game.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace
{
constexpr auto mainMenuLabel = L"Главное меню";
constexpr auto retryLabel = L"Начать заново";
constexpr auto progressFile = "progress.json";

std::size_t readCount(const nlohmann::json& count, std::size_t maximum)
{
    if (!count.is_number_unsigned())
    {
        return 0;
    }

    if (count > maximum)
    {
        return maximum;
    }

    return count.get<std::size_t>();
}

std::size_t readStarTotal(const char* filename)
{
    std::ifstream input(filename);
    const auto data = nlohmann::json::parse(input, nullptr, false);
    const auto stars = data.find("stars");
    if (stars != data.end() && stars->is_array())
    {
        return stars->size();
    }
    return 0;
}

std::wstring formatStarCount(const Level& level)
{
    return std::to_wstring(level.getCollectedStars()) + L" из " + std::to_wstring(level.getStarCount());
}
}

Game::Progress Game::loadProgress() const
{
    std::ifstream input(progressFile);
    const auto data = nlohmann::json::parse(input, nullptr, false);
    Progress saved;
    if (const auto count = data.find("completedLevels"); count != data.end())
    {
        saved.completedLevels = readCount(*count, levelFiles.size());
    }
    if (const auto stars = data.find("bestStars"); stars != data.end() && stars->is_array())
    {
        for (std::size_t i = 0; i < std::min(stars->size(), saved.bestStars.size()); ++i)
        {
            saved.bestStars[i] = readCount((*stars)[i], starTotals[i]);
        }
    }
    return saved;
}

bool Game::saveProgress() const
{
    constexpr auto temporaryFile = "progress.json.tmp";
    std::ofstream output(temporaryFile);
    output << nlohmann::json{
        {"completedLevels", progress.completedLevels},
        {"bestStars", progress.bestStars}
    }.dump(2) << '\n';
    output.close();
    if (!output)
    {
        return false;
    }

    std::error_code error;
    std::filesystem::rename(temporaryFile, progressFile, error);
    return !error;
}

Game::Game()
    : window(sf::VideoMode({1100, 700}), "Red Ball"),
      // Use the visible part of each PNG so the art reaches the ground.
      ballTexture("assets/sprites/ball.png", false, {{23, 46}, {1183, 1187}}),
      enemyTexture("assets/sprites/enemy.png", false, {{87, 117}, {1081, 1044}}),
      starTexture("assets/sprites/star.png"),
      camera(sf::FloatRect({0.f, 0.f}, {1100.f, 700.f}))
{
    window.setKeyRepeatEnabled(false); // запрещает автоповтор событий при удержании клавиши
    window.setFramerateLimit(120);
    ballTexture.setSmooth(true);
    enemyTexture.setSmooth(true);
    starTexture.setSmooth(true);
    (void)ballTexture.generateMipmap();
    (void)enemyTexture.generateMipmap();
    player.setTexture(ballTexture);
    for (std::size_t i = 0; i < levelFiles.size(); ++i)
    {
        starTotals[i] = readStarTotal(levelFiles[i]);
    }
    progress = loadProgress();
    openMainMenu();
}

void Game::run()
{
    clock.restart();

    while (window.isOpen())
    {
        float deltaTime = std::min(clock.restart().asSeconds(), 0.05f);
        const bool wasPlaying = state == State::Playing;

        processEvents();

        if (!window.isOpen())
        {
            break;
        }

        // Small physics steps keep jumps stable during brief frame delays.
        while (wasPlaying && deltaTime > 0.f && state == State::Playing)
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
            if (state == State::Playing)
            {
                state = State::Paused;
                ui.showMenu(L"Пауза", {{L"Продолжить"}, {retryLabel}, {mainMenuLabel}});
            }
            else if (state == State::Paused)
            {
                state = State::Playing;
            }
            else
            {
                openMainMenu();
            }
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
        if (i < progress.completedLevels)
        {
            label += L" — пройден";
        }
        else if (i > progress.completedLevels)
        {
            label += L" — закрыт";
        }
        if (starTotals[i] > 0)
        {
            label += L" · " + std::to_wstring(progress.bestStars[i]) + L"/" + std::to_wstring(starTotals[i]) + L" звёзд";
        }
        options.push_back({label, i <= progress.completedLevels});
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

        case State::Paused:
            if (index == 0)
            {
                state = State::Playing;
            }
            else if (index == 1)
            {
                restartLevel();
            }
            else
            {
                openMainMenu();
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

    level->collectStars(player);

    if (!checkpointReached && player.isOnGround() &&
        player.intersects(level->getCheckpointBounds()))
    {
        checkpointReached = true;
        respawnPosition = level->getCheckpointSpawn();
        level->activateCheckpoint();
    }

    if (level->hasReachedExit(player.getPosition().x))
    {
        const auto saved = loadProgress();
        progress.completedLevels = std::max({progress.completedLevels, saved.completedLevels, levelIndex + 1});
        for (std::size_t i = 0; i < progress.bestStars.size(); ++i)
        {
            progress.bestStars[i] = std::max(progress.bestStars[i], saved.bestStars[i]);
        }
        auto& bestStars = progress.bestStars[levelIndex];
        bestStars = std::max(bestStars, level->getCollectedStars());
        state = State::Won;
        const std::wstring title = L"Уровень " + std::to_wstring(levelIndex + 1) + L" пройден";
        std::wstring detail;
        if (level->getStarCount() > 0)
        {
            detail = L"Собрано " + formatStarCount(*level) + L" звёзд";
        }
        const bool progressChanged = progress.completedLevels != saved.completedLevels || progress.bestStars != saved.bestStars;
        if (progressChanged && !saveProgress())
        {
            if (!detail.empty())
            {
                detail += L"   •   ";
            }
            detail += L"Не удалось сохранить прогресс";
        }
        const auto firstOption = levelIndex + 1 < levelFiles.size() ? L"Следующий уровень" : L"Сыграть ещё раз";
        ui.showMenu(title, {{firstOption}, {mainMenuLabel}}, detail);
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
    level->draw(window, starTexture);
    player.draw(window);

    window.setView(window.getDefaultView());
    float startX = level->getSpawn().x;
    float routeLength = level->getWidth() - startX;
    float routeFraction = (player.getPosition().x - startX) / routeLength;
    int progress = static_cast<int>(std::clamp(routeFraction, 0.f, 1.f) * 100.f);
    std::wstring message = L"A/D или стрелки — движение   W/↑/пробел — прыжок   Esc — пауза\nУровень " +
                           std::to_wstring(levelIndex + 1) + L" — " + std::to_wstring(progress) + L"%";
    if (level->getStarCount() > 0)
    {
        message += L"   Звёзды " + formatStarCount(*level);
    }
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

    if (state == State::Paused || state == State::Lost || state == State::Won)
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
