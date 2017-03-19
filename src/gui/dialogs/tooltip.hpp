/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_TIP_HPP_INCLUDED
#define GUI_DIALOGS_TIP_HPP_INCLUDED

#include <SDL_rect.h>
#include <string>

class CVideo;
class t_string;

namespace gui2
{

struct point;

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
 * @param video               The video which contains the surface to draw
 *                            upon.
 * @param window_id           The id of the window used to show the tip.
 * @param message             The message to show in the tip.
 * @param mouse               The position of the mouse.
 */
void show(CVideo& video,
		  const std::string& window_id,
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

#endif
