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
#include "game_events/pump.hpp"
#include "play_controller.hpp"
#include "display.hpp"
#include "units/unit.hpp"
#include "units/udisplay.hpp"
#include "units/map.hpp"
#include "whiteboard/manager.hpp"
#include "actions/vision.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include <boost/range/adaptor/reversed.hpp>

bool surrender_confirmation::open_ = false;
std::vector<surrender_confirmation*> surrender_confirmation::blockers_ = std::vector<surrender_confirmation*>();

bool surrender_confirmation::surrender()
{
	bool doSurrender = false;


	if(!open_) {
		open_ = true;
		for(surrender_confirmation* blocker : boost::adaptors::reverse(blockers_))
		{
			if(blocker->prompt_()) {
				doSurrender=true;
			} else {
				doSurrender=false;
			}
		}
		open_ = false;

		if(doSurrender)
		{
			display& disp = resources::controller->get_display();
			unit_map& units = resources::gameboard->units();
			for(unit_map::iterator i = units.begin(); i != units.end(); ++i) {
				if(i->side() == disp.viewing_side()) {
						const map_location loc = (*i).get_location();
						const int dying_side = i->side();
						resources::controller->pump().fire("last_breath", loc, loc);
						if (i.valid()) {
							unit_display::unit_die(loc, *i);
						}
						resources::screen->redraw_minimap();
						if (i.valid()) {
							i->set_hitpoints(0);
						}
						resources::controller->pump().fire("die", loc, loc);
						if (i.valid()) {
							resources::gameboard->units().erase(i);
						}
						resources::whiteboard->on_kill_unit();
						actions::recalculate_fog(dying_side);
				}
			}
		}
	}

	return doSurrender;
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
