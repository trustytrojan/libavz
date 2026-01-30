#include "BassNationUtils.hpp"
#include <cmath>

namespace avz::examples
{

float bass_nation_additional_displacement(std::span<const float> bass_amps)
{
	// Constants for the "Nation" look
	constexpr auto mid_point = 0.3f;	// Full weight at 53Hz
	constexpr auto k_rise = 0.08f;		// Smooth, quick rise from 20Hz
	constexpr auto k_fall = 0.08f;		// Gentle fall toward 135Hz
	constexpr auto threshold = 1.0f;	// Your "Kick" detection threshold
	constexpr auto boost_factor = 3.0f; // How much the "Bonus" multiplies

	float max{};

	for (size_t i = 0; i < bass_amps.size(); ++i)
	{
		const auto x = (float)i / bass_amps.size();

		// 1. Optimized Asymmetric Weighting
		const auto dist = x - mid_point;
		const auto k = (dist < 0) ? k_rise : k_fall;
		const auto freq_weight = std::exp(-(dist * dist) / k);

		// 2. Apply weight to raw amplitude
		auto val = bass_amps[i] * freq_weight;

		// 3. The "Kick Bonus" (Exponential Expansion)
		// Example: if val is 1.2, the 'extra' 0.2 is taken out, squared, then added back to val.
		if (val > threshold)
		{
			const auto extra = val - threshold;
			val = threshold + (extra * extra * boost_factor);
		}

		// Dividing by 5, for some reason, makes it look perfect.
		val /= 5;

		// Keep finding the max.
		if (val > max)
			max = val;
	}

	return max;
}

} // namespace avz::examples
