#include "BassNationSpectrumLayer.hpp"
#include "BassNationUtils.hpp"
#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>
#include <future>
#include <memory>

using namespace avz::examples;

struct OldBassNation : ExampleBase
{
	const int fft_size;

	std::vector<std::unique_ptr<BassNationSpectrumLayer>> spectrums;
	avz::ColorSettings cs;
	std::vector<std::future<void>> futures;

	// calculate the FFT amplitudes array indices of min/max frequencies in Hz
	const int min_fft_index = avz::util::bin_index_from_freq(20, sample_rate_hz, fft_size);
	const int max_fft_index = avz::util::bin_index_from_freq(135, sample_rate_hz, fft_size);
	std::vector<float> a, p;
	avz::ParticleSystem ps;
	avz::FrequencyAnalyzer fa{fft_size};
	avz::AudioAnalyzer aa;
	avz::fx::Polar ps_polar{
		(sf::Vector2f)size, // Dimensions of linear space
		size.y * 0.25f,		// Base radius inner hole: 25% screen height
		size.x * 0.6f,		// Max radius: 60% screen width, so that you don't see any particles disappear
		M_PI / 2,			// Angle start
		2 * M_PI,			// Angle span
		0.0f				// warping_factor: no warping for particles
	};

	avz::fx::Polar polar_left;
	avz::fx::Polar polar_right;

	OldBassNation(const Args &args)
		: ExampleBase{args},
		  fft_size{static_cast<int>(args.get_audio_duration_sec() * sample_rate_hz)},
		  ps{{{}, (sf::Vector2i)size}, 50, (int)args.get_framerate()},
		  polar_left{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, M_PI / 2, M_PI},
		  polar_right{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, -M_PI / 2, M_PI}
	{
		ps.set_fade_out(false);
		ps.set_start_offscreen(false);
		ps.set_particle_velocity_x_range(-0.5f, 0.5f);
		auto &particles_layer = emplace_layer<avz::PostProcessLayer>("particles", size);
		particles_layer.add_draw({ps, &ps_polar});

		cs.set_mode(avz::ColorSettings::Mode::SOLID);

		static const std::array<sf::Color, 9> colors{
			sf::Color::Green,
			sf::Color::Cyan,
			sf::Color::Blue,
			sf::Color{146, 29, 255}, // purple
			sf::Color::Magenta,
			sf::Color::Red,
			sf::Color{255, 165, 0}, // orange
			sf::Color::Yellow,
			sf::Color::White //
		};

		const auto delta_duration = 0.015f;
		const auto max_duration_diff = (colors.size() - 1) * delta_duration;

		auto &spectrum_layer = emplace_layer<avz::Layer>("spectrum");

		for (int i = 0; i < colors.size(); ++i)
		{
			float duration_diff = max_duration_diff - i * delta_duration;
			const auto new_duration_sec = args.get_audio_duration_sec() - duration_diff;
			const int new_fft_size = new_duration_sec * sample_rate_hz;

			cs.set_solid_color(colors[i]);

			auto &left_layer = *spectrums.emplace_back(
				std::make_unique<BassNationSpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, true));
			left_layer.configure_spectrum(false, size);
			left_layer.spectrum.set_use_gs(args.get_geometry_shader_enabled());

			auto &right_layer = *spectrums.emplace_back(
				std::make_unique<BassNationSpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, false));
			right_layer.configure_spectrum(true, size);
			right_layer.spectrum.set_use_gs(args.get_geometry_shader_enabled());

			spectrum_layer.add_draw({left_layer.spectrum, &polar_left});
			spectrum_layer.add_draw({right_layer.spectrum, &polar_right});
		}

		// Enable GS spectrum-bar expansion on the polar effect
		if (args.get_geometry_shader_enabled())
		{
			// we can use any of them, since they all have the same bar_width and bottom_y
			polar_left.set_gs_spectrum_bars(&spectrums[0]->spectrum);
			polar_right.set_gs_spectrum_bars(&spectrums[0]->spectrum);
		}

		futures.resize(spectrums.size());
	}

	void update(std::span<const float> audio_buffer) override
	{
		std::ranges::transform(
			spectrums, futures.begin(), std::bind_back(&BassNationSpectrumLayer::trigger_work, audio_buffer));

		// while the spectrums are updating, update our particle system
		{
			// make sure we can fit one channel of audio
			a.resize(fft_size);

			// extract first channel of audio and perform FFT (needed in case media file has stereo audio)
			capture_time("extract_channel", avz::util::extract_channel(a, audio_buffer, num_channels, 0));
			capture_time("fft", aa.execute_fft(fa, a));

			// compute amplitudes from FFT output
			capture_time("amplitudes", aa.compute_amplitudes(fa));

			// compute FFT amplitudes, take only the bass frequencies
			const auto bass_amps = aa.get_amplitudes().subspan(min_fft_index, max_fft_index - min_fft_index);
			p.resize(bass_amps.size());

			// calculate and pass additional particle displacement to the ParticleSystem's update() method
			float max;
			capture_time("bn_calc", max = bass_nation_additional_displacement(bass_amps));
			capture_time("ps_update", ps.update(bass_nation_additional_displacement(bass_amps)));
		}

		// wait for all compute tasks
		std::ranges::for_each(futures, &std::future<void>::wait);
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	OldBassNation, "Multi-spectrum polar visualization with bass frequencies (old version)", 0.25f, viz.fft_size)
