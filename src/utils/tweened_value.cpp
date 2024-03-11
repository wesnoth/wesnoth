/*
	Copyright (C) 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "utils/tweened_value.hpp"
#include "log.hpp"
#include <algorithm>

namespace utils
{
tweened_value::tweened_value(
	int32_t value_start, int32_t value_end, tweened_value::Resolution duration, std::function<double(double)> easing)
	: duration{duration}
	, value_start{value_start}
	, delta{value_end - value_start}
	, start_time{std::nullopt}
	, easing{easing}
{
}

std::pair<int32_t, bool> tweened_value::value()
{
	if(delta == 0) {
	//	return {value_start, true};
	}

	if(!start_time) {
		start_time = std::chrono::steady_clock::now();
		return {value_start, false};
	}

	const auto elapsed = elapsed_time();
	if(elapsed >= duration) {
		start_time.reset();
		return {value_start + delta, true};
	}

	const double completed = std::clamp(1.0 * elapsed / duration, 0.0, 1.0);
	return {value_start + (delta * easing(completed)), false};
}

tweened_value::Resolution tweened_value::elapsed_time() const
{
	if(start_time) {
		return std::chrono::duration_cast<Resolution>(std::chrono::steady_clock::now() - *start_time);
	} else {
		return Resolution{0};
	}
}

int32_t tweened_value_queue::value()
{
	if(queue.empty()) {
		throw std::runtime_error("Attempt to get tweened value from empty queue");
	}

	const auto& [value, stage_complete] = queue.front().value();

	if(stage_complete) {
		if(mode == loop_mode::loop) {
			loop_queue.push(std::exchange(queue.front(), tweened_value{}));
		}

		queue.pop();
	}

	if(queue.empty()) {
		if(on_queue_exhausted) {
			on_queue_exhausted();
		}

		if(mode == loop_mode::loop) {
			std::swap(loop_queue, queue);
		}
	}

	return value;
}

} // end namespace utils
