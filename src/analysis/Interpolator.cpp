#include <avz/analysis/Interpolator.hpp>
#include <iostream>

#ifdef LIBAVZ_IMGUI
#include <imgui.h>
#endif

namespace avz
{

void Interpolator::interpolate(std::span<float> range)
{
	const auto size = range.size();

	// separate the nonzero values (y's) and their indices (x's)
	m_spline_x.clear();
	m_spline_y.clear();
	zero_indices.clear();
	for (size_t i = 0; i < size; ++i)
	{
		if (!range[i])
		{
			zero_indices.emplace_back(i);
			continue;
		}
		m_spline_x.emplace_back(i);
		m_spline_y.emplace_back(range[i]);
	}

	// we can ignore this problem in release builds as tk-spline's assertion will be gone
#ifndef NDEBUG
	if (m_spline_x.size() < 3)
	{
		std::cerr << "[Interpolator::interpolate] < 3 points provided, skipping interpolation\n";
		return;
	}
#endif

	spline.set_points(m_spline_x, m_spline_y, (tk::spline::spline_type)type);

	// fill in the gaps
	for (const auto zero_index : zero_indices)
		range[zero_index] = spline(zero_index);
}

void Interpolator::set_values(std::span<const float> values)
{
	m_spline_x.clear();
	m_spline_y.clear();
	for (size_t i = 0; i < values.size(); ++i)
	{
		m_spline_x.emplace_back(i);
		m_spline_y.emplace_back(values[i]);
	}

	// we can ignore this problem in release builds as tk-spline's assertion will be gone
#ifndef NDEBUG
	if (m_spline_x.size() < 3)
	{
		std::cerr << "[Interpolator::set_values] < 3 points provided, skipping interpolation\n";
		return;
	}
#endif

	spline.set_points(m_spline_x, m_spline_y, (tk::spline::spline_type)type);
}

} // namespace avz

#ifdef LIBAVZ_IMGUI
void avz::Interpolator::imgui()
{
	if (ImGui::CollapsingHeader("Interpolator"))
	{
		const char *modes[] = {"Linear", "Cubic (cspline)", "Cubic (hermite)"};
		int it = get_interp_type_index();
		if (ImGui::Combo("Type", &it, modes, IM_ARRAYSIZE(modes)))
			set_interp_type_index(it);
	}
}
#endif
