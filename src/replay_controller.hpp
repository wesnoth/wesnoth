/* $Id: replay_controller.hpp 7396 2005-07-02 21:37:20Z ott $ */
/*
   Copyright (C) 2006 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef REPLAY_CONTROLLER_H_INCLUDED
#define REPLAY_CONTROLLER_H_INCLUDED

#include "display.hpp"
#include "font.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "halo.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "play_controller.hpp"
#include "preferences_display.hpp"
#include "tooltips.hpp"
#include "wml_separators.hpp"

#include <vector>

class replay_controller : public play_controller
{
public:
	replay_controller(const config& level, const game_data& gameinfo, game_state& state_of_game,
		const int ticks, const int num_turns, const config& game_config, CVideo& video);
	~replay_controller();

	virtual bool can_execute_command(hotkey::HOTKEY_COMMAND command, int index=-1) const;

	std::vector<team>& get_teams();
	unit_map get_units();
	gamemap& get_map();

	//event handlers
	virtual void preferences();
	virtual void show_statistics();
	void play_replay();
	void reset_replay();
	void stop_replay();
	void replay_next_turn();
	void replay_next_side();
	void replay_switch_fog();
	void replay_switch_shroud();
	void replay_skip_animation();

	std::vector<team> teams_start_;

protected:
	virtual void init_gui();
	void init_shroudfog_controls(const std::vector<team>::iterator);

private:
	void init();
	virtual void play_turn();
	virtual void play_side(const unsigned int team_index, bool save);
	void update_teams();
	void update_gui();
	void init_replay_display();

	game_state gamestate_start_;
	gamestatus status_start_;
	unit_map units_start_;

	unsigned int current_turn_;
	int delay_;
	bool is_playing_;
};


LEVEL_RESULT play_replay_level(const game_data& gameinfo, const config& terrain_config,
		const config* level, CVideo& video,
		game_state& state_of_game);

#endif
