#pragma once

#include <avz/gfx/Layer.hpp>
#include <avz/gfx/Profiler.hpp>
#include <avz/gfx/RenderTexture.hpp>

#include <SFML/Graphics.hpp>
#include <memory>
#include <span>
#include <string>
#include <vector>

#define capture_time(label, code)     \
	if (profiler_enabled)             \
	{                                 \
		profiler.startSection(label); \
		code;                         \
		profiler.endSection();        \
	}                                 \
	else                              \
		code;

namespace avz
{

/**
 * Base class containing the boilerplate for an avz visualizer.
 * Extend this class to start building your own visualizer!
 */
class Base : public sf::Drawable
{
public:
	const sf::Vector2u size;

protected:
	bool profiler_enabled{};
	Profiler profiler;
	sf::Font font;

private:
	std::vector<std::unique_ptr<Layer>> layers;
	RenderTexture final_rt;
	sf::Text profiler_text{font};

public:
	Base(sf::Vector2u size);

	// Emplace a derived Layer type in-place and return a pointer to it.
	template <typename T, typename... Args>
	T &emplace_layer(Args &&...args)
	{
		static_assert(std::is_base_of_v<Layer, T>, "T must derive from Layer");
		auto up = std::make_unique<T>(std::forward<Args>(args)...);
		const auto ptr = up.get();
		layers.emplace_back(std::move(up));
		return *ptr;
	}

	// Find a layer by name (non-owning raw pointer). Returns nullptr if not found.
	Layer *get_layer(const std::string &name);

	// Get a layer by name with automatic type casting.
	template <typename T>
	T *get_layer_as(const std::string &name)
	{
		static_assert(std::is_base_of_v<Layer, T>, "T must derive from Layer");
		return dynamic_cast<T *>(get_layer(name));
	}

	/**
	 * Prepare the next frame to be drawn with `draw()`. Calls the derived `update()` and runs all layers.
	 * @param audio_buffer A span of the audio data for this frame.
	 */
	void next_frame(std::span<const float> audio_buffer);

	void draw(sf::RenderTarget &, sf::RenderStates) const override;

	inline void set_font(const std::string &path) { font = sf::Font{path}; }
	inline void set_profiler_enabled(bool b) { profiler_enabled = b; }

protected:
	virtual void update(std::span<const float> audio_buffer) {}
};

} // namespace avz
