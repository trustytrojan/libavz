#include <algorithm>
#include <avz/analysis/BinPacker.hpp>
#include <cmath>
#include <stdexcept>

#ifdef LIBAVZ_IMGUI
#include <imgui.h>
#endif

namespace avz
{

void BinPacker::_scale_max::calc(const int max, float nthroot_inv)
{
	linear = max;
	log = ::logf(max);
	sqrt = ::sqrtf(max);
	cbrt = ::cbrtf(max);
	nthroot = ::powf(max, nthroot_inv);
}

void BinPacker::set_scale(const Scale scale)
{
	this->scale = scale;
	compute_bin_pack_index_mappings(bin_pack_index_mapping.size(), bin_pack_input_size);
}

void BinPacker::set_nth_root(const int nth_root)
{
	if (!nth_root)
		throw std::invalid_argument{"[BinPacker::set_nth_root] nth_root cannot be zero!"};
	this->nth_root = nth_root;
	nthroot_inv = 1.f / nth_root;
	compute_bin_pack_index_mappings(bin_pack_index_mapping.size(), bin_pack_input_size);
}

void BinPacker::bin_pack(std::span<float> out, std::span<const float> in)
{
	const auto out_size = out.size(), in_size = in.size();

	if (out_size != bin_pack_index_mapping.size() || in_size != bin_pack_input_size)
		compute_bin_pack_index_mappings(out_size, in_size);

	std::ranges::fill(out, 0);

	for (size_t i = 0; i < out_size; ++i)
	{
		const auto [start, end] = bin_pack_index_mapping[i];

		if (start == -1)
			continue;

		float a{};
		for (int j = start; j < end; ++j)
		{
			const float x = in[j];
			if (am == AccumulationMethod::SUM)
				a += x;
			else
				a = std::max(a, x);
		}

		out[i] = a;
	}
}

float BinPacker::calc_index_ratio(const float i) const
{
	switch (scale)
	{
	case Scale::LINEAR:
		return i / scale_max.linear;
	case Scale::LOG:
		return logf(i ? i : 1) / scale_max.log;
	case Scale::NTH_ROOT:
		switch (nth_root)
		{
		case 1:
			return i / scale_max.linear;
		case 2:
			return sqrtf(i) / scale_max.sqrt;
		case 3:
			return cbrtf(i) / scale_max.cbrt;
		default:
			return powf(i, nthroot_inv) / scale_max.nthroot;
		}
	default:
		throw std::logic_error{"[BinPacker::calc_index_ratio] default case hit"};
	}
}

void BinPacker::compute_bin_pack_index_mappings(const size_t out_size, const size_t in_size)
{
	// update cached scale maxima for mapping calculations
	scale_max.calc(in_size, nthroot_inv);
	bin_pack_input_size = in_size;
	bin_pack_index_mapping.assign(out_size, {-1, -1});

	for (size_t i = 0; i < in_size; ++i)
	{
		const auto out_index = std::clamp((size_t)(calc_index_ratio(i) * out_size), (size_t)0, out_size - 1);
		if (bin_pack_index_mapping[out_index].first == -1)
			bin_pack_index_mapping[out_index].first = i;
		bin_pack_index_mapping[out_index].second = i + 1;
	}
}

} // namespace avz

#ifdef LIBAVZ_IMGUI
void avz::BinPacker::imgui()
{
	if (ImGui::CollapsingHeader("BinPacker"))
	{
		const char *scales[] = {"Linear", "Log", "Nth root"};
		int scale_idx = static_cast<int>(scale);
		if (ImGui::Combo("Scale", &scale_idx, scales, IM_ARRAYSIZE(scales)))
			set_scale(static_cast<Scale>(scale_idx));

		if (scale == Scale::NTH_ROOT)
		{
			int nth = nth_root;
			if (ImGui::SliderInt("Nth root", &nth, 2, 16))
				set_nth_root(nth);
		}

		const char *accum[] = {"SUM", "MAX"};
		ImGui::Combo("Accumulation", (int *)&am, accum, IM_ARRAYSIZE(accum));
	}
}
#endif
