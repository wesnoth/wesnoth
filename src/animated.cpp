/*
	Copyright (C) 2007 - 2025
	by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/** @file animated.cpp
 * Global timeline management for the animation system.
 * Maintains two parallel timelines:
 * - Normal timeline: Updated at real wall-clock rate
 * - Accelerated timeline: Can be sped up/slowed down
 * Call update_animation_timers() once per frame before advancing animations. */

#include "animated.hpp"

// Put these here to ensure that there's only
// one instance of the normal_timeline/accelerated_timeline variables
namespace {

	// Current time point on the normal (real-time) timeline
	std::chrono::steady_clock::time_point normal_timeline = std::chrono::steady_clock::now();

	// Current time point on the accelerated timeline (for unit movement, etc.)
	std::chrono::steady_clock::time_point accelerated_timeline = normal_timeline;

	std::chrono::steady_clock::time_point old_normal_timeline = normal_timeline;
}

void update_animation_timers(double acceleration)
{
	// Update normal timeline to current wall-clock time
	old_normal_timeline = normal_timeline;
	normal_timeline = std::chrono::steady_clock::now();

	// Update accelerated timeline based on elapsed time and acceleration factor
	const auto elapsed = normal_timeline - old_normal_timeline;
	if(acceleration != 1.0) {
		const auto accelerated_elapsed = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
			elapsed * acceleration
		);
		accelerated_timeline += accelerated_elapsed;
	} else {
		accelerated_timeline += elapsed;
	}
}

std::chrono::steady_clock::time_point get_current_animation_tick(bool uses_acceleration)
{
	return uses_acceleration ? accelerated_timeline : normal_timeline;
}
