#pragma once

#include <complex>
#include <fftw3.h>
#include <span>
#include <vector>

namespace avz
{

class fftwf_dft_r2c_1d
{
private:
	template <typename T>
	struct fftwf_allocator
	{
		using value_type = T;
		constexpr T *allocate(const size_t n) { return (T *)fftwf_malloc(n * sizeof(T)); }
		constexpr void deallocate(T *const p, size_t) { fftwf_free(p); }
	};

	template <typename T>
	using Vector = std::vector<T, fftwf_allocator<T>>;

	Vector<float> in;
	Vector<std::complex<float>> out;
	fftwf_plan plan{};

	constexpr void cleanup()
	{
		if (!plan)
			return;
		fftwf_destroy_plan(plan);
		plan = {};
	}

public:
	constexpr ~fftwf_dft_r2c_1d() { cleanup(); }

	constexpr void set_n(const int n)
	{
		in.resize(n);
		out.resize(n / 2 + 1);
		cleanup();
		plan = fftwf_plan_dft_r2c_1d(n, in.data(), (fftwf_complex *)out.data(), FFTW_ESTIMATE);
	}

	constexpr void execute() const { fftwf_execute(plan); }
	constexpr std::span<float> input() { return in; }
	constexpr std::span<const std::complex<float>> output() const { return out; }
};

} // namespace avz
