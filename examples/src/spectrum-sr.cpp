#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

#ifdef LIBAVZ_IMGUI
#include <imgui.h>
#endif

using namespace avz::examples;

struct SpectrumResamplerExample : ExampleBase
{
	// audio, spectrum
	std::vector<float> a, s;

	avz::ColorSettings color;
	avz::SpectrumDrawable spectrum;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;

	// SpectrumResampler combines logarithmic scaling with interpolation in one smooth operation
	avz::SpectrumResampler sr;
	avz::Interpolator ip;

	SpectrumResamplerExample(const Args &args)
		: ExampleBase{args},
		  fa{args.get_audio_duration_sec() * sample_rate_hz},
		  spectrum{{{}, (sf::Vector2i)size}, color}
	{
		spectrum.set_bar_width(1);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(4);
		emplace_layer<avz::Layer>("spectrum").add_draw({spectrum});

		// Configure logarithmic frequency scaling from 20 Hz to 8 kHz
		sr.set_scale(avz::SpectrumResampler::Scale::LOG);
	}

	/**
	 * IMPORTANT: We override this method for consumers of our visualizer
	 * to know how many audio frames they should prepare for us.
	 */
	int get_audio_frames_needed() override { return fa.get_fft_size(); }

	/**
	 * This is REQUIRED to implement. Here we should update all of our objects using the new audio.
	 */
	void update(std::span<const float> audio_buffer) override
	{
		// make sure we can fit one channel of audio
		a.resize(fa.get_fft_size());

		// extract first channel of audio and perform FFT (needed in case media file has stereo audio)
		capture_time("extract_channel", avz::util::extract_channel(a, audio_buffer, num_channels, 0));
		capture_time("fft", aa.execute_fft(fa, a));
		capture_time("amplitudes", aa.compute_amplitudes(fa));

		// make sure we can fit all the spectrum bars
		s.resize(spectrum.get_bar_count());

		// Resample FFT amplitudes with logarithmic scaling and interpolation in one smooth operation!
		// No gaps, no separate interpolation step needed - just beautiful curves
		capture_time(
			"resample_spectrum", sr.resample_spectrum(s, aa.get_amplitudes(), sample_rate_hz, fa.get_fft_size(), ip));

		// finally, pass the data to SpectrumDrawable to draw to the screen!
		capture_time("spectrum_update", spectrum.update(s));

		// ensure the color wheel moves with time, if configured to do so.
		color.increment_wheel_time();

#ifdef LIBAVZ_IMGUI
		// run imgui at the END so that we don't break assertions mid-update.
		ImGui::Begin("SpectrumResamplerExample");
		spectrum.imgui();
		sr.imgui();
		ip.imgui();
		fa.imgui();
		color.imgui();
		ImGui::End();
#endif
	}
};

LIBAVZ_EXAMPLE_MAIN(
	SpectrumResamplerExample,
	"Spectrum visualization with logarithmic frequency scaling and customizable frequency range",
	0.1f)
