/* $Id$ */
/*
   Copyright (C) 2006 - 2008 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef REPLAY_CONTROLLER_H_INCLUDED
#define REPLAY_CONTROLLER_H_INCLUDED

class video;

#include "play_controller.hpp"

#include <vector>

class replay_controller : public play_controller
{
public:
	replay_controller(const config& level, const game_data& gameinfo, game_state& state_of_game,
		const int ticks, const int num_turns, const config& game_config, CVideo& video);
	~replay_controller();

	virtual bool can_execute_command(hotkey::HOTKEY_COMMAND command, int index=-1) const;

	//event handlers
	virtual void preferences();
	virtual void show_statistics();
	void play_replay();
	void reset_replay();
	void stop_replay();
	void replay_next_turn();
	void replay_next_side();
	void replay_show_everything();
	void replay_show_each();
	void replay_show_team1();
	void replay_skip_animation();

	std::vector<team> teams_start_;

protected:
	virtual void init_gui();

private:
	bool continue_replay();
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

	bool show_everything_;
	unsigned int show_team_;
};


LEVEL_RESULT play_replay_level(const game_data& gameinfo, const config& terrain_config,
		const config* level, CVideo& video,
		game_state& state_of_game);

#endif
