/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "game_events/fwd.hpp"
#include "game_events/wmi_manager.hpp"
#include "config.hpp"

#include <functional>
#include <set>
#include <string>

class game_lua_kernel;

namespace game_events
{
class wml_event_pump;
class event_handlers;
class pending_event_handler;

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
	const std::unique_ptr<event_handlers> event_handlers_;
	std::set<std::string> unit_wml_ids_;

	const std::unique_ptr<game_events::wml_event_pump> pump_;
	game_events::wmi_manager wml_menu_items_;

public:
	manager(const manager&) = delete;
	manager& operator=(const manager&) = delete;

	explicit manager();
	void read_scenario(const config& scenario_cfg, game_lua_kernel& lk);
	~manager();

	/** Create an event handler from an [event] tag. */
	void add_event_handler_from_wml(const config& handler, game_lua_kernel& lk, bool is_menu_item = false);
	/** Create an empty event handler. Expects the caller to finish setting up the event. */
	pending_event_handler add_event_handler_from_lua(const std::string& name, const std::string& id, bool repeat = false, double priority = 0., bool is_menu_item = false);

	/** Removes an event handler. */
	void remove_event_handler(const std::string& id);

	/** Gets an event handler by ID */
	const handler_ptr get_event_handler_by_id(const std::string& id);

	void add_events(const config::const_child_itors& cfgs, game_lua_kernel& lk, const std::string& type = std::string());

	// Normally non-serializable events are skipped when serializing (with a warning).
	// If include_nonserializable is true, the game attempts to serialize them anyway.
	// This will produce output that kind of looks like the event but would not deserialize to the same event.
	void write_events(config& cfg, bool include_nonserializable=false) const;

	using event_func_t = std::function<void(game_events::manager&, handler_ptr&)>;
	void execute_on_events(const std::string& event_id, const event_func_t& func);

	bool is_event_running() const;

	game_events::wml_event_pump& pump();

	game_events::wmi_manager& wml_menu_items()
	{
		return wml_menu_items_;
	}
};
}
