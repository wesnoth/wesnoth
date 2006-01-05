/* $Id$ */
/*
   Copyright (C) 2005 by Rusty Russell <rusty@rustcorp.com.au>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UPLOAD_LOG_H_INCLUDED
#define UPLOAD_LOG_H_INCLUDED
#include "config.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "team.hpp"
#include "thread.hpp"
#include "tstring.hpp"
#include "unit.hpp"

struct upload_log
{
	struct manager {
		manager() { };
		~manager();
	};

	// We only enable logging when playing campaigns.
	upload_log(bool enable);
	~upload_log();

	// User starts a game (may be new campaign or saved).
	void start(game_state &state, const team &team,
			   int team_number, const unit_map &map, const t_string &turn,
			   int num_turns);

	// User finishes a level.
	void defeat(int turn);
	void victory(int turn, int gold);
	void quit(int turn);

	// Argument passed to upload thread.
	struct thread_info {
		threading::thread *t;
		std::string lastfile;
	};

private:
	config &add_game_result(const std::string &str, int turn);
	bool game_finished(config *game);

	static struct thread_info thread_;
	friend struct manager;

	config config_;
	config *game_;
	std::string filename_;
	bool enabled_;
};

namespace upload_log_dialog
{
	// Please please please upload stats?
	void show_beg_dialog(display& disp);
}

#endif // UPLOAD_LOG_H_INCLUDED
