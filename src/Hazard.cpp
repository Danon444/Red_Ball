#include "Hazard.h"
#include "Ball.h"

#include <algorithm>
#include <cmath>

Hazard::Hazard(sf::FloatRect bounds)
    : bounds(bounds)
{
    const int spikeCount = static_cast<int>(std::clamp(std::ceil(bounds.size.x / 24.f), 1.f, 64.f));
    const float spikeWidth = bounds.size.x / static_cast<float>(spikeCount);
    const float bottom = bounds.position.y + bounds.size.y;
    const sf::Color tipColor(255, 162, 148);
    const sf::Color baseColor(157, 42, 58);

    for (int i = 0; i < spikeCount; ++i)
    {
        const float left = bounds.position.x + static_cast<float>(i) * spikeWidth;
        spikes.append({{left, bottom}, baseColor});
        spikes.append({{left + spikeWidth / 2.f, bounds.position.y}, tipColor});
        spikes.append({{left + spikeWidth, bottom}, baseColor});
    }
}

bool Hazard::touches(const Ball& player) const
{
    return player.intersects(bounds);
}

void Hazard::draw(sf::RenderWindow& window) const
{
    window.draw(spikes);
}
