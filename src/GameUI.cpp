#include "GameUI.h"

#include <utility>

GameUI::GameUI()
    : font("assets/fonts/arial.ttf"), heading(font, "", 48), menuDetail(font, "", 22), hud(font, "", 18)
{
    heading.setStyle(sf::Text::Bold);
    heading.setFillColor(sf::Color(255, 238, 207));
    menuDetail.setFillColor(sf::Color(249, 239, 222));
    hud.setPosition({18.f, 14.f});
    hud.setFillColor(sf::Color(24, 46, 54));
}

void GameUI::showMenu(const sf::String& title, std::vector<Option> newOptions, const sf::String& detail)
{
    heading.setString(title);
    heading.setOrigin(heading.getLocalBounds().getCenter());
    menuDetail.setString(detail);
    menuDetail.setOrigin(menuDetail.getLocalBounds().getCenter());
    options = std::move(newOptions);
    selected = 0;
    while (selected < options.size() && !options[selected].enabled)
    {
        ++selected;
    }
}

sf::FloatRect GameUI::buttonBounds(std::size_t index, sf::Vector2f size) const
{
    const float y = size.y / 2.f - 42.f + 62.f * static_cast<float>(index);
    return {{size.x / 2.f - 190.f, y}, {380.f, 50.f}};
}

std::optional<std::size_t> GameUI::buttonAt(sf::Vector2i pixel, const sf::RenderWindow& window) const
{
    const auto& view = window.getDefaultView();
    const auto point = window.mapPixelToCoords(pixel, view);
    for (std::size_t i = 0; i < options.size(); ++i)
    {
        if (options[i].enabled && buttonBounds(i, view.getSize()).contains(point))
        {
            return i;
        }
    }
    return std::nullopt;
}

void GameUI::moveSelection(int direction)
{
    for (std::size_t attempts = 0; attempts < options.size(); ++attempts)
    {
        if (direction < 0)
        {
            selected = selected == 0 ? options.size() - 1 : selected - 1;
        }
        else
        {
            selected = (selected + 1) % options.size();
        }

        if (options[selected].enabled)
        {
            return;
        }
    }
}

std::optional<std::size_t> GameUI::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W)
        {
            moveSelection(-1);
        }
        else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S)
        {
            moveSelection(1);
        }
        else if (key->code == sf::Keyboard::Key::Enter && selected < options.size() &&
                 options[selected].enabled)
        {
            return selected;
        }
    }
    else if (const auto* mouse = event.getIf<sf::Event::MouseMoved>())
    {
        if (const auto hovered = buttonAt(mouse->position, window))
        {
            selected = *hovered;
        }
    }
    else if (const auto* mouse = event.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mouse->button == sf::Mouse::Button::Left)
        {
            return buttonAt(mouse->position, window);
        }
    }
    return std::nullopt;
}

void GameUI::drawMenu(sf::RenderWindow& window, bool overlay) const
{
    const auto size = window.getDefaultView().getSize();
    if (overlay)
    {
        sf::RectangleShape shade(size);
        shade.setFillColor(sf::Color(15, 29, 36, 225));
        window.draw(shade);
    }

    auto title = heading;
    title.setPosition({size.x / 2.f, size.y / 2.f - 116.f});
    window.draw(title);

    if (!menuDetail.getString().isEmpty())
    {
        auto detail = menuDetail;
        detail.setPosition({size.x / 2.f, size.y / 2.f - 69.f});
        window.draw(detail);
    }

    sf::Text label(font, "", 23);
    for (std::size_t i = 0; i < options.size(); ++i)
    {
        const auto bounds = buttonBounds(i, size);
        const bool highlighted = i == selected && options[i].enabled;
        sf::RectangleShape button(bounds.size);
        button.setPosition(bounds.position);
        button.setFillColor(highlighted ? sf::Color(255, 207, 126) : sf::Color(41, 66, 75));
        window.draw(button);

        label.setString(options[i].label);
        label.setOrigin(label.getLocalBounds().getCenter());
        label.setPosition(bounds.getCenter());
        if (highlighted)
        {
            label.setFillColor(sf::Color(36, 31, 27));
        }
        else if (options[i].enabled)
        {
            label.setFillColor(sf::Color(249, 239, 222));
        }
        else
        {
            label.setFillColor(sf::Color(161, 184, 189));
        }
        window.draw(label);
    }
}

void GameUI::drawHud(sf::RenderWindow& window, const sf::String& message)
{
    hud.setString(message);
    window.draw(hud);
}
