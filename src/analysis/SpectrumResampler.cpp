#include <algorithm>
#include <avz/analysis/SpectrumResampler.hpp>
#include <cassert>
#include <cmath>
#include <stdexcept>

#ifdef LIBAVZ_IMGUI
#include <imgui.h>
#endif

namespace avz
{

void SpectrumResampler::_freq_scale_cache::calc(const int start_hz, const int end_hz, const float nthroot_inv)
{
	linear_start = (float)start_hz;
	linear_end = (float)end_hz;

	log_start = std::log((float)start_hz);
	log_end = std::log((float)end_hz);

	root_start = std::pow((float)start_hz, nthroot_inv);
	root_end = std::pow((float)end_hz, nthroot_inv);
}

void SpectrumResampler::set_scale(const Scale scale)
{
	this->scale = scale;
	// Invalidate cache to force recomputation
	cached_output_size = -1;
}

void SpectrumResampler::set_frequency_range(const int start_hz, const int end_hz)
{
	if (start_hz <= 0 || end_hz <= start_hz)
		throw std::invalid_argument{"[SpectrumResampler::set_frequency_range] Invalid frequency range"};

	start_freq = start_hz;
	end_freq = end_hz;
	freq_cache.calc(start_hz, end_hz, nthroot_inv);
	// Invalidate cache to force recomputation
	cached_output_size = -1;
}

void SpectrumResampler::set_nth_root(const int nth_root)
{
	if (nth_root == 0)
		throw std::invalid_argument{"[SpectrumResampler::set_nth_root] nth_root cannot be zero"};

	this->nth_root = nth_root;
	nthroot_inv = 1.0f / nth_root;
	freq_cache.calc(start_freq, end_freq, nthroot_inv);
	// Invalidate cache to force recomputation
	cached_output_size = -1;
}

void SpectrumResampler::resample_spectrum(
	std::span<float> spectrum,
	std::span<const float> in_amps,
	const int sample_rate_hz,
	const int fft_size,
	Interpolator &interpolator)
{
	assert(!spectrum.empty() && "Output spectrum must not be empty");
	assert(!in_amps.empty() && "Input amplitudes must not be empty");

	const auto output_size = spectrum.size();

	// Recompute bin positions if configuration has changed
	if (cached_sample_rate != sample_rate_hz || cached_fft_size != fft_size || cached_output_size != (int)output_size)
		compute_bin_positions(output_size, sample_rate_hz, fft_size);

	// Set up the interpolator with input data
	interpolator.set_values(in_amps);

	// Sample the interpolated spectrum at the pre-calculated bin positions
#pragma GCC ivdep
	for (size_t i = 0; i < output_size; ++i)
		spectrum[i] = interpolator.sample(bin_positions[i]);
}

void SpectrumResampler::compute_bin_positions(const size_t output_size, const int sample_rate_hz, const int fft_size)
{
	bin_positions.resize(output_size);

	const float bin_size = (float)sample_rate_hz / fft_size;
	const float max_i = std::max(1.0f, (float)output_size - 1.0f);

	for (size_t i = 0; i < output_size; ++i)
	{
		// Calculate frequency for this output index according to the scale
		const float ratio = (float)i / max_i;
		const float freq = ratio_to_freq(ratio);

		// Convert frequency to bin position
		bin_positions[i] = freq / bin_size;
	}

	// Update cache
	cached_sample_rate = sample_rate_hz;
	cached_fft_size = fft_size;
	cached_output_size = output_size;
}

float SpectrumResampler::ratio_to_freq(const float ratio) const
{
	switch (scale)
	{
	case Scale::LINEAR:
		// Linear interpolation between start and end frequency
		return freq_cache.linear_start + ratio * (freq_cache.linear_end - freq_cache.linear_start);

	case Scale::LOG:
		// Logarithmic: freq = start * (end/start)^ratio
		// This gives perceptually uniform spacing
		return std::exp(freq_cache.log_start + ratio * (freq_cache.log_end - freq_cache.log_start));

	case Scale::NTH_ROOT:
		// Nth-root spacing: interpolate in root-space, then raise to power
		// This is between linear (n=1) and log (n→∞)
		{
			const float root_freq = freq_cache.root_start + ratio * (freq_cache.root_end - freq_cache.root_start);
			return std::pow(root_freq, (float)nth_root);
		}

	default:
		throw std::logic_error{"[SpectrumResampler::ratio_to_freq] Invalid scale type"};
	}
}

#ifdef LIBAVZ_IMGUI
void SpectrumResampler::imgui()
{
	if (!ImGui::CollapsingHeader(imgui_header.c_str()))
		return;

	const char *scales[] = {"Linear", "Log", "Nth root"};
	int scale_idx = static_cast<int>(scale);
	if (ImGui::Combo("Scale", &scale_idx, scales, IM_ARRAYSIZE(scales)))
		set_scale(static_cast<Scale>(scale_idx));

	if (scale == Scale::NTH_ROOT)
	{
		int nth = nth_root;
		if (ImGui::SliderInt("Nth root", &nth, 2, 16))
			set_nth_root(nth);
	}

	int freq_range[2] = {start_freq, end_freq};
	if (ImGui::SliderInt2("Freq Range (Hz)", freq_range, 1, 24000))
	{
		try
		{
			set_frequency_range(freq_range[0], freq_range[1]);
		}
		catch (const std::invalid_argument &)
		{
			// Ignore invalid ranges during drag
		}
	}

	// Show cached state info
	ImGui::Separator();
	ImGui::Text("Cache: SR=%d, FFT=%d, Out=%d", cached_sample_rate, cached_fft_size, cached_output_size);
}
#endif

} // namespace avz
