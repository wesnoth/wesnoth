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

#include "desktop/clipboard.hpp"
#include "serialization/unicode.hpp"
#include <algorithm>

#include <SDL_events.h>
#include <SDL_clipboard.h>

#define CLIPBOARD_FUNCS_DEFINED

/*
 * Note SDL 2.0 has its own clipboard routines, but they don't support
 * different clipboards (yet).
 */

namespace desktop {

namespace clipboard {

void copy_to_clipboard(const std::string& text, const bool)
{
	SDL_SetClipboardText(text.c_str());
}

std::string copy_from_clipboard(const bool)
{
	char* clipboard = SDL_GetClipboardText();
	if(!clipboard) {
		return std::string();
	}

	const std::string result(clipboard);
	SDL_free(clipboard);
	return result;
}

void handle_system_event(const SDL_Event& /*event*/)
{
}

bool available()
{
	return true;
}

} // end namespace clipboard

} // end namespace desktop

