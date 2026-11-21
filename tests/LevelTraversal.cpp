#include "Ball.h"
#include "Level.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message)
{
    if (!condition) { throw std::runtime_error(message); }
}

std::vector<sf::FloatRect> surfacesFromLeftToRight(const Level& level)
{
    auto surfaces = level.getSurfaceBounds();
    std::sort(surfaces.begin(), surfaces.end(), [](const auto& left, const auto& right)
    {
        return left.position.x < right.position.x;
    });
    return surfaces;
}

// Drives the real physics with deterministic controls and no window.
void traverse(float step)
{
    Level level;
    Ball ball;
    ball.reset(level.getSpawn());
    auto surfaces = surfacesFromLeftToRight(level);
    std::size_t platform = 0;
    bool checkpointSeen = false;
    bool finished = false;
    for (int frame = 0; frame < static_cast<int>(90.f / step); ++frame)
    {
        float direction = 1.f;
        if (ball.isOnGround())
        {
            const auto bounds = ball.getBounds();
            for (std::size_t i = platform; i < surfaces.size(); ++i)
            {
                if (std::abs(bounds.position.y + bounds.size.y - surfaces[i].position.y) < 0.1f &&
                    bounds.position.x + bounds.size.x > surfaces[i].position.x &&
                    bounds.position.x < surfaces[i].position.x + surfaces[i].size.x)
                {
                    platform = i;
                }
            }
            if (bounds.findIntersection(level.getCheckpointBounds())) { checkpointSeen = true; }
            if (bounds.findIntersection(level.getFinishBounds())) { finished = true; break; }
            float edge = surfaces[platform].position.x + surfaces[platform].size.x;
            if (platform + 1 < surfaces.size() && bounds.position.x >= edge - 80.f)
            {
                auto ferry = level.getMovingPlatforms().front().getBounds();
                bool boarding = surfaces[platform + 1].position == ferry.position;
                bool leaving = surfaces[platform].position == ferry.position;
                float gap = surfaces[platform + 1].position.x - edge;
                if ((boarding || leaving) && gap > 75.f)
                {
                    direction = 0.f;
                }
                else
                {
                    sf::Event::KeyPressed key{};
                    key.code = sf::Keyboard::Key::Space;
                    ball.handleEvent(sf::Event(key));
                }
            }
        }
        level.updateMovingPlatforms(step, ball);
        surfaces = surfacesFromLeftToRight(level);
        ball.update(step, surfaces, direction);
        if (ball.getPosition().y > level.getDeathY())
        {
            std::cerr << "Fall after platform " << platform << " at x=" << ball.getPosition().x << '\n';
            throw std::runtime_error("The route could not be traversed");
        }
    }
    require(checkpointSeen, "Checkpoint was missed");
    require(finished, "Finish was not reached");
    ball.reset(level.getCheckpointSpawn());
    const float respawnX = ball.getPosition().x;
    for (int i = 0; i < 120; ++i) { ball.update(1.f / 120.f, surfaces, 0.f); }
    require(ball.isOnGround(), "Checkpoint respawn has no support");
    require(std::abs(ball.getPosition().x - respawnX) < 0.01f, "Respawn retained horizontal momentum");
    require(!ball.getBounds().findIntersection(level.getFinishBounds()), "Checkpoint overlaps finish");
    ball.reset(level.getSpawn());
    require(ball.getPosition() == level.getSpawn(), "Full restart did not return to the start");
}

int main()
{
    try
    {
        traverse(1.f / 120.f);
        traverse(1.f / 240.f);
        std::cout << "Route, checkpoint respawn, and restart checks passed.\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
