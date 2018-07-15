/*
   Copyright (C) 2011 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include <SDL_rect.h>
#include <string>

class t_string;
struct point;

namespace gui2
{

namespace dialogs
{

namespace tip
{

/**
 * Shows a tip.
 *
 * The tip is a tooltip or a helptip. One type of tip is shown at the same
 * time, opening a second tip closes the first.
 *
 * @param window_id           The id of the window used to show the tip.
 * @param message             The message to show in the tip.
 * @param mouse               The position of the mouse.
 */
void show(const std::string& window_id,
		  const t_string& message,
		  const point& mouse,
		  const SDL_Rect& source_rect);

/**
 * Removes a tip.
 *
 * It is safe to call this function when no tip is shown.
 * */
void remove();

} // namespace tip
} // namespace dialogs
} // namespace gui2
