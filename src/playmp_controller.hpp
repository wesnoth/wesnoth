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

#ifndef PLAYMP_CONTROLLER_H_INCLUDED
#define PLAYMP_CONTROLLER_H_INCLUDED

#include "playsingle_controller.hpp"
#include "syncmp_handler.hpp"

class turn_info;

class playmp_controller : public playsingle_controller, public syncmp_handler
{
public:
	playmp_controller(const config& level, saved_game& state_of_game,
		const int ticks, const config& game_config, 
		const tdata_cache & tdata, CVideo& video,
		bool skip_replay, bool blindfold_replay, bool is_host);
	virtual ~playmp_controller();

	static unsigned int replay_last_turn() { return replay_last_turn_; }
	static void set_replay_last_turn(unsigned int turn);

	void maybe_linger();
	void process_oos(const std::string& err_msg) const;

	void pull_remote_choice();
	void send_user_choice();

	class hotkey_handler;
protected:
	virtual void handle_generic_event(const std::string& name);

	void start_network();
	void stop_network();

	virtual void play_side();
	virtual void play_human_turn();
	virtual void play_linger_turn();
	virtual void after_human_turn();
	virtual void finish_side_turn();
	virtual void play_network_turn();
	virtual void do_idle_notification();
	virtual void play_idle_loop();

	void linger();
	/** Wait for the host to upload the next scenario. */
	void wait_for_upload();

	mutable bool network_processing_stopped_;

	virtual void on_not_observer();
	void remove_blindfold();

	blindfold blindfold_;
private:
	void set_end_scenario_button();
	void reset_end_scenario_button();
	void process_network_data();
	static unsigned int replay_last_turn_;
};

#endif
