#pragma once

#include <SFML/Graphics.hpp>
#include <avz/gfx/ColorSettings.hpp>
#include <avz/gfx/util.hpp>
#include <span>

namespace avz
{

/**
 * A customizable frequency spectrum visualizer using a single mesh for efficient rendering.
 * Uses sf::TriangleStrip for batched drawing of all bars in a single draw call.
 */
class SpectrumDrawable : public sf::Drawable
{
	const ColorSettings &color;
	float multiplier{1};
	sf::VertexArray vertex_array;
	sf::IntRect rect;
	bool backwards{};
	bool debug_rect{};
	bool use_gs{false};

	struct
	{
		int width{10}, spacing{5}, count{};
	} bar;

#ifdef LIBAVZ_IMGUI
	std::string imgui_header{std::format("SpectrumDrawable @ {}", (void *)this)};
#endif

public:
	SpectrumDrawable(const ColorSettings &color, const bool backwards = false);
	SpectrumDrawable(const sf::IntRect &rect, const ColorSettings &color, const bool backwards = false);

	void set_bar_width(const int width);
	void set_bar_spacing(const int spacing);
	constexpr int get_bar_width() const { return bar.width; }
	constexpr int get_bar_spacing() const { return bar.spacing; }
	constexpr int get_bar_count() const { return bar.count; }

	constexpr void set_debug_rect(bool b) { debug_rect = b; }
	constexpr bool get_debug_rect() const { return debug_rect; }

	constexpr void set_multiplier(const float multiplier) { this->multiplier = multiplier; }
	constexpr float get_multiplier() const { return multiplier; }

	void set_use_gs(bool b);
	constexpr bool get_use_gs() const { return use_gs; }

	void set_rect(const sf::IntRect &rect);
	constexpr sf::IntRect get_rect() const { return rect; }

	void set_backwards(const bool b);
	constexpr bool get_backwards() const { return backwards; }

	void update(std::span<const float> spectrum);
	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override;

#ifdef LIBAVZ_IMGUI
	void imgui();
#endif

private:
	int get_bar_vertex_index(int bar_idx, int vertex_num) const;
	void update_bars();
};

} // namespace avz
