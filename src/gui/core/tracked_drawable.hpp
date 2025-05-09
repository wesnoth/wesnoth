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

#pragma once

#include "events.hpp"

#include <boost/circular_buffer.hpp>

#include <chrono>
#include "utils/optional_fwd.hpp"
#include <tuple>

namespace gui2
{
/**
 * Middleware class that tracks framerate and times.
 *
 * It should be used in conjunction with classes that implement top_level_drawable.
 * Strictly, it will measure the between invocations of update_count, which should
 * be invoked from the render function.
 */
class tracked_drawable : private events::pump_monitor
{
public:
	tracked_drawable();

	struct frame_info
	{
		std::chrono::milliseconds min_time{};
		std::chrono::milliseconds avg_time{};
		std::chrono::milliseconds max_time{};

		unsigned min_fps{};
		unsigned avg_fps{};
		unsigned max_fps{};
		unsigned act_fps{};
	};

	/** Returns the current frame time and info, or nullopt if no times have been recorded. */
	auto get_info() const -> utils::optional<frame_info>;

protected:
	~tracked_drawable();

	/** Records time since last invocation. */
	void update_count();

private:
	/** Inherited from events::pump_monitor. */
	void process() override;

	using clock = std::chrono::steady_clock;
	using times = std::tuple<clock::duration, clock::duration, clock::duration>;

	/** Get min, average, and max frametimes in steady_clock resolution. */
	auto get_times() const -> times;

	boost::circular_buffer<clock::duration> frametimes_;

	unsigned render_count_;
	unsigned render_counter_;

	clock::time_point last_lap_;
	clock::time_point last_render_;
};

} // namespace gui2
