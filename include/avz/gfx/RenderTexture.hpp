#pragma once

#include <SFML/Graphics.hpp>
#include <avz/gfx/Sprite.hpp>

namespace avz
{

/**
 * Convenience extension of `sf::RenderTexture`
 */
class RenderTexture : public sf::RenderTexture
{
public:
	constexpr RenderTexture()
		: sf::RenderTexture{}
	{
	}

	constexpr RenderTexture(const sf::Vector2u size, unsigned antialiasing = 0)
		: sf::RenderTexture{size, {.antiAliasingLevel = antialiasing}}
	{
	}

	constexpr operator Sprite() const { return getTexture(); }
};

} // namespace avz
