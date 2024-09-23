/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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

union SDL_Event;

namespace desktop {

namespace clipboard {

/**
 * Copies text to the clipboard.
 *
 * @param text         The text to copy.
 */
void copy_to_clipboard(const std::string& text);

/**
 * Copies text from the clipboard.
 *
 * @returns            String on clipbaord.
 */
std::string copy_from_clipboard();

} // end namespace clipboard

} // end namespace desktop
