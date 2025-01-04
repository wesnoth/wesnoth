/*
	Copyright (C) 2015 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "video.hpp" // only for video::quit
#include "resources.hpp"
#include "playmp_controller.hpp"
#include "gui/dialogs/surrender_quit.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/retval.hpp"
#include "utils/ranges.hpp"

bool quit_confirmation::quit()
{
	if(!open_) {
		open_ = true;
		for(quit_confirmation* blocker : blockers_ | utils::views::reverse)
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
	if(quit()) { throw video::quit(); }
}

bool quit_confirmation::show_prompt(const std::string& message)
{
	return gui2::show_message(_("Quit"), message,
		gui2::dialogs::message::yes_no_buttons) != gui2::retval::CANCEL;
}

bool quit_confirmation::default_prompt()
{
	playmp_controller* pmc = dynamic_cast<playmp_controller*>(resources::controller);
	std::size_t humans_notme_cnt = 0;

	if(pmc != nullptr) {
		for(const auto& t : pmc->get_teams()) {
			if(t.is_network_human()) {
				++humans_notme_cnt;
			}
		}
	}

	if(!(pmc == nullptr || humans_notme_cnt < 1 || pmc->is_linger_mode() || pmc->is_observer())) {
		gui2::dialogs::surrender_quit sq;
		sq.show();
		int retval = sq.get_retval();
		if(retval == 1)
		{
			pmc->surrender(display::get_singleton()->viewing_team_index());
			return true;
		}
		else if(retval == 2)
		{
			return true;
		}
		else
		{
			return false;
		}
	} else {
		return show_prompt(_("Do you really want to quit?"));
	}
}
