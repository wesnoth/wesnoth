/*
   Copyright (C) 2015 by the Battle for Wesnoth Project
   <http://www.wesnoth.org/>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "quit_confirmation.hpp"
#include "game_end_exceptions.hpp"
#include "gettext.hpp"
#include "video.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include "resources.hpp"

std::vector<quit_confirmation*> quit_confirmation::blockers_ = std::vector<quit_confirmation*>();
bool quit_confirmation::open_ = false;

void quit_confirmation::quit(const bool full_exit)
{
	if(!open_)
	{
		open_ = true;
		BOOST_REVERSE_FOREACH(quit_confirmation* blocker, blockers_)
		{
			if(!blocker->prompt_()) {
				open_ = false;
				return;
			}
		}
		open_ = false;
	}

	if(!full_exit) {
		throw_quit_game_exception();
	}

	throw CVideo::quit();
}

bool quit_confirmation::default_prompt()
{
	return gui2::show_message(CVideo::get_singleton(), _("Quit"), _("Do you really want to quit?"),
		gui2::tmessage::yes_no_buttons) != gui2::twindow::CANCEL;
}
