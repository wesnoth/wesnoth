/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 * Contains the gui2 timer routines.
 *
 * This code avoids the following problems with the sdl timers:
 * - the callback must be a C function with a fixed signature.
 * - the callback needs to push an event in the event queue, between the
 *   pushing and execution of that event the timer can't be stopped. (Makes
 *   sense since the timer has expired, but not what the user wants.)
 *
 * With these functions it's possible to remove the event between pushing in
 * the queue and the actual execution. Since the callback is a std::function
 * object it's possible to make the callback as fancy as wanted.
 */

#pragma once

#include <chrono>
#include <functional>

namespace gui2
{

/**
 * Adds a new timer.
 *
 * @param interval                The timer interval in ms.
 * @param callback                The function to call when the timer expires,
 *                                the id send as parameter is the id of the
 *                                timer.
 * @param repeat                  If true the timer will restart after it
 *                                expires.
 *
 * @returns                       The id of the timer.
 * @retval [0]                    Failed to create a timer.
 */
std::size_t add_timer(const std::chrono::milliseconds& interval,
				 const std::function<void(std::size_t id)>& callback,
				 const bool repeat = false);

/**
 * Removes a timer.
 *
 * It's save to remove a timer in its own callback, only the value returned
 * might not be accurate. The destruction is postponed until the execution is
 * finished  and the return value is whether the postponing was successful.
 *
 * @param id                      The id of the timer to remove, this is the id
 *                                returned by add_timer.
 *
 * @returns                       Status, false if the timer couldn't be
 *                                removed.
 */
bool remove_timer(const std::size_t id);

/**
 * Executes a timer.
 *
 * @note this function is only meant to be executed by the event handling
 * system.
 *
 * @param id                      The id of the timer to execute, this is the
 *                                id returned by add_timer.
 *
 * @returns                       Status, false if the timer couldn't be
 *                                executed.
 */
bool execute_timer(const std::size_t id);

} // namespace gui2
