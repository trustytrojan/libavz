#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

using namespace avz::examples;

struct ParticleSystemExampleArgs : avz::examples::Args
{
	ParticleSystemExampleArgs(int argc, const char *const *argv, float default_audio_duration = 0.25f)
		: Args{argc, argv, default_audio_duration, false}
	{
		// clang-format off
		add_argument("--particle-count")
			.help("Number of particles")
			.default_value(50u)
			.scan<'u', unsigned>();

		add_argument("--point-count")
			.help("Number of points per particle circle")
			.default_value(10u)
			.scan<'u', unsigned>();

		add_argument("--radius-min")
			.help("Minimum particle radius")
			.default_value(1.f)
			.scan<'f', float>();

		add_argument("--radius-max")
			.help("Maximum particle radius")
			.default_value(5.f)
			.scan<'f', float>();

		add_argument("--velocity-x-min")
			.help("Minimum X velocity")
			.default_value(-1.f)
			.scan<'f', float>();

		add_argument("--velocity-x-max")
			.help("Maximum X velocity")
			.default_value(1.f)
			.scan<'f', float>();

		add_argument("--velocity-y-min")
			.help("Minimum Y velocity")
			.default_value(-1.f)
			.scan<'f', float>();

		add_argument("--velocity-y-max")
			.help("Maximum Y velocity")
			.default_value(0.f)
			.scan<'f', float>();
		// clang-format on

		parse(argc, argv);
	}
};

struct ParticleSystemExample : ExampleBase
{
	const int fft_size;

	// calculate the FFT amplitudes array indices of min/max frequencies in Hz
	const int min_fft_index = avz::util::bin_index_from_freq(0, sample_rate_hz, fft_size);
	const int max_fft_index = avz::util::bin_index_from_freq(250, sample_rate_hz, fft_size);

	std::vector<float> a, p;

	avz::ParticleSystem ps;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;

	ParticleSystemExample(const ParticleSystemExampleArgs &args)
		: ExampleBase{args},
		  fft_size{static_cast<int>(args.get_audio_duration_sec() * sample_rate_hz)},
		  ps{{{}, (sf::Vector2i)size}, args.get<unsigned>("--particle-count"), (int)args.get_framerate()},
		  fa{fft_size}
	{
		ps.set_particle_point_count(args.get<unsigned>("--point-count"));
		ps.set_particle_radius_range(args.get<float>("--radius-min"), args.get<float>("--radius-max"));
		ps.set_particle_velocity_x_range(args.get<float>("--velocity-x-min"), args.get<float>("--velocity-x-max"));
		ps.set_particle_velocity_y_range(args.get<float>("--velocity-y-min"), args.get<float>("--velocity-y-max"));
		emplace_layer<avz::Layer>("particles").add_draw({ps});
	}

	int get_audio_frames_needed() override { return fa.get_fft_size(); }

	void update(std::span<const float> audio_buffer) override
	{
		// make sure we can fit one channel of audio
		a.resize(fft_size);

		// extract first channel of audio and perform FFT (needed in case media file has stereo audio)
		capture_time("extract_channel", avz::util::extract_channel(a, audio_buffer, num_channels, 0));
		capture_time("fft", aa.execute_fft(fa, a));

		// compute amplitudes from FFT output
		capture_time("amplitudes", aa.compute_amplitudes(fa));

		// compute FFT amplitudes, take only the bass frequencies
		const auto amps = aa.get_amplitudes().subspan(min_fft_index, max_fft_index - min_fft_index);
		p.resize(amps.size());

		// weight each element by its normalized position through a Gompertz function
		// this makes lower bass frequencies boost the particles more than higher bass frequencies
		for (size_t i = 0; i < amps.size(); ++i)
		{
			const auto normalized_pos = (float)i / amps.size();
			p[i] = amps[i] * expf(-expf(4.5 * normalized_pos - 4)) / 5;
		}

		float max;
		capture_time("max", max = std::ranges::max(p));

		// pass the max as `additional_displacement` to ParticleSystem::update.
		// the displacement is then scaled to the window's height, and dampened
		// with sqrt (otherwise the particles just go crazy).
		capture_time("ps_update", ps.update(max));
	}
};

int main(int argc, const char *const *argv)
{
	ParticleSystemExampleArgs args{argc, argv, 0.25f};
	ParticleSystemExample viz{args};
	int audio_frames = viz.fft_size;
	avz::Player{viz, viz.media, (int)args.get_framerate()}.start_in_window("ParticleSystemExample");
	return EXIT_SUCCESS;
}
