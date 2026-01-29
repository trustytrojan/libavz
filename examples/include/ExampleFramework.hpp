#pragma once

#include <avz/gfx.hpp>
#include <avz/main.hpp>
#include <avz/media.hpp>

#include <SFML/Graphics.hpp>
#include <argparse/argparse.hpp>

// clang-format off
#define EXAMPLE_ARGS_GETTER(type, name, arg_str) \
	inline constexpr type get_##name() const { return get<type>(arg_str); }
// clang-format on

namespace avz::examples
{

struct Args : argparse::ArgumentParser
{
	Args(int argc, const char *const *argv, float default_audio_duration = 0.25f, bool auto_parse = true);

	inline constexpr void parse(int argc, const char *const *argv)
	{
		try
		{
			parse_args(argc, argv);
		}
		catch (const std::exception &err)
		{
			std::cerr << err.what() << '\n';
			std::exit(EXIT_FAILURE);
		}
	}

	inline constexpr sf::Vector2u get_size() const
	{
		const auto &size_args = get<std::vector<unsigned>>("--size");
		return {size_args[0], size_args[1]};
	}

	EXAMPLE_ARGS_GETTER(std::string, media_url, "media_url")
	EXAMPLE_ARGS_GETTER(unsigned, framerate, "-r")
	EXAMPLE_ARGS_GETTER(float, audio_duration_sec, "-d")
	EXAMPLE_ARGS_GETTER(unsigned, media_start_time, "--media-start")
	EXAMPLE_ARGS_GETTER(bool, profiler_enabled, "-p")
	EXAMPLE_ARGS_GETTER(std::string, font_path, "--text-font")
	EXAMPLE_ARGS_GETTER(bool, geometry_shader_enabled, "-gs")
};

/**
 * @brief Base class for example programs
 *
 * Provides common initialization and reduces boilerplate code
 */
class ExampleBase : public avz::Base
{
public:
	avz::FfmpegPopenMedia media;
	int sample_rate_hz;
	int num_channels;

	ExampleBase(const Args &args);
	virtual ~ExampleBase() = default;
};

/**
 * @brief Alternative main function helper that allows custom audio frame calculation
 */
#define LIBAVZ_EXAMPLE_MAIN_CUSTOM(VizClass, description, default_audio_duration, audio_frames_expr)     \
	int main(int argc, const char *const *argv)                                                          \
	{                                                                                                    \
		avz::examples::Args args{argc, argv, default_audio_duration};                                    \
		VizClass viz{args};                                                                              \
		int audio_frames = (audio_frames_expr);                                                          \
		avz::Player{viz, viz.media, (int)args.get_framerate(), audio_frames}.start_in_window(#VizClass); \
		return EXIT_SUCCESS;                                                                             \
	}

} // namespace avz::examples
