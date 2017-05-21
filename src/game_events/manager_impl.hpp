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

#include <memory>
#include <unordered_map>

namespace game_events
{
// event_handlers is essentially the implementation details of the manager
class event_handlers
{
private:
	using handler_vec_t = std::vector<handler_ptr>;
	using map_t = std::unordered_map<std::string, handler_list>;
	using id_map_t = std::unordered_map<std::string, weak_handler_ptr>;

	/**
	 * Active event handlers. Will not have elements removed unless the event_handlers is clear()ed.
	 * This is the only container that actually 'owns' any events in the form of shared_ptrs. The other
	 * three storage methods own weak_ptrs.
	 */
	handler_vec_t active_;

	/** Active event handlers with fixed event names, organized by event name. */
	map_t by_name_;

	/** Active event handlers with variables in their event names. */
	handler_list dynamic_;

	/** Allows quick locating of handlers by id. */
	id_map_t id_map_;

	void log_handlers();

	/** Utility to standardize the event names used in by_name_. */
	static std::string standardize_name(const std::string& name);

public:
	event_handlers()
		: active_()
		, by_name_()
		, dynamic_()
		, id_map_()
	{
	}

	/** Access to the handlers with varying event names. */
	handler_list& get_dynamic()
	{
		return dynamic_;
	}

	/** Read-only access to the active event handlers. Essentially gives all events. */
	const handler_vec_t& get_active() const
	{
		return active_;
	}

	/** Access to the handlers with fixed event names, by event name. */
	handler_list& get(const std::string& name);

	/** Adds an event handler. */
	void add_event_handler(const config& cfg, bool is_menu_item = false);

	/** Removes an event handler, identified by its ID. */
	void remove_event_handler(const std::string& id);

	/**
	 * Removes all expired event handlers and any weak_ptrs to them.
	 *
	 * @param event_name      The event name from whose by-name queue to clean
	 *                        up handlers.
	 */
	void clean_up_expired_handlers(const std::string& event_name);

	/** Gets an event handler, identified by its ID. */
	const handler_ptr get_event_handler_by_id(const std::string& id);

	/** The number of active event handlers. */
	size_t size() const
	{
		return active_.size();
	}
};

} // end namespace game_events
