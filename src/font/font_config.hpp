/*
	Copyright (C) 2016 - 2024
	by Chris Beck<render787@gmail.com>
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

/***
 * The font::manager initializes cairo and font_config in order to figure out
 * what local fonts to use.
 */

#include "font_options.hpp"

class t_string;

namespace font {

// Object which initializes font rendering libraries and related caches.
// When it is created, the font directory is found within game_config::path.
// If that path changes then this object should be destroyed and recreated.
//
// You should not use GUI1 or GUI2 or any font api function unless a
// font::manager is alive.
// Don't create two font::manager objects at once.
//
struct manager {
	manager();
	~manager();

	manager(const manager &) = delete;
	manager & operator = (const manager &) = delete;
};

/***
 * load_font_config actually searches the game font path for fonts, and refreshes
 * the set of loaded fonts
 *
 * Returns true in case of success
 */
bool load_font_config();

/** Returns the currently defined fonts. */
const t_string& get_font_families(family_class fclass = FONT_SANS_SERIF);

} // end namespace font
