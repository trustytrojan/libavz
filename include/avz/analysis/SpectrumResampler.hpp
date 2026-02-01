#pragma once

#include <avz/analysis/Interpolator.hpp>
#include <cmath>
#include <format>
#include <span>
#include <string>
#include <vector>


namespace avz
{

/**
 * Resamples frequency spectrum data with configurable frequency scaling (linear, logarithmic, nth-root).
 * Maintains state for efficient recomputation when parameters change infrequently.
 * 
 * This class provides smooth, gap-free spectrum resampling by combining interpolation with
 * arbitrary frequency scale mappings. Unlike BinPacker which creates discrete bins with gaps,
 * SpectrumResampler produces uniform, smooth curves by sampling the interpolated spectrum
 * at precisely calculated frequency positions.
 */
class SpectrumResampler
{
public:
	enum class Scale
	{
		LINEAR,   ///< Linear frequency spacing
		LOG,      ///< Logarithmic frequency spacing (perceptually uniform)
		NTH_ROOT  ///< Nth-root frequency spacing (configurable via set_nth_root)
	};

private:
	// Configuration
	Scale scale{Scale::LOG};
	int start_freq{20};
	int end_freq{20000};
	int nth_root{2};
	float nthroot_inv{1.f / nth_root};

	// Cached frequency scale bounds to avoid recomputation
	struct _freq_scale_cache
	{
		float linear_start, linear_end;
		float log_start, log_end;
		float root_start, root_end;
		void calc(int start_hz, int end_hz, float nthroot_inv);
	} freq_cache;

	// Cached state for efficiency
	int cached_sample_rate{-1};
	int cached_fft_size{-1};
	int cached_output_size{-1};
	std::vector<float> bin_positions;  ///< Pre-calculated bin positions for current config

#ifdef LIBAVZ_IMGUI
	std::string imgui_header{std::format("SpectrumResampler @ {}", (void *)this)};
#endif

public:
	SpectrumResampler() = default;

	/**
	 * Set the frequency scale type.
	 * @param scale new scale to use
	 */
	void set_scale(Scale scale);

	/**
	 * Set the frequency range to resample.
	 * @param start_hz start frequency in Hz
	 * @param end_hz end frequency in Hz
	 */
	void set_frequency_range(int start_hz, int end_hz);

	/**
	 * Set the nth-root to use when using the NTH_ROOT scale.
	 * @param nth_root new nth_root to use (must be non-zero)
	 * @throws std::invalid_argument if nth_root is zero
	 */
	void set_nth_root(int nth_root);

	/**
	 * Get the current scale type.
	 */
	constexpr Scale get_scale() const { return scale; }

	/**
	 * Get the start frequency in Hz.
	 */
	constexpr int get_start_freq() const { return start_freq; }

	/**
	 * Get the end frequency in Hz.
	 */
	constexpr int get_end_freq() const { return end_freq; }

	/**
	 * Get the current nth root value.
	 */
	constexpr int get_nth_root() const { return nth_root; }

	/**
	 * Resample spectrum from input FFT amplitudes using configured scale and frequency range.
	 * Produces smooth, gap-free output by interpolating between input bins at precisely
	 * calculated frequency positions according to the selected scale.
	 * 
	 * @param spectrum output spectrum buffer (determines output size)
	 * @param in_amps input FFT amplitudes (from AudioAnalyzer)
	 * @param sample_rate_hz sample rate in Hz
	 * @param fft_size FFT size used to produce in_amps
	 * @param interpolator interpolator to use for sampling between bins
	 */
	void resample_spectrum(
		std::span<float> spectrum,
		std::span<const float> in_amps,
		int sample_rate_hz,
		int fft_size,
		Interpolator &interpolator);

#ifdef LIBAVZ_IMGUI
	/**
	 * Render ImGui controls for spectrum resampler properties.
	 */
	void imgui();
#endif

private:
	/**
	 * Compute bin positions for the current configuration and cache them.
	 */
	void compute_bin_positions(size_t output_size, int sample_rate_hz, int fft_size);

	/**
	 * Convert a frequency ratio (0.0 to 1.0) to an actual frequency according to the scale.
	 */
	float ratio_to_freq(float ratio) const;
};

} // namespace avz
