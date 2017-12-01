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

#pragma once

#include "game_events/handlers.hpp"
#include "game_events/wmi_manager.hpp"

#include <functional>
#include <set>
#include <string>

class game_lua_kernel;
class filter_context;
class game_display;
class game_data;
class unit_map;

namespace game_events
{
class wml_event_pump;
class event_handlers;

/**
 * The game event manager loads the scenario configuration object,
 * and ensures that events are handled according to the
 * scenario configuration for its lifetime.
 *
 * Thus, a manager object should be created when a scenario is played,
 * and destroyed at the end of the scenario.
 * If a second manager object is created before destroying the previous
 * one, the game will crash with an assertion failure.
 */
class manager
{
private:
	/**
	 * This class is similar to an input iterator through event handlers,
	 * except each instance knows its own end (determined when constructed).
	 * Subsequent dereferences are not guaranteed to return the same element,
	 * so it is important to assign a dereference to a variable if you want
	 * to use it more than once. On the other hand, a dereference will not
	 * return a null pointer until the end of the iteration is reached (and
	 * this is how to detect the end of the iteration).
	 *
	 * For simplicity, this class is neither assignable nor equality
	 * comparable nor default constructable, and there is no postincrement.
	 * Typedefs are also skipped.
	 */
	class iteration
	{
	public:
		/// Event-specific constructor.
		explicit iteration(const std::string& event_name, manager&);

		// Increment:
		iteration& operator++();
		// Dereference:
		handler_ptr operator*();

	private:
		/// The fixed-name event handlers for this iteration.
		handler_list& main_list_;
		/// The varying-name event handlers for this iteration.
		handler_list& var_list_;
		/// The event name for this iteration.
		const std::string event_name_;

		/// Set to true upon dereferencing.
		bool current_is_known_;
		/// true if the most recent dereference was taken from main_list_.
		bool main_is_current_;
		/// The current (or next) element from main_list_.
		handler_list::iterator main_it_;
		/// The current (or next) element from var_list_.
		handler_list::iterator var_it_;

		game_data* gamedata_;
	};

	const std::unique_ptr<event_handlers> event_handlers_;
	std::set<std::string> unit_wml_ids_;

	const std::unique_ptr<game_events::wml_event_pump> pump_;
	game_events::wmi_manager wml_menu_items_;

public:
	manager(const manager&) = delete;
	manager& operator=(const manager&) = delete;

	explicit manager();
	void read_scenario(const config& scenario_cfg);
	~manager();

	/** Create an event handler. */
	void add_event_handler(const config& handler, bool is_menu_item = false);

	/** Removes an event handler. */
	void remove_event_handler(const std::string& id);

	/** Gets an event handler by ID */
	const handler_ptr get_event_handler_by_id(const std::string& id);

	void add_events(const config::const_child_itors& cfgs, const std::string& type = std::string());

	void write_events(config& cfg) const;

	using event_func_t = std::function<void(game_events::manager&, handler_ptr&)>;
	void execute_on_events(const std::string& event_id, event_func_t func);

	game_events::wml_event_pump& pump();

	game_events::wmi_manager& wml_menu_items()
	{
		return wml_menu_items_;
	}
};
}
