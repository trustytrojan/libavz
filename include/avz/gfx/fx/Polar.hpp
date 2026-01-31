#pragma once

#include "avz/gfx/SpectrumDrawable.hpp"
#include <SFML/Graphics.hpp>
#include <avz/gfx/fx/TransformEffect.hpp>

namespace avz::fx
{

struct Polar : TransformEffect
{
	sf::Vector2f size;
	float base_radius, max_radius;
	float angle_start;
	float angle_span;
	float warping_factor;

	Polar(sf::Vector2f size, float br, float mr, float angle_start, float angle_span, float warping_factor = 1.0f);

	virtual const sf::Shader &getShader() const override;
	virtual void setShaderUniforms() const override;

	/**
	 * If this `Polar` effect is paired with a `SpectrumDrawable` setup to use its geometry shader,
	 * pass a reference to it here and this object's `getShader()` will return a geometry shader
	 * specifically for emitting the spectrum bars' vertices in polar coordinates.
	 */
	constexpr void enable_spectrum_gs(const SpectrumDrawable &spectrum)
	{
		spectrum_gs = true;
		spectrum_bar_width = spectrum.get_bar_width();
		const auto rect = spectrum.get_rect();
		spectrum_bottom_y = rect.position.y + rect.size.y;
	}

	/**
	 * Disable the spectrum geometry shader and use the normal vertex shader instead.
	 */
	constexpr void disable_spectrum_gs() { spectrum_gs = {}; }

private:
	bool spectrum_gs{};
	float spectrum_bar_width;
	float spectrum_bottom_y;
};

} // namespace avz::fx
