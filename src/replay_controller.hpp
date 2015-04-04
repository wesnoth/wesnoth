/*
   Copyright (C) 2006 - 2015 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef REPLAY_CONTROLLER_H_INCLUDED
#define REPLAY_CONTROLLER_H_INCLUDED

#include "global.hpp"
#include "game_end_exceptions.hpp"
#include "saved_game.hpp"
#include "play_controller.hpp"

#include <vector>

class video;

class replay_controller : public play_controller
{
public:
	replay_controller(const config& level, saved_game& state_of_game,
		const int ticks, const config& game_config, const tdata_cache & tdata, CVideo& video);
	virtual ~replay_controller();

	void play_replay();
	void reset_replay();
	void stop_replay();
	void replay_next_move_or_side(bool one_move);
	void replay_next_turn();
	void replay_next_side();
	void replay_next_move();
	void process_oos(const std::string& msg) const;
	void replay_show_everything();
	void replay_show_each();
	void replay_show_team1();
	void replay_skip_animation();

	virtual void force_end_turn() {}
	virtual void check_objectives() {}
	virtual void on_not_observer() {}

	void try_run_to_completion();

	bool recorder_at_end();

	class hotkey_handler;
	
	virtual bool is_replay() OVERRIDE { return true; }
protected:
	virtual void init_gui();

private:
	void init();
	void play_turn();
	void play_move_or_side(bool one_move = false);
	void play_side();
	void play_move();
	void update_teams();
	void update_gui();
	void init_replay_display();
	void rebuild_replay_theme();
	void handle_generic_event(const std::string& /*name*/);

	void play_replay_main_loop();

	void reset_replay_ui();
	void update_replay_ui();

	void replay_ui_playback_should_start();
	void replay_ui_playback_should_stop();

	gui::button* play_button();
	gui::button* stop_button();
	gui::button* reset_button();
	gui::button* play_turn_button();
	gui::button* play_side_button();
	gui::button* play_move_button();

	bool replay_ui_has_all_buttons() {
		return play_button() && stop_button() && reset_button() &&
		       play_turn_button() && play_side_button();
	}
	game_board gameboard_start_;
	tod_manager tod_manager_start_;
	unsigned int last_replay_action;

	bool is_playing_;

	bool show_everything_;
	unsigned int show_team_;
};


LEVEL_RESULT play_replay_level(const config& game_config, const tdata_cache & tdata, CVideo& video,
		saved_game& state_of_game, bool is_unit_test = false);

#endif
