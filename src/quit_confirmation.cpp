/*
   Copyright (C) 2015 - 2018 by the Battle for Wesnoth Project
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

#include <boost/range/adaptor/reversed.hpp>

std::vector<quit_confirmation*> quit_confirmation::blockers_ = std::vector<quit_confirmation*>();
bool quit_confirmation::open_ = false;

bool quit_confirmation::quit()
{
	if(!open_) {
		open_ = true;
		for(quit_confirmation* blocker : boost::adaptors::reverse(blockers_))
		{
			if(!blocker->prompt_()) {
				open_ = false;
				return false;
			}
		}
		open_ = false;
	}

	return true;
}

void quit_confirmation::quit_to_title()
{
	if(quit()) { throw_quit_game_exception(); }
}

void quit_confirmation::quit_to_desktop()
{
	if(quit()) { throw CVideo::quit(); }
}

bool quit_confirmation::show_prompt(const std::string& message)
{
	return gui2::show_message(_("Quit"), message,
		gui2::dialogs::message::yes_no_buttons) != gui2::window::CANCEL;
}

bool quit_confirmation::default_prompt()
{
	return show_prompt(_("Do you really want to quit?"));
}
