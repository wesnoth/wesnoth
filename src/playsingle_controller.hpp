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

#ifndef PLAYSINGLE_CONTROLLER_H_INCLUDED
#define PLAYSINGLE_CONTROLLER_H_INCLUDED

#include "play_controller.hpp"

#include "cursor.hpp"
#include "playturn_network_adapter.hpp"
#include "playturn.hpp"
#include "replay.hpp"
#include "saved_game.hpp"
class team;

class playsingle_controller : public play_controller
{
public:
	playsingle_controller(const config& level, saved_game& state_of_game,
		const int ticks, const config& game_config, const tdata_cache & tdata, CVideo& video, bool skip_replay);
	virtual ~playsingle_controller();

	LEVEL_RESULT play_scenario(const config::const_child_itors &story);
	void play_scenario_init();
	void play_scenario_main_loop();

	virtual void handle_generic_event(const std::string& name);

	virtual void check_objectives();
	void report_victory(std::ostringstream &report, team& t, int finishing_bonus_per_turn,
			int turns_left, int finishing_bonus);
	virtual void on_not_observer() {}
	bool is_host() const ;
	virtual void maybe_linger();

	void end_turn();
	void force_end_turn();

	class hotkey_handler;
	std::string describe_result() const;
	
	bool get_player_type_changed() const { return player_type_changed_; }
	void set_player_type_changed() { player_type_changed_ = true; }
	virtual bool should_return_to_play_side()
	{ return player_type_changed_ || end_turn_ != END_TURN_NONE || is_regular_game_end(); }
protected:
	virtual void play_side_impl();
	void before_human_turn();
	void show_turn_dialog();
	void execute_gotos();
	virtual void play_human_turn();
	virtual void after_human_turn();
	void end_turn_enable(bool enable);
	void play_ai_turn();
	virtual void play_idle_loop();
	virtual void do_idle_notification();
	virtual void play_network_turn();
	virtual void init_gui();
	void store_recalls();
	void store_gold(bool obs = false);

	const cursor::setter cursor_setter;
	gui::floating_textbox textbox_info_;

	replay_network_sender replay_sender_;
	playturn_network_adapter network_reader_;
	turn_info turn_data_;
	enum END_TURN_STATE
	{
		/// The turn was not ended yet
		END_TURN_NONE,
		/// And endturn was required eigher by the player, by the ai or by [end_turn]
		END_TURN_REQUIRED,
		/// An [end_turn] was added to the replay.
		END_TURN_SYNCED,
	};
	END_TURN_STATE end_turn_;
	bool skip_next_turn_;
	void linger();
	void sync_end_turn();
};


#endif
