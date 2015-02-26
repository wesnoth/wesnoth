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

struct set_completion
{
	set_completion(saved_game& state, const std::string& completion) :
		state_(state), completion_(completion)
	{
	}
	~set_completion()
	{
		state_.classification().completion = completion_;
	}
	private:
	saved_game& state_;
	const std::string completion_;
};

class playsingle_controller : public play_controller
{
public:
	playsingle_controller(const config& level, saved_game& state_of_game,
		const int ticks, const config& game_config, const tdata_cache & tdata, CVideo& video, bool skip_replay);
	virtual ~playsingle_controller();

	LEVEL_RESULT play_scenario(const config::const_child_itors &story,
		bool skip_replay);
	boost::optional<LEVEL_RESULT> play_scenario_init(end_level_data & eld, bool & past_prestart );
	LEVEL_RESULT play_scenario_main_loop(end_level_data & eld, bool & past_prestart );

	virtual void handle_generic_event(const std::string& name);

	virtual void force_end_level(LEVEL_RESULT res)
	{ level_result_ = res; }
	virtual void check_end_level();
	void report_victory(std::ostringstream &report, team& t, int finishing_bonus_per_turn,
			int turns_left, int finishing_bonus);
	virtual void on_not_observer() {}
	bool is_host() const ;
	virtual void maybe_linger();

	void end_turn();
	void force_end_turn();

	class hotkey_handler;
	virtual bool is_end_turn() const { return end_turn_; }

protected:
	possible_end_play_signal play_turn();
	virtual possible_end_play_signal play_side();
	virtual void before_human_turn();
	void show_turn_dialog();
	void execute_gotos();
	virtual possible_end_play_signal play_human_turn();
	virtual void after_human_turn();
	void end_turn_enable(bool enable);
	void play_ai_turn();
	virtual possible_end_play_signal play_idle_loop();
	virtual void do_idle_notification();
	virtual possible_end_play_signal play_network_turn();
	virtual void init_gui();
	possible_end_play_signal check_time_over();
	void store_recalls();
	void store_gold(bool obs = false);

	const cursor::setter cursor_setter;
	gui::floating_textbox textbox_info_;

	replay_network_sender replay_sender_;
	playturn_network_adapter network_reader_;
	turn_info turn_data_;
	bool end_turn_;
	bool player_type_changed_;
	bool replaying_;
	bool skip_next_turn_;
	bool do_autosaves_;
	LEVEL_RESULT level_result_;
	void linger();
};


#endif
