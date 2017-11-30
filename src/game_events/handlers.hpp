/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * Define the handlers for the game's events mechanism.
 *
 * Events might be units moving or fighting, or when victory or defeat occurs.
 * A scenario's configuration file will define actions to take when certain events occur.
 * This module is responsible for tracking these definitions.
 */

#pragma once

#include "config.hpp"

#include <list>
#include <memory>
#include <set>
#include <string>

class game_data;
class game_lua_kernel;

namespace game_events
{
struct queued_event;
class event_handler; // Defined a few lines down.
class manager;

using handler_ptr = std::shared_ptr<event_handler>;
using weak_handler_ptr = std::weak_ptr<event_handler>;
using handler_list = std::list<weak_handler_ptr>;

class event_handler
{
public:
	event_handler(config&& cfg, bool is_menu_item);

	bool disabled() const
	{
		return disabled_;
	}

	bool matches_name(const std::string& name, const game_data* data) const;

	bool is_menu_item() const
	{
		return is_menu_item_;
	}

	/** Flag this handler as disabled. */
	void disable();

	/**
	 * Handles the queued event, according to our WML instructions.
	 *
	 * @param[in]     event_info  Information about the event that needs handling.
	 */
	void handle_event(const queued_event& event_info, game_lua_kernel&);

	const config& get_config() const
	{
		return cfg_;
	}

private:
	bool first_time_only_;
	bool is_menu_item_;
	bool disabled_;
	config cfg_;
};

}
