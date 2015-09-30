/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * An extension of play_controller::hotkey_handler, which has support for
 * replay controller features
 */

#ifndef HOTKEY_HANDLER_REPLAY_HPP_INCL_
#define HOTKEY_HANDLER_REPLAY_HPP_INCL_

#include "replay_controller.hpp"
#include "hotkey_handler.hpp"
#include "global.hpp"

class replay_controller::hotkey_handler : public play_controller::hotkey_handler {

protected:
	replay_controller & replay_controller_;

public:
	hotkey_handler(replay_controller &, saved_game &);
	~hotkey_handler();

	virtual void preferences();
	virtual void show_statistics();
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const;
	
	virtual void reset_replay() OVERRIDE
	{ return replay_controller_.reset_replay(); }
	virtual void stop_replay() OVERRIDE
	{ return replay_controller_.stop_replay(); }
	virtual void play_replay() OVERRIDE
	{ return replay_controller_.play_replay(); }
	virtual void replay_next_turn() OVERRIDE
	{ return replay_controller_.replay_next_turn(); }
	virtual void replay_next_side() OVERRIDE
	{ return replay_controller_.replay_next_side(); }
	virtual void replay_next_move() OVERRIDE
	{ return replay_controller_.replay_next_move(); }
	virtual void replay_show_everything() OVERRIDE
	{ return replay_controller_.replay_show_everything(); }
	virtual void replay_show_each() OVERRIDE
	{ return replay_controller_.replay_show_each(); }
	virtual void replay_show_team1() OVERRIDE
	{ return replay_controller_.replay_show_team1(); }

};

#endif
