/*
	Copyright (C) 2003 - 2025
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

#include <string>

class game_lua_kernel;
class variable_set;

namespace game_events
{
struct queued_event;
/** Represents a single filter condition on an event. */
struct event_filter {
	/** Runs the filter and returns whether it passes on the given event. */
	virtual bool operator()(const queued_event& event_info) const = 0;
	/** Serializes the filter into a config, if possible. */
	virtual void serialize(config& cfg) const;
	/** Returns true if it is possible to serialize the filter into a config. */
	virtual bool can_serialize() const;
	virtual ~event_filter() = default;
	event_filter() = default;
private:
	event_filter(const event_filter&) = delete;
	event_filter& operator=(const event_filter&) = delete;
};

class event_handler
{
public:
	event_handler(const std::string& types, const std::string& id = "");

	std::vector<std::string> names(const variable_set* vars) const;
	const std::string& names_raw() const
	{
		return types_;
	}

	bool disabled() const
	{
		return disabled_;
	}

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
	 * @param[in]     lk The lua kernel to run the WML command.
	 */
	void handle_event(const queued_event& event_info, game_lua_kernel& lk);

	bool filter_event(const queued_event& event_info) const;

	const config& arguments() const
	{
		return args_;
	}

	const std::string& id() const
	{
		return id_;
	}

	const double& priority() const
	{
		return priority_;
	}

	bool empty() const;

	bool repeatable() const
	{
		return !first_time_only_;
	}

	// Normally non-serializable events are skipped when serializing (with a warning).
	// If include_nonserializable is true, the game attempts to serialize them anyway.
	// This will produce output that kind of looks like the event but would not deserialize to the same event.
	void write_config(config& cfg, bool include_nonserializable = false) const;

	void set_repeatable(bool repeat = true)
	{
		first_time_only_ = !repeat;
	}

	void set_priority(double priority)
	{
		priority_ = priority;
	}
	void set_menu_item(bool imi)
	{
		is_menu_item_ = imi;
	}

	void set_arguments(const config& cfg)
	{
		args_ = cfg;
	}

	void read_filters(const config& cfg);
	void add_filter(std::unique_ptr<event_filter>&& filter);

	void register_wml_event(game_lua_kernel& lk);
	void set_event_ref(int idx, bool has_preloaded);

private:
	bool first_time_only_;
	bool is_menu_item_;
	bool disabled_;
	/**
	 * Tracks whether the event was registered from the Lua API.
	 * This allows a warning to be issued in cases that will break saved games.
	 */
	bool is_lua_;
	/**
	 * Tracks whether the event was registered before or after the Lua preload event fired.
	 * This allows a warning to be issued in cases that will break saved games.
	 *
	 * Rationale: Events where the filter or action is a Lua function cannot be serialized.
	 * Therefore, if a saved game relies on it being serialized, it will fail.
	 * Events registered during or before preload do not need to be serialized, because when
	 * a saved game is loaded, the preload event re-triggers and re-registers the event.
	 * This is actually a common use-case for the Lua events API.
	 * So, this flag allows avoiding false positives in the warning message.
	 */
	bool has_preloaded_;
	int event_ref_;
	double priority_;
	config args_;
	std::vector<std::shared_ptr<event_filter>> filters_;
	std::string id_, types_;
};

}
