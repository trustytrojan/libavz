#pragma once

#include <avz/gfx/Base.hpp>
#include <avz/media/Media.hpp>

namespace avz
{

class Player
{
	Base &viz;
	Media &media;
	int framerate;

	// audio frames per video frame
	int afpvf{media.audio_sample_rate() / framerate};

public:
	Player(Base &viz, Media &media, int framerate);

	void start_in_window(const std::string &title);
	void encode(const std::string &outfile, const std::string &vcodec, const std::string &acodec);

	inline void set_framerate(int framerate)
	{
		if (this->framerate == framerate)
			return;
		this->framerate = framerate;
		afpvf = media.audio_sample_rate() / framerate;
	}
};

} // namespace avz
