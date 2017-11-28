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

/// Shared pointer to handler objects.
typedef std::shared_ptr<event_handler> handler_ptr;

/// Storage of event handlers.
typedef std::vector<handler_ptr> handler_vec;

class event_handler
{
public:
	event_handler(const config& cfg, bool is_menu_item, handler_vec::size_type index, manager&);

	/** The index of *this should only be of interest when controlling iterations. */
	handler_vec::size_type index() const
	{
		return index_;
	}

	bool matches_name(const std::string& name, const game_data* data) const;

	bool is_menu_item() const
	{
		return is_menu_item_;
	}

	/** Disables *this, removing it from the game. */
	void disable();
	void handle_event(const queued_event& event_info, handler_ptr& handler_p, game_lua_kernel&);

	const config& get_config() const
	{
		return cfg_;
	}

private:
	bool first_time_only_;
	bool is_menu_item_;
	handler_vec::size_type index_;
	manager* man_;
	config cfg_;
};

using weak_handler_ptr = std::weak_ptr<event_handler>;
using handler_list = std::list<weak_handler_ptr>;
}
