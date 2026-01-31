#pragma once

#include <SFML/Graphics.hpp>
#include <avz/gfx/ColorSettings.hpp>
#include <avz/gfx/util.hpp>
#include <span>

namespace avz
{

/**
 * A customizable frequency spectrum visualizer with efficient rendering options.
 */
class SpectrumDrawable : public sf::Drawable
{
	const ColorSettings &color;
	float multiplier{1};
	sf::VertexArray vertex_array;
	sf::IntRect rect;
	bool backwards{}, debug_rect{}, use_gs{};

	struct
	{
		int width{10}, spacing{5}, count{};
	} bar;

#ifdef LIBAVZ_IMGUI
	std::string imgui_header{std::format("SpectrumDrawable @ {}", (void *)this)};
#endif

public:
	SpectrumDrawable(const ColorSettings &color);
	SpectrumDrawable(const sf::IntRect &rect, const ColorSettings &color);

	void set_bar_width(const int width);
	void set_bar_spacing(const int spacing);
	constexpr int get_bar_width() const { return bar.width; }
	constexpr int get_bar_spacing() const { return bar.spacing; }

	/**
	 * Get the number of bars to be rendered.
	 * This is computed from the bar width/spacing and the rect's width.
	 */
	constexpr int get_bar_count() const { return bar.count; }

	/**
	 * Set whether a white rectangle/circle showing the bounding box & origin
	 * of the object is shown. A red version of the same shapes is shown
	 * without the `RenderStates` passed to `draw()`.
	 */
	constexpr void set_debug_rect(bool b) { debug_rect = b; }
	constexpr bool get_debug_rect() const { return debug_rect; }

	/**
	 * Set the amount to scale the bars' heights by.
	 */
	constexpr void set_multiplier(const float m) { multiplier = m; }
	constexpr float get_multiplier() const { return multiplier; }

	/**
	 * Enable the geometry shader for this `SpectrumDrawable`, saving CPU time and GPU bandwidth
	 * when there are many bars to render.
	 * NOTE: This is also `necessary` to enable if you pair this spectrum with a `Polar` effect
	 * with its `spectrum_gs` feature enabled (using `*this` as its argument).
	 */
	void set_use_gs(bool b);

	/**
	 * Return whether this spectrum is rendering with a geometry shader.
	 */
	constexpr bool get_use_gs() const { return use_gs; }

	/**
	 * Set the bounding box for this spectrum.
	 */
	void set_rect(const sf::IntRect &rect);
	constexpr sf::IntRect get_rect() const { return rect; }

	/**
	 * Set whether the bars are ordered in reverse.
	 */
	void set_backwards(const bool b);
	constexpr bool get_backwards() const { return backwards; }

	/**
	 * Update the spectrum bars' heights with the values in the passed `spectrum` span.
	 * REQUIRES that `spectrum.size()` is equal to the number of bars to be rendered!
	 */
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
