/*
   Copyright (C) 2006 - 2014 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
#include "replay.hpp"

class playsingle_controller : public play_controller
{
public:
	playsingle_controller(const config& level, game_state& state_of_game,
		const int ticks, const int num_turns, const config& game_config, CVideo& video, bool skip_replay);
	virtual ~playsingle_controller();

	LEVEL_RESULT play_scenario(const config::const_child_itors &story,
		bool skip_replay);

	virtual void handle_generic_event(const std::string& name);

	virtual void recruit();
	virtual void repeat_recruit();
	virtual void recall();
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const;
	virtual void toggle_shroud_updates();
	virtual void update_shroud_now();
	virtual void end_turn();
	virtual void force_end_turn();
	virtual void rename_unit();
	virtual void create_unit();
	virtual void change_side();
	virtual void kill_unit();
	virtual void label_terrain(bool);
	virtual void clear_labels();
	virtual void continue_move();
	virtual void unit_hold_position();
	virtual void end_unit_turn();
	virtual void user_command();
	virtual void custom_command();
	virtual void ai_formula();
	virtual void clear_messages();
	// Whiteboard hotkeys
	virtual void whiteboard_toggle();
	virtual void whiteboard_execute_action();
	virtual void whiteboard_execute_all_actions();
	virtual void whiteboard_delete_action();
	virtual void whiteboard_bump_up_action();
	virtual void whiteboard_bump_down_action();
	virtual void whiteboard_suppose_dead();
	void linger();

	virtual void force_end_level(LEVEL_RESULT res)
	{ level_result_ = res; }
	virtual void check_end_level();
	void report_victory(std::ostringstream &report, int player_gold,
			int remaining_gold, int finishing_bonus_per_turn,
			int turns_left, int finishing_bonus);
	virtual void on_not_observer() {}

protected:
	virtual void play_turn(bool save);
	virtual void play_side(const unsigned int side_number, bool save);
	virtual void before_human_turn(bool save);
	void show_turn_dialog();
	void execute_gotos();
	virtual void play_human_turn();
	virtual void after_human_turn();
	void end_turn_record();
	void end_turn_record_unlock();
	void end_turn_enable(bool enable);
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command, int index) const;
	void play_ai_turn();
	virtual void play_idle_loop();
	virtual void do_idle_notification();
	virtual void play_network_turn();
	virtual void init_gui();
	void check_time_over();
	void store_recalls();
	void store_gold(bool obs = false);

	const cursor::setter cursor_setter;
	std::deque<config> data_backlog_;
	gui::floating_textbox textbox_info_;

	replay_network_sender replay_sender_;

	bool end_turn_;
	bool player_type_changed_;
	bool replaying_;
	bool turn_over_;
	bool skip_next_turn_;
	LEVEL_RESULT level_result_;
};


#endif
