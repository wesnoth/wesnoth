/*
   Copyright (C) 2015 - 2017 by the Battle for Wesnoth Project
   <http://www.wesnoth.org/>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "surrender_confirmation.hpp"
#include "gettext.hpp"
#include "video.hpp"
#include "resources.hpp"
#include "game_board.hpp"
#include "play_controller.hpp"
#include "replay_helper.hpp"
#include "synced_commands.hpp"
#include "synced_context.hpp"
#include "display.hpp"
#include "team.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include <boost/range/adaptor/reversed.hpp>

bool surrender_confirmation::open_ = false;
std::vector<surrender_confirmation*> surrender_confirmation::blockers_ = std::vector<surrender_confirmation*>();

bool surrender_confirmation::surrender()
{
	if(!open_) {
		open_ = true;
		for(surrender_confirmation* blocker : boost::adaptors::reverse(blockers_))
		{
			if(!blocker->prompt_()) {
				open_ = false;
				return false;
			} else {
				synced_context::run_and_throw("surrender", replay_helper::get_init_side());
			}
			open_ = false;
			return true;
		}
		open_ = false;
	}

	return true;
}

bool surrender_confirmation::show_prompt(const std::string& message)
{
	return gui2::show_message(CVideo::get_singleton(), _("Surrender"), message,
		gui2::dialogs::message::yes_no_buttons) != gui2::window::CANCEL;
}

bool surrender_confirmation::default_prompt()
{
	return show_prompt(_("Do you really want to surrender?"));
}
