#pragma once

#include <span>

namespace avz::examples
{

/**
 * Calculates the additional particle displacement using the bass frequency spectrum passed
 * as `bass_amps`. It is expected to contain a 20-135Hz frequency range, straight from FFT
 * amplitude output.
 */
float bass_nation_additional_displacement(std::span<const float> bass_amps);

} // namespace avz::examples
