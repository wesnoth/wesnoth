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

namespace gui2
{
class tracked_drawable;

namespace dialogs::fps
{
/**
 * Displays the fps report popup for the given tracked_drawable.
 *
 * Only one popup may be active at a time, and subsequent calls
 * to this function are no-op unless @ref hide is called first.
 */
void show(const gui2::tracked_drawable& target);

/** Hides the fps report popup. */
void hide();

} // namespace dialogs::fps

} // namespace gui2
