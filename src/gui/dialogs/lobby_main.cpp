/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth"

#include "gui/dialogs/lobby_main.hpp"
#include "gui/dialogs/field.hpp"

namespace gui2 {

tlobby_main::tlobby_main()
{
}

twindow* tlobby_main::build_window(CVideo& video)
{
	return build(video, get_id(LOBBY_MAIN));
}

} // namespace gui2
