/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#pragma once

#include <string>

union SDL_Event;

namespace desktop {

namespace clipboard {

/**
 * Copies text to the clipboard.
 *
 * @param text         The text to copy.
 * @param mouse        Is the selection done by the mouse? On UNIX systems there
 *                     are multiple clipboards and the mouse selection uses a
 *                     different clipboard. Ignored on other systems.
 */
void copy_to_clipboard(const std::string& text, const bool mouse);

/**
 * Copies text from the clipboard.
 *
 * @param mouse        Is the pasting done by the mouse?
 *
 * @returns            String on clipbaord.
 */
std::string copy_from_clipboard(const bool mouse);

void handle_system_event(const SDL_Event& ev);

/**
 * Whether wesnoth was compiled with support for a clipboard.
 */
bool available();

} // end namespace clipboard

} // end namespace desktop
