#pragma once

#include <SFML/System.hpp>
#include <algorithm>
#include <format>
#include <limits>
#include <stack>
#include <string>
#include <vector>

namespace avz
{

class Profiler
{
	struct TimingStat
	{
		std::string name;
		float min{std::numeric_limits<float>::max()}, max{}, total{}, current{};
		size_t count{};
		constexpr float avg() const { return (count == 0) ? 0.0f : total / count; }
		constexpr operator std::string() const
		{
			return std::format("{:<20}{:<7.3f}{:<7.3f}{:<7.3f}{:<7.3f}\n", name, current, avg(), min, max);
		}
	};

	std::vector<TimingStat> stats;
	std::stack<sf::Clock> current_clocks;
	std::stack<std::string> current_sections;

public:
	constexpr void startSection(std::string_view name)
	{
		current_sections.emplace(name);
		current_clocks.emplace();
	}

	constexpr void endSection()
	{
		auto &clock = current_clocks.top();
		clock.stop();
		const float time_ms = clock.getElapsedTime().asMicroseconds() / 1e3f;
		current_clocks.pop(); // destroys the clock!

		const auto &current_section = current_sections.top();
		auto &stat = get_or_create_timing_stat(current_section);
		current_sections.pop(); // destroys the string!

		stat.current = time_ms;
		if (time_ms < stat.min)
			stat.min = time_ms;
		if (time_ms > stat.max)
			stat.max = time_ms;
		stat.total += time_ms;
		stat.count++;
	}

	constexpr void endStartSection(std::string_view name)
	{
		endSection();
		startSection(name);
	}

	constexpr std::string getSummary()
	{
		auto s = std::format("{:<20}{:<7}{:<7}{:<7}{:<7}\n", "", "curr", "avg", "min", "max");
		for (const auto &stat : stats)
			s += stat;
		return s;
	}

private:
	TimingStat &get_or_create_timing_stat(const std::string &label)
	{
		const auto it = std::ranges::find_if(stats, [&label](auto &stat) { return stat.name == label; });
		return (it != stats.end()) ? *it : stats.emplace_back(label);
	}
};

} // namespace avz
