/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

/**
 *  @file
 *  Manage display-related preferences, e.g. screen-size, etc.
 */

#include "global.hpp"
#include "preferences_display.hpp"

#include "display.hpp"
#include "formatter.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"

#include <boost/foreach.hpp>
#include <boost/math/common_factor_rt.hpp>

namespace preferences {

static display* disp = NULL;

display_manager::display_manager(display* d)
{
	disp = d;

	load_hotkeys();

	set_grid(grid());
	set_turbo(turbo());
	set_turbo_speed(turbo_speed());
	set_scroll_to_action(scroll_to_action());
	set_color_cursors(preferences::get("color_cursors", false));
}

display_manager::~display_manager()
{
	disp = NULL;
}

void set_scroll_to_action(bool ison)
{
	_set_scroll_to_action(ison);
}

void set_turbo(bool ison)
{
	_set_turbo(ison);

	if(disp != NULL) {
		disp->set_turbo(ison);
	}
}

void set_turbo_speed(double speed)
{
	save_turbo_speed(speed);

	if(disp != NULL) {
		disp->set_turbo_speed(speed);
	}
}

void set_ellipses(bool ison)
{
	_set_ellipses(ison);
}

void set_grid(bool ison)
{
	_set_grid(ison);

	if(disp != NULL) {
		disp->set_grid(ison);
	}
}

void set_color_cursors(bool value)
{
	_set_color_cursors(value);

	cursor::set();
}

void set_idle_anim(bool ison) {
	_set_idle_anim(ison);
	if(disp != NULL) {
		disp->set_idle_anim(ison);
	}
}

void set_idle_anim_rate(int rate) {
	_set_idle_anim_rate(rate);
	if(disp != NULL) {
		disp->set_idle_anim_rate(rate);
	}
}

} // end namespace preferences

