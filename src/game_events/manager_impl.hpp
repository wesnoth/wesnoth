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
#include "config.hpp"

#include <deque>
#include <unordered_map>


namespace game_events
{
class event_handlers;
/**
 * Represents a handler that is about to be added to the events manager but is still waiting for some data.
 * The handler will automatically be added when this class is destroyed, unless it has become invalid somehow.
 */
class pending_event_handler
{
	event_handlers& list_;
	handler_ptr handler_;
	pending_event_handler(const pending_event_handler&) = delete;
	pending_event_handler& operator=(const pending_event_handler&) = delete;
	// It's move-constructible, but there's no way to make it move-assignable since it contains a reference...
	pending_event_handler& operator=(pending_event_handler&&) = delete;
public:
	pending_event_handler(event_handlers& list, handler_ptr handler)
		: list_(list)
		, handler_(handler)
	{}
	/** Check if this handler is valid. */
	bool valid() const {return handler_.get();}
	/** Access the event handler. */
	event_handler* operator->() {return handler_.get();}
	event_handler& operator*() {return *handler_;}
	pending_event_handler(pending_event_handler&&) = default;
	~pending_event_handler();
};

// event_handlers is essentially the implementation details of the manager
class event_handlers
{
private:
	using handler_queue_t = std::deque<handler_ptr>;
	using map_t = std::unordered_map<std::string, handler_list>;
	using id_map_t = std::unordered_map<std::string, weak_handler_ptr>;

	/**
	 * Active event handlers. Will not have elements removed unless the event_handlers is clear()ed.
	 * This is the only container that actually 'owns' any events in the form of shared_ptrs. The other
	 * three storage methods own weak_ptrs.
	 */
	handler_queue_t active_;

	/** Active event handlers with fixed event names, organized by event name. */
	map_t by_name_;

	/** Active event handlers with variables in their event names. */
	handler_list dynamic_;

	/** Allows quick locating of handlers by id. */
	id_map_t id_map_;

	void log_handlers();

	friend pending_event_handler;
	void finish_adding_event_handler(const handler_ptr& new_handler);

public:
	/** Utility to standardize the event names used in by_name_. */
	static std::string standardize_name(const std::string& name);

	/** Compare function to sort event handlers by priority. */
	static bool cmp(const handler_ptr& lhs, const handler_ptr& rhs);

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
	const handler_queue_t& get_active() const
	{
		return active_;
	}

	handler_queue_t& get_active()
	{
		return active_;
	}

	/** Adds an event handler. */
	pending_event_handler add_event_handler(const std::string& name, const std::string& id, bool repeat, double priority = 0., bool is_menu_item = false);

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
	std::size_t size() const
	{
		return active_.size();
	}
};

} // end namespace game_events
