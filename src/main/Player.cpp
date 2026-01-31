#include <avz/main/Player.hpp>
#include <avz/media/FfmpegPopenEncoder.hpp>

#ifdef LIBAVZ_PORTAUDIO
#include <fcntl.h>
#include <portaudio.hpp>
#endif

#ifdef LIBAVZ_IMGUI
#include <imgui-SFML.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#endif

namespace avz
{

Player::Player(Base &viz, Media &media, int framerate)
	: viz{viz},
	  media{media},
	  framerate{framerate}
{
}

/*
TODO: You want to make libavz more usage-agnostic by removing portaudio/imgui dependencies.
libavz by itself should be a machine that produces user-defined video from ambiguous audio.
Let client programs/callers/end-users handle the gathering of audio and whether drawables are editable.
*/

void Player::start_in_window(const std::string &title)
{
	sf::RenderWindow window{
		sf::VideoMode{viz.size},
		title,
		sf::Style::Titlebar | sf::Style::Close,
		sf::State::Windowed,
	};
	window.setVerticalSyncEnabled(true);

#ifdef LIBAVZ_IMGUI
	if (!ImGui::SFML::Init(window))
	{
		std::cerr << "[Player::start_in_window] Failed to initialize ImGui-SFML\n";
		return;
	}
	// clock used by ImGui-SFML to compute deltaTime each frame
	sf::Clock imgui_delta_clock;
#endif

#ifdef LIBAVZ_PORTAUDIO
#ifdef __linux__
	// Suppress ALSA warnings during PortAudio initialization
	int stderr_backup = dup(STDERR_FILENO);
	int devnull = open("/dev/null", O_WRONLY);
	dup2(devnull, STDERR_FILENO);
	close(devnull);
#endif

	pa::Init pa_init;
	pa::Stream pa_stream{0, media.audio_channels(), paFloat32, media.audio_sample_rate(), afpvf};
	pa_stream.start();

#ifdef __linux__
	// Restore stderr
	dup2(stderr_backup, STDERR_FILENO);
	close(stderr_backup);
#endif
#endif

	bool paused{}, vsync{true}, profiler{};
	window.setVerticalSyncEnabled(vsync);

#ifdef LIBAVZ_IMGUI
	std::string font_path_str;
#endif

	const auto handlePause = [&]
	{
		paused = !paused;
#ifdef LIBAVZ_PORTAUDIO
		if (paused)
			pa_stream.stop();
		else
			pa_stream.start();
#endif
	};

	const auto handleVsync = [&]
	{
		vsync = !vsync;
		window.setVerticalSyncEnabled(vsync);
	};

	const auto handleProfiler = [&]
	{
		profiler = !profiler;
		viz.set_profiler_enabled(profiler);
	};

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
#ifdef LIBAVZ_IMGUI
			ImGui::SFML::ProcessEvent(window, *event);
#else
			if (const auto key = event->getIf<sf::Event::KeyPressed>())
				switch (key->scancode)
				{
					// clang-format off
				case sf::Keyboard::Scan::Space: handlePause(); break;
				case sf::Keyboard::Scan::V: handleVsync(); break;
				case sf::Keyboard::Scan::P: handleProfiler(); break;
				case sf::Keyboard::Scan::Escape: window.close(); break;
				default: break;
					// clang-format on
				}
#endif
		}

#ifdef LIBAVZ_IMGUI
		ImGui::SFML::Update(window, imgui_delta_clock.restart());
		ImGui::Begin("Player");
		int fr = framerate;
		if (ImGui::SliderInt("Framerate", &fr, 1, 60))
		{
			set_framerate(fr);
			window.setFramerateLimit(fr);
		}
		if (ImGui::Button(paused ? "Play" : "Pause"))
			handlePause();
		if (ImGui::Button(vsync ? "VSync Enabled" : "VSync Disabled"))
			handleVsync();
		if (ImGui::Button(profiler ? "Disable Profiler" : "Enable Profiler"))
			handleProfiler();
		if (ImGui::InputText("Font Path", &font_path_str) && std::filesystem::is_regular_file(font_path_str))
			try
			{
				viz.set_font(font_path_str);
			}
			catch (const std::exception &e)
			{
				ImGui::TextColored({1, 0, 0, 1}, "Failed to load font: %s", e.what());
			}
		ImGui::End();
#endif

		const auto frames = std::max(viz.get_audio_frames_needed(), afpvf);
		const auto audio = media.read_audio(frames);

		if (!audio)
		{
			window.close();
			continue;
		}

#ifdef LIBAVZ_PORTAUDIO
		if (!paused)
		{
			try
			{
				pa_stream.write(audio->data(), afpvf);
			}
			catch (const pa::Error &e)
			{
				if (e.code != paOutputUnderflowed)
					throw e;
				std::cerr << "PortAudio: Output underflowed\n";
			}
		}
#endif

		viz.next_frame(*audio);

		if (!paused)
		{
			// erase the audio "played" during this frame
			media.consume_audio(afpvf);
		}

		window.clear();
		window.draw(viz);
#ifdef LIBAVZ_IMGUI
		ImGui::SFML::Render(window);
#endif
		window.display();
	}

#ifdef LIBAVZ_IMGUI
	ImGui::SFML::Shutdown();
#endif
}

void Player::encode(const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	// Create OpenGL context first (sf::RenderWindow usually does this for us) otherwise GL extensions will be null!
	sf::Context c;
	sf::RenderTexture rt{viz.size};
	FfmpegPopenEncoder ffmpeg{media.url, viz.size.x, viz.size.y, framerate, outfile, vcodec, acodec};
	while (true)
	{
		const auto frames = std::max(viz.get_audio_frames_needed(), afpvf);
		const auto audio = media.read_audio(frames);

		if (!audio)
			break;

		viz.next_frame(*audio);

		// erase the audio "played" during this frame
		media.consume_audio(afpvf);

		rt.clear();
		rt.draw(viz);
		rt.display();
		ffmpeg.send_frame(rt.getTexture().getNativeHandle());
	}
}

} // namespace avz
