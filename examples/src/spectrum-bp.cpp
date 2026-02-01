#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

#ifdef LIBAVZ_IMGUI
#include <imgui.h>
#endif

using namespace avz::examples;

/*
Frequency spectrum visualizer example. Update logic/flow:

1. read `fft_size` interleaved samples from the media source.
   this is communicated via our override of the `get_audio_samples_needed()` method.
2. extract the first channel of audio from the received chunk
3. perform FFT on the extracted audio
4. compute amplitudes from the FFT output
5. use `BinPacker` to "pack" FFT amplitudes into (typically logarithmically) scaled
   "bins" which correspond to the spectrum visualizer's bars
6. use `Interpolator` to interpolate the gaps left behind by non-linear scaling
7. finally, pass the data to our `SpectrumDrawable` for it to render

This is the preferred (and faster) way to represent the full range of frequencies from the input audio.
See `spectrum-sr.cpp` for a method that allows a customizable frequency range.
*/
struct SpectrumExample : ExampleBase
{
	// audio, spectrum
	std::vector<float> a, s;

	avz::ColorSettings color;
	avz::SpectrumDrawable spectrum;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;

	// needed to logarithmically pack fft_size spectral samples into the spectrum's bars
	avz::BinPacker bp;

	// needed to interpolate the gaps left by BinPacker's spreading out of bins
	avz::Interpolator ip;

	SpectrumExample(const Args &args)
		: ExampleBase{args},
		  fa{args.get_audio_duration_sec() * sample_rate_hz},
		  spectrum{{{}, (sf::Vector2i)size}, color}
	{
		spectrum.set_bar_width(1);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(4);
		emplace_layer<avz::Layer>("spectrum").add_draw({spectrum});

		// logarithmically scale bin indices (frequencies)
		bp.set_scale(avz::BinPacker::Scale::LOG);

		// when multiple values go to a bin, accumulate them using std::max()
		// for fun, change this to SUM and see what happens
		bp.set_accum_method(avz::BinPacker::AccumulationMethod::MAX);
	}

	/**
	 * IMPORTANT: We override this method for consumers of our visualizer
	 * to know how many audio frames they should prepare for us.
	 */
	int get_audio_frames_needed() override { return fa.get_fft_size(); }

	/**
	 * This is REQUIRED to implement. We have to update all of our objects using the new audio.
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
		// it's not necessary here, but assign all zero in case the spectrum bar count changes dynamically
		s.assign(spectrum.get_bar_count(), 0);

		// pack FFT amplitudes into a smaller set of "bins" (our spectrum bars!)
		capture_time("bin_pack", bp.bin_pack(s, aa.get_amplitudes()));

		// there will be gaps caused by BinPacker because we logarithmically spread the output bins
		// interpolate between them to make a nice looking curve
		capture_time("interpolate", ip.interpolate(s));

		// finally, pass the data to SpectrumDrawable to draw to the screen!
		capture_time("spectrum_update", spectrum.update(s));

		// ensure the color wheel moves with time, if configured to do so.
		color.increment_wheel_time();

#ifdef LIBAVZ_IMGUI
		// run imgui at the END so that we don't break assertions mid-update.
		ImGui::Begin("SpectrumExample");
		spectrum.imgui();
		bp.imgui();
		ip.imgui();
		fa.imgui();
		color.imgui();
		ImGui::End();
#endif
	}
};

LIBAVZ_EXAMPLE_MAIN(SpectrumExample, "Spectrum visualization with logarithmic frequency scaling", 0.1f)
