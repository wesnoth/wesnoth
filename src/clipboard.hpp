/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef CLIPBOARD_HPP_INCLUDED
#define CLIPBOARD_HPP_INCLUDED

#include <string>
#include "SDL.h"

/**
 * Copies text to the clipboard.
 *
 * @param text         The text to copy.
 * @param mouse        Is the selection done by the mouse? On UNIX systems there
 *                     are multiple clipboards and the mouse selction uses a
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

#if defined(_X11) && !defined(__APPLE__)
void handle_system_event(const SDL_Event& ev);
#endif

#endif
