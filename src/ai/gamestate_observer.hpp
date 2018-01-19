/*
   Copyright (C) 2010 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * A helper class to observe the game state
 */

#pragma once

#include "generic_event.hpp"

namespace ai {

class gamestate_observer : public events::observer {
public:


	gamestate_observer();


	virtual ~gamestate_observer();


	void handle_generic_event(const std::string &event_name);


	/**
	 * Check if the gamestate has changed since last reset
	 * reset is done once on construction, and can be done
	 * by hand via reset() method.
	 * @return has gamestate changed since last reset?
	 */
	bool is_gamestate_changed();

	/**
	 * Reset the counter of gamestate changes.
	 */
	void reset();

private:
	int gamestate_change_counter_;
};

} //of namespace ai
