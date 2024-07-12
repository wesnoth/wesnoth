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

#pragma once

#include "utils/easings.hpp"

#include <chrono>
#include <functional>
#include <initializer_list>
#include <optional>
#include <queue>
#include <tuple>

namespace utils
{
class tweened_value
{
	using Resolution = std::chrono::milliseconds;

public:
	tweened_value() = default;

	tweened_value(int32_t value_start, int32_t value_end, Resolution duration, std::function<double(double)> easing);

	std::pair<int32_t, bool> value();

	Resolution elapsed_time() const;

private:
	Resolution duration;

	int32_t value_start;
	int32_t delta;

	std::optional<std::chrono::steady_clock::time_point> start_time;
	std::function<double(double)> easing;
};

class tweened_value_queue
{
public:
	enum class loop_mode { once, loop };

	tweened_value_queue() = default;

	tweened_value_queue(std::initializer_list<tweened_value> list)
		: queue{decltype(queue)::container_type{list}}
	{
	}

	void queue_value(tweened_value v)
	{
		queue.push(std::move(v));
	}

	void on_complete(std::function<void()> f)
	{
		on_queue_exhausted = f;
	}

	int32_t value();

private:
	std::queue<tweened_value> queue;
	std::queue<tweened_value> loop_queue;

	std::function<void()> on_queue_exhausted;

	loop_mode mode = loop_mode::loop;
};

} // end namespace utils
