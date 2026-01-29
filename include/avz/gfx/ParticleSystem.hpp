#pragma once

#include <SFML/Graphics.hpp>

namespace avz
{

class ParticleSystem : public sf::Drawable
{
private:
	// leave private here until there is another usecase
	class Particle : public sf::CircleShape
	{
		sf::Vector2f base_velocity;
		sf::Vector2f current_velocity;

	public:
		inline void updatePosition() { setPosition(getPosition() + current_velocity); }
		inline void setBaseVelocity(const sf::Vector2f v) { base_velocity = v; }
		inline void applyTimescale(float scale) { current_velocity = base_velocity * scale; }
	};

	sf::IntRect rect;
	std::vector<Particle> particles;
	float timestep_scale{1.f};
	bool debug_rect{};
	bool fade_out{true};
	bool start_offscreen{true};

	// Particle configuration
	unsigned int particle_point_count{10};
	float particle_radius_min{1.f};
	float particle_radius_max{5.f};
	float particle_velocity_x_min{-1.f};
	float particle_velocity_x_max{1.f};
	float particle_velocity_y_min{-1.f};
	float particle_velocity_y_max{0.f};

public:
	inline ParticleSystem(const sf::IntRect &rect, const int particle_count)
		: rect{rect},
		  particles{particle_count}
	{
		init_particles();
	}

	inline ParticleSystem(const sf::IntRect &rect, const int particle_count, const int framerate)
		: rect{rect},
		  particles{particle_count},
		  timestep_scale{framerate > 0 ? 60.f / framerate : 1.f}
	{
		init_particles();
	}

	/**
	 * Scales the particle's velocity by `60 / framerate`, keeping particle speed
	 * consistent regardless of the caller's framerate.
	 */
	void set_framerate(int framerate);

	/**
	 * Update the system, applying an additional displacement to all particles if nonzero.
	 */
	void update(const float additional_displacement = 0.f);

	inline void set_debug_rect(bool b) { debug_rect = b; }
	inline void set_fade_out(bool b) { fade_out = b; }

	inline void set_start_offscreen(bool b)
	{
		if (start_offscreen == b)
			return;
		start_offscreen = b;
		init_particles();
	}

	inline void set_particle_point_count(unsigned int count)
	{
		particle_point_count = count;
		init_particles();
	}

	inline void set_particle_radius_range(float min, float max)
	{
		particle_radius_min = min;
		particle_radius_max = max;
		init_particles();
	}

	inline void set_particle_velocity_x_range(float min, float max)
	{
		particle_velocity_x_min = min;
		particle_velocity_x_max = max;
		init_particles();
	}

	inline void set_particle_velocity_y_range(float min, float max)
	{
		particle_velocity_y_min = min;
		particle_velocity_y_max = max;
		init_particles();
	}

	inline unsigned int get_particle_point_count() const { return particle_point_count; }
	inline float get_particle_radius_min() const { return particle_radius_min; }
	inline float get_particle_radius_max() const { return particle_radius_max; }
	inline float get_particle_velocity_x_min() const { return particle_velocity_x_min; }
	inline float get_particle_velocity_x_max() const { return particle_velocity_x_max; }
	inline float get_particle_velocity_y_min() const { return particle_velocity_y_min; }
	inline float get_particle_velocity_y_max() const { return particle_velocity_y_max; }

	void draw(sf::RenderTarget &target, const sf::RenderStates states) const override;

	void set_rect(const sf::IntRect &rect);

	inline void set_particle_count(const size_t count)
	{
		particles.resize(count);
		init_particles();
	}

	inline size_t get_particle_count() const { return particles.size(); }
	inline sf::IntRect get_rect() const { return rect; }
	inline bool get_debug_rect() const { return debug_rect; }

private:
	void init_particle(Particle &p);
	void init_particles();
	void teleport_particle_opposite_side(Particle &p);
};

} // namespace avz
