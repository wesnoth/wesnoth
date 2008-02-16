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

#ifndef PLAYMP_CONTROLLER_H_INCLUDED
#define PLAYMP_CONTROLLER_H_INCLUDED

#include "global.hpp"

#include "hotkeys.hpp"
#include "playsingle_controller.hpp"

#include <vector>

class turn_info;

class playmp_controller : public playsingle_controller, public events::pump_monitor
{
public:
	playmp_controller(const config& level, const game_data& gameinfo, game_state& state_of_game,
		const int ticks, const int num_turns, const config& game_config, CVideo& video, 
		bool skip_replay, bool is_host);
	~playmp_controller();

	const bool is_host() { return is_host_; }

	static unsigned int replay_last_turn() { return replay_last_turn_; }
	static void set_replay_last_turn(unsigned int turn);

	bool counting_down();
	void think_about_countdown(int ticks);
	void process(events::pump_info &info);
	void linger(upload_log& log);
	//! Wait for the host to upload the next scenario.
	void wait_for_upload();

protected:
	virtual void handle_generic_event(const std::string& name);

	virtual void speak();
	virtual void whisper();
	virtual void shout();
	virtual void clear_labels();
	virtual bool can_execute_command(hotkey::HOTKEY_COMMAND command, int index=-1) const;

	virtual void play_side(const unsigned int team_index, bool save);
	virtual void before_human_turn(bool save);
	virtual void play_human_turn();
	virtual void after_human_turn();
	virtual void finish_side_turn();
	void play_network_turn();

	turn_info* turn_data_;

	int beep_warning_time_;
private:
	void process_oos(const std::string& err_msg);
	void set_end_scenario_button();
	void reset_end_scenario_button();
	static unsigned int replay_last_turn_;
};

#endif
