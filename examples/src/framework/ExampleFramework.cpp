#include "ExampleFramework.hpp"

namespace avz::examples
{

Args::Args(const int argc, const char *const *const argv, const float default_audio_duration)
{
	// clang-format off
	// Required positional argument: media file
	add_argument("media_url")
		.help("Path to the media file (audio/video)");

	// Optional arguments with sensible defaults
	add_argument("-s", "--size")
		.help("Window size as width height (pixels)")
		.nargs(2)
		.default_value(std::vector<unsigned>{1280, 720})
		.scan<'u', unsigned>();

	add_argument("-r", "--framerate")
		.help("Output framerate (FPS)")
		.default_value(60u)
		.scan<'u', unsigned>();

	add_argument("-d", "--audio-duration")
		.help("Audio window duration, for examples that need one (seconds)")
		.default_value(default_audio_duration)
		.scan<'f', float>();

	add_argument("--media-start")
		.help("Media seek position (seconds)")
		.default_value(0u)
		.scan<'u', unsigned>();

	add_argument("-p", "--profiler")
		.help("Enable performance profiler")
		.flag();

	add_argument("--text-font")
		.help("Path to font file for profiler")
		.default_value("");

	add_argument("-gs", "--geometry-shader")
		.help("Use geometry shader, if the example has one")
		.flag();
	// clang-format on

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

ExampleBase::ExampleBase(const Args &args)
	: Base{args.get_size()},
	  media{std::string{args.get_media_url()}, static_cast<float>(args.get_media_start_time())},
	  sample_rate_hz{media.audio_sample_rate()},
	  num_channels{media.audio_channels()}
{
	if (args.get_profiler_enabled())
	{
		enable_profiler();
		if (args.get_font_path().size())
			set_font(std::string{args.get_font_path()});
		else
			std::cerr << "profiler enabled but no font file provided, profiler text will not be visible\n";
	}
}

} // namespace avz::examples
