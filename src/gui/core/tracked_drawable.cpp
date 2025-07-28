/*
	Copyright (C) 2025 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/core/tracked_drawable.hpp"

#include "gui/dialogs/fps_report.hpp"
#include "preferences/preferences.hpp"

#include <algorithm>
#include <numeric>
#include <utility>

namespace gui2
{
using namespace std::chrono_literals;

tracked_drawable::tracked_drawable()
	: frametimes_(50)
	, render_count_(0)
	, render_counter_(0)
	, last_lap_()
	, last_render_(clock::time_point::max())
{
}

tracked_drawable::~tracked_drawable()
{
	// Unconditionally close the fps report, since there currently can only be one
	// report open at a time. If we need to support multiple reports, we should add
	// a way to track its associated drawable and close its companion report here.
	gui2::dialogs::fps::hide();
}

void tracked_drawable::process()
{
	if(prefs::get().show_fps()) {
		gui2::dialogs::fps::show(*this);
	} else {
		gui2::dialogs::fps::hide();
	}
}

auto tracked_drawable::get_info() const -> utils::optional<frame_info>
{
	if(frametimes_.empty()) {
		return utils::nullopt;
	}

	using std::chrono::milliseconds;
	const auto [min_time, avg_time, max_time] = get_times();

	return frame_info {
		std::chrono::duration_cast<milliseconds>(min_time),
		std::chrono::duration_cast<milliseconds>(avg_time),
		std::chrono::duration_cast<milliseconds>(max_time),

		// NOTE: max fps corresponds to the *shortest* time between frames, and vice-versa
		static_cast<unsigned>(1s / max_time), // min
		static_cast<unsigned>(1s / avg_time), // avg
		static_cast<unsigned>(1s / min_time), // max
		render_count_
	};
}

auto tracked_drawable::get_times() const -> times
{
	const auto [min_time, max_time]
		= std::minmax_element(frametimes_.begin(), frametimes_.end());

	const auto total_time
		= std::accumulate(frametimes_.begin(), frametimes_.end(), clock::duration{0});

	return { *min_time, total_time / frametimes_.size(), *max_time };
}

void tracked_drawable::update_count()
{
	auto now = clock::now();
	auto elapsed = now - last_render_;

	if(elapsed > clock::duration{0}) {
		frametimes_.push_back(elapsed);
	}

	last_render_ = now;
	++render_counter_;

	if(now - last_lap_ >= 1s) {
		last_lap_ = now;
		render_count_ = std::exchange(render_counter_, 0);
	}
}

} // namespace gui2
