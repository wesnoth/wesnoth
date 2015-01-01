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

class replay_controller::hotkey_handler : public play_controller::hotkey_handler {

protected:
	replay_controller & replay_controller_;

public:
	hotkey_handler(replay_controller &, saved_game &);
	~hotkey_handler();

	virtual void preferences();
	virtual void show_statistics();
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const;
};

#endif
