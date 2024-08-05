/*
	Copyright (C) 2008 - 2024
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

#pragma once

#include <string>

namespace gui2
{
/**
 * Initializes the GUI subsystems.
 *
 * @note This function must be called before other parts of the UI engine
 * are used. Use @ref switch_theme below to actually activate the theme.
 */
void init();

/**
 * Set and activate the given gui2 theme
 *
 * @param current_theme    the name of the gui2 theme to switch to
 */
void switch_theme(const std::string& current_theme);


} // namespace gui2
