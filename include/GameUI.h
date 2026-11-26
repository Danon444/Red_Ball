#pragma once

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <optional>
#include <vector>

class GameUI
{
public:
    struct Option
    {
        sf::String label;
        bool enabled = true;
    };

    GameUI();
    void showMenu(const sf::String& title, std::vector<Option> options, const sf::String& detail = {});
    std::optional<std::size_t> handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void drawMenu(sf::RenderWindow& window, bool overlay) const;
    void drawHud(sf::RenderWindow& window, const sf::String& message);

private:
    sf::Font font;
    sf::Text heading;
    sf::Text menuDetail;
    sf::Text hud;
    std::vector<Option> options;
    std::size_t selected = 0;

    sf::FloatRect buttonBounds(std::size_t index, sf::Vector2f size) const;
    std::optional<std::size_t> buttonAt(sf::Vector2i pixel, const sf::RenderWindow& window) const;
    void moveSelection(int direction);
};
