#pragma once

#include <SFML/Graphics.hpp>
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
        Lost,
        Won
    };

    State state = State::MainMenu;
    sf::RenderWindow window;
    sf::Texture ballTexture;
    sf::Texture enemyTexture;
    Ball player;

    std::optional<Level> level;

    sf::Clock clock;
    GameUI ui;
    sf::View camera;
    sf::Vector2f respawnPosition;
    bool checkpointReached = false;
    std::size_t levelIndex = 0;
    std::size_t completedLevels = 0;

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
