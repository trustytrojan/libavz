#pragma once

#include <avz/analysis.hpp>
#include <avz/gfx.hpp>
#include <avz/media.hpp>

#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

namespace avz::examples
{

struct BassNationSpectrumLayer
{
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;
	avz::Interpolator ip;
	avz::SpectrumResampler sr;
	std::vector<float> s, a;
	avz::ColorSettings cs;
	avz::SpectrumDrawable spectrum;
	bool is_left;
	int sample_rate;
	int spectrum_number;

	BassNationSpectrumLayer(
		int spectrum_number, int fft_size, int sample_rate, sf::Vector2u size, sf::Color color, bool left);
	~BassNationSpectrumLayer();

	void compute(std::span<const float> audio_buffer);
	std::future<void> trigger_work(std::span<const float> audio_buffer);
	void configure_spectrum(bool prev, sf::Vector2u size);

#ifdef LIBAVZ_IMGUI
	void imgui();
#endif

private:
	void worker_loop();

#ifdef LIBAVZ_IMGUI
	std::string imgui_header{
		std::format("BassNationSpectrumLayer {} #{}", is_left ? "Left" : "Right", spectrum_number)};
#endif

	// worker state
	std::thread worker;
	std::mutex mu;
	std::condition_variable cv;
	bool has_work{false};
	bool stop{false};
	std::span<const float> work_audio;
	std::promise<void> work_promise;
};

} // namespace avz::examples
