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

#include "game_events/manager_impl.hpp"

#include "game_events/handlers.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/pump.hpp"

#include "formula/string_utils.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "reports.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "serialization/string_utils.hpp"

#include <boost/algorithm/string.hpp>

#include <iostream>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_event_handler("event_handler");
#define DBG_EH LOG_STREAM(debug, log_event_handler)

namespace game_events
{
void event_handlers::log_handlers()
{
	if(lg::debug().dont_log("event_handler")) {
		return;
	}

	std::stringstream ss;

	for(const handler_ptr& h : active_) {
		if(!h) {
			continue;
		}

		const config& cfg = h->get_config();
		ss << "name=" << cfg["name"] << ", with id=" << cfg["id"] << "; ";
	}

	DBG_EH << "active handlers are now " << ss.str() << "\n";
}

/**
 * Utility to standardize the event names used in by_name_.
 * This means stripping leading and trailing spaces, and converting internal
 * spaces to underscores.
 */
std::string event_handlers::standardize_name(const std::string& name)
{
	std::string retval = name;

	// Trim leading and trailing spaces.
	boost::trim(retval);

	// Replace internal spaces with underscores.
	boost::replace_all(retval, " ", "_");

	return retval;
}

/**
 * Read-only access to the handlers with fixed event names, by event name.
 */
const handler_list& event_handlers::get(const std::string& name) const
{
	// Empty list for the "not found" case.
	static const handler_list empty_list;

	// Look for the name in the name map.
	auto find_it = by_name_.find(standardize_name(name));
	return find_it == by_name_.end() ? empty_list : find_it->second;
}

/**
 * Adds an event handler.
 * An event with a nonempty ID will not be added if an event with that
 * ID already exists.
 */
void event_handlers::add_event_handler(const config& cfg, manager& man, bool is_menu_item)
{
	const std::string name = cfg["name"];
	std::string id = cfg["id"];

	if(!id.empty()) {
		// Ignore this handler if there is already one with this ID.
		auto find_it = id_map_.find(id);
		if(find_it != id_map_.end() && !find_it->second.expired()) {
			DBG_EH << "ignoring event handler for name='" << name << "' with id '" << id << "'\n";
			return;
		}
	}

	// Create a new handler.
	DBG_EH << "inserting event handler for name=" << name << " with id=" << id << "\n";
	handler_ptr new_handler(new event_handler(cfg, is_menu_item, active_.size(), man));

	active_.push_back(new_handler);

	// File by name.
	if(utils::might_contain_variables(name)) {
		dynamic_.push_back(new_handler);
	} else {
		for(const std::string& single_name : utils::split(name)) {
			by_name_[standardize_name(single_name)].push_back(new_handler);
		}
	}

	// File by ID.
	if(!id.empty()) {
		id_map_[id] = new_handler;
	}

	log_handlers();
}

/**
 * Removes an event handler, identified by its ID.
 * Events with empty IDs cannot be removed.
 */
void event_handlers::remove_event_handler(const std::string& id)
{
	if(id.empty()) {
		return;
	}

	DBG_EH << "removing event handler with id " << id << "\n";

	// Find the existing handler with this ID.
	auto find_it = id_map_.find(id);
	if(find_it != id_map_.end()) {
		handler_ptr handler = find_it->second.lock();

		// Remove handler.
		if(handler) {
			handler->disable();
		}

		// Do this even if the lock failed.
		// The index by name will self-adjust later. No need to adjust it now.
		id_map_.erase(find_it);
	}

	log_handlers();
}

const handler_ptr event_handlers::get_event_handler_by_id(const std::string& id)
{
	auto find_it = id_map_.find(id);
	if(find_it != id_map_.end() && !find_it->second.expired()) {
		return find_it->second.lock();
	}

	return nullptr;
}

} // end namespace game_events
