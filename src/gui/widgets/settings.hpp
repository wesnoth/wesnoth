/*
   Copyright (C) 2007 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
 * This file contains the settings handling of the widget library.
 */

#pragma once

#include "gui/auxiliary/tips.hpp"
#include "tstring.hpp"

#include <string>
#include <vector>

namespace gui2
{

class gui_definition;
class game_tip;

/** Do we wish to use the new library or not. */
extern bool new_widgets;

/** This namespace contains the 'global' settings. */
namespace settings
{
/**
 * The screen resolution should be available for all widgets since
 * their drawing method will depend on it.
 */
extern unsigned screen_width;
extern unsigned screen_height;

/**
 * The offset between the left edge of the screen and the gamemap.
 */
extern unsigned gamemap_x_offset;

/**
 * The size of the map area, if not available equal to the screen
 * size.
 */
extern unsigned gamemap_width;
extern unsigned gamemap_height;

/** These are copied from the active gui. */
extern unsigned popup_show_delay;
extern unsigned popup_show_time;
extern unsigned help_show_time;
extern unsigned double_click_time;
extern unsigned repeat_button_repeat_time;

extern std::string sound_button_click;
extern std::string sound_toggle_button_click;
extern std::string sound_toggle_panel_click;
extern std::string sound_slider_adjust;

extern t_string has_helptip_message;

extern std::vector<game_tip> tips;

/**
 * Update the size of the screen variables in settings.
 *
 * Before a window gets build the screen sizes need to be updated. This
 * function does that. It's only done when no other window is active, if
 * another window is active it already updates the sizes with it's resize
 * event.
 */
void update_screen_size_variables();
}

} // namespace gui2
