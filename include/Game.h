#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include "Ball.h"
#include "GameUI.h"
#include "Level.h"

class Game
{
private:
    enum class State
    {
        MainMenu,
        LevelSelect,
        Playing,
        Paused,
        Lost,
        Won
    };

    static constexpr std::array levelFiles{
        "assets/levels/level1.json",
        "assets/levels/level2.json",
        "assets/levels/level3.json"
    };

    struct Progress
    {
        std::size_t completedLevels = 0;
        std::array<std::size_t, levelFiles.size()> bestStars{};
    };

    State state = State::MainMenu;
    sf::RenderWindow window;
    sf::Texture ballTexture;
    sf::Texture enemyTexture;
    sf::Texture starTexture;
    Ball player;

    std::optional<Level> level;

    sf::Clock clock;
    GameUI ui;
    sf::View camera;
    sf::Vector2f respawnPosition;
    bool checkpointReached = false;
    std::size_t levelIndex = 0;
    std::array<std::size_t, levelFiles.size()> starTotals{};
    Progress progress;

    Progress loadProgress() const;
    bool saveProgress() const;
    void openMainMenu();
    void openLevelSelect();
    void startLevel(std::size_t index);
    void activateMenuOption(std::size_t index);
    void restartLevel();
    void processEvents();
    void update(float deltaTime);
    void render();
    void updateCamera();

public:
    Game();
    void run();
};
