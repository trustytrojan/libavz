#include <avz/gfx/ColorSettings.hpp>
#include <avz/gfx/util.hpp>

#ifdef LIBAVZ_IMGUI
#include <imgui.h>
#endif

namespace avz
{

sf::Color ColorSettings::calculate_color(const float index_ratio) const
{
	switch (mode)
	{
	case Mode::WHEEL:
	{
		const auto [h, s, v] = wheel.hsv;
		return util::hsv2rgb(index_ratio + h + wheel.time, s, v);
	}

	case Mode::WHEEL_RANGES:
	{
		const auto [h, s, v] = util::interpolate(index_ratio + wheel.time, wheel.start_hsv, wheel.end_hsv);
		return util::hsv2rgb(h, s, v);
	}

	case Mode::WHEEL_RANGES_REVERSE:
	{
		const auto [h, s, v] = util::interpolate_and_reverse(index_ratio + wheel.time, wheel.start_hsv, wheel.end_hsv);
		return util::hsv2rgb(h, s, v);
	}

	case Mode::SOLID:
		return solid;

	default:
		throw std::logic_error{"[ColorSettings::calculate_color] default case hit"};
	}
}

} // namespace avz

#ifdef LIBAVZ_IMGUI
void avz::ColorSettings::imgui()
{
	if (ImGui::CollapsingHeader("Colors"))
	{
		const char *modes[] = {"WHEEL", "WHEEL_RANGES", "WHEEL_RANGES_REVERSE", "SOLID"};
		int cm = static_cast<int>(mode);
		if (ImGui::Combo("Color mode", &cm, modes, IM_ARRAYSIZE(modes)))
			set_mode(static_cast<Mode>(cm));

		// Use the combo's index for immediate UI reaction (avoids a one-frame mismatch)
		switch (static_cast<Mode>(cm))
		{
		case Mode::SOLID:
		{
			float col[3] = {solid.r / 255.f, solid.g / 255.f, solid.b / 255.f};
			if (ImGui::ColorEdit3("Solid color", col))
				set_solid_color(
					sf::Color(
						static_cast<unsigned char>(col[0] * 255.f),
						static_cast<unsigned char>(col[1] * 255.f),
						static_cast<unsigned char>(col[2] * 255.f)));
			break;
		}

		case Mode::WHEEL:
		{
			ImGui::SliderFloat("Wheel rate", &wheel.rate, 0.f, 1.f);
			ImGui::SliderFloat3("Wheel HSV", &wheel.hsv.x, 0.f, 1.f);
			break;
		}

		case Mode::WHEEL_RANGES:
		case Mode::WHEEL_RANGES_REVERSE:
		{
			ImGui::SliderFloat("Wheel rate", &wheel.rate, 0.f, 1.f);
			ImGui::SliderFloat3("Wheel start HSV", &wheel.start_hsv.x, 0.f, 1.f);
			ImGui::SliderFloat3("Wheel end HSV", &wheel.end_hsv.x, 0.f, 1.f);
			break;
		}

		default:
			break;
		}
	}
}
#endif
