#pragma once

#include <format>
#include <span>
#include <string>
#include <tk-spline.hpp>
#include <vector>


namespace avz
{

/**
 * Handles interpolation of frequency spectrum data using various methods.
 */
class Interpolator
{
public:
	enum class InterpolationType
	{
		LINEAR = tk::spline::linear,
		CSPLINE = tk::spline::cspline,
		CSPLINE_HERMITE = tk::spline::cspline_hermite
	};

private:
	tk::spline spline;
	std::vector<double> m_spline_x, m_spline_y;
	std::vector<size_t> zero_indices;
	InterpolationType type{InterpolationType::CSPLINE};

#ifdef LIBAVZ_IMGUI
	std::string imgui_header{std::format("Interpolator @ {}", (void *)this)};
#endif

public:
	/**
	 * Set the interpolation type to use.
	 * @param type interpolation type
	 */
	void set_interp_type(InterpolationType type) { this->type = type; }

	/**
	 * Get the current interpolation type.
	 */
	InterpolationType get_interp_type() const { return type; }

	/**
	 * Helpers to map UI indices (0..N-1) to enum values and back.
	 * The underlying tk::spline values are not guaranteed to be consecutive,
	 * so the UI must use a compact 0-based index space.
	 */
	constexpr static int interp_type_to_index(InterpolationType t)
	{
		switch (t)
		{
		case InterpolationType::LINEAR:
			return 0;
		case InterpolationType::CSPLINE:
			return 1;
		case InterpolationType::CSPLINE_HERMITE:
			return 2;
		default:
			return 1;
		}
	}

	constexpr static InterpolationType interp_type_from_index(int idx)
	{
		switch (idx)
		{
		case 0:
			return InterpolationType::LINEAR;
		case 1:
			return InterpolationType::CSPLINE;
		default:
			return InterpolationType::CSPLINE_HERMITE;
		}
	}

	constexpr int get_interp_type_index() const { return interp_type_to_index(type); }
	constexpr void set_interp_type_index(int idx) { set_interp_type(interp_type_from_index(idx)); }

	/**
	 * Interpolate spectrum data to fill gaps between non-zero values.
	 * Uses the interpolation type set via set_interp_type().
	 * @param spectrum spectrum data to interpolate in-place
	 */
	void interpolate(std::span<float> spectrum);

	/**
	 * Set the values for the spline interpolation.
	 * The X values are assumed to be 0, 1, 2, ...
	 * @param values Y values
	 */
	void set_values(std::span<const float> values);

	/**
	 * Sample the spline at a given X coordinate.
	 * @param x X coordinate (index)
	 * @return interpolated value
	 */
	constexpr float sample(float x) const { return spline(x); }

#ifdef LIBAVZ_IMGUI
	/**
	 * Render ImGui controls for interpolation type.
	 */
	void imgui();
#endif
};

} // namespace avz
