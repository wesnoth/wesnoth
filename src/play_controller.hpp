/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAY_CONTROLLER_H_INCLUDED
#define PLAY_CONTROLLER_H_INCLUDED

#include "global.hpp"

#include "display.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "halo.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "key.hpp"
#include "playlevel.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "tooltips.hpp"
#include "unit_types.hpp"

#include <vector>

class play_controller
{
public:
	play_controller(const config& level, const game_data& gameinfo, game_state& state_of_game, 
		int ticks, int num_turns, const config& game_config, CVideo& video);
	~play_controller();

	const int get_xp_modifier();

protected:
	virtual void init(CVideo& video);
	void init_managers();

	//managers
	const verification_manager verify_manager_;
	teams_manager team_manager_;
	preferences::display_manager* prefs_disp_manager_;
	tooltips::manager* tooltips_manager_;
	game_events::manager* events_manager_;
	halo::manager* halo_manager_;
	font::floating_label_context labels_manager_;
	help::help_manager help_manager_;

	display* gui_;

	std::vector<team> teams_;
	const game_data& gameinfo_;
	const config& level_;
	const config& game_config_;
	game_state& gamestate_;
	gamestatus status_;
	gamemap map_;
	unit_map units_;
	const statistics::scenario_context statistics_context_;

	CKey key_;
	const int ticks_;
	const int xp_modifier_;
	int first_human_team_;
};


#endif
