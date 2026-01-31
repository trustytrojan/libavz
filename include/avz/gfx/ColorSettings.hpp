#pragma once

#include <SFML/Graphics.hpp>
#include <print>

namespace avz
{

class ColorSettings
{
public:
	enum class Mode
	{
		WHEEL,
		WHEEL_RANGES,
		WHEEL_RANGES_REVERSE,
		SOLID
	};

	Mode mode{Mode::WHEEL};
	sf::Color solid{255, 255, 255};
	class
	{
		friend ColorSettings;
		float time{};

	public:
		float rate{};
		sf::Vector3f hsv{0.9, 0.7, 1}, start_hsv{0.9, 0.7, 1}, end_hsv{.5, .2, 1};
		constexpr void increment_time() { time += rate; }
	} wheel;

	constexpr void set_mode(Mode m) { this->mode = m; }
	constexpr void set_solid_color(sf::Color color) { solid = color; }
	constexpr void set_wheel_hsv(sf::Vector3f hsv) { wheel.hsv = hsv; }
	constexpr void set_wheel_ranges_start_hsv(sf::Vector3f hsv) { wheel.start_hsv = hsv; }
	constexpr void set_wheel_ranges_end_hsv(sf::Vector3f hsv) { wheel.end_hsv = hsv; }
	constexpr void set_wheel_rate(float rate) { wheel.rate = rate; }
	constexpr void increment_wheel_time() { wheel.increment_time(); }

	/**
	 * @param index_ratio the ratio of your loop index (`i`) to the total number of bars to print (`bars.size()`)
	 */
	sf::Color calculate_color(float index_ratio) const;

#ifdef LIBAVZ_IMGUI
	/**
	 * Render ImGui controls for color settings.
	 */
	void imgui();

private:
	std::string imgui_header{std::format("ColorSettings @ {}", (void *)this)};
#endif
};

} // namespace avz
