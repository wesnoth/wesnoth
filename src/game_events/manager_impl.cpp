/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
#include "formula/string_utils.hpp"
#include "log.hpp"
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
handler_list& event_handlers::get(const std::string& name)
{
	// Empty list for the "not found" case.
	static handler_list empty_list;

	// Look for the name in the name map.
	auto find_it = by_name_.find(standardize_name(name));
	return find_it == by_name_.end() ? empty_list : find_it->second;
}

/**
 * Adds an event handler.
 * An event with a nonempty ID will not be added if an event with that
 * ID already exists.
 */
void event_handlers::add_event_handler(const config& cfg, bool is_menu_item)
{
	// Someone decided to register an empty event... bail.
	if(cfg.empty()) {
		return;
	}

	std::string name = cfg["name"];
	std::string id = cfg["id"];

	if(!id.empty()) {
		// Ignore this handler if there is already one with this ID.
		auto find_it = id_map_.find(id);

		if(find_it != id_map_.end() && !find_it->second.expired()) {
			DBG_EH << "ignoring event handler for name='" << name << "' with id '" << id << "'\n";
			return;
		}
	}

	if(name.empty()) {
		lg::wml_error() << "[event] is missing name field\n";
		return;
	}

	// Make a copy of the event cfg here in order to do some standardization on the
	// name field. Will be moved into the handler.
	config event_cfg = cfg;

	// Split the name field...
	std::vector<std::string> standardized_names = utils::split(name);

	// ...and standardize each one individually. This ensures they're all valid for by-name lookup.
	for(std::string& single_name : standardized_names) {
		if(!utils::might_contain_variables(single_name)) {
			single_name = standardize_name(single_name);
		}
	}

	assert(!standardized_names.empty());

	// Write the new name back to the config.
	name = utils::join(standardized_names);
	event_cfg["name"] = name;

	// Create a new handler.
	// Do note active_ holds the main shared_ptr, and the other three containers
	// construct weak_ptrs from the shared one.
	DBG_EH << "inserting event handler for name=" << name << " with id=" << id << "\n";
	active_.emplace_back(new event_handler(std::move(event_cfg), is_menu_item, standardized_names));

	//
	//  !! event_cfg is invalid past this point! DO NOT USE!
	//

	// File by name.
	if(utils::might_contain_variables(name)) {
		dynamic_.emplace_back(active_.back());
	} else {
		for(const std::string& single_name : standardized_names) {
			by_name_[single_name].emplace_back(active_.back());
		}
	}

	// File by ID.
	if(!id.empty()) {
		id_map_[id] = active_.back();
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

		if(handler && !handler->disabled()) {
			handler->disable();
		}

		// Do this even if the lock failed.
		id_map_.erase(find_it);

		// We don't delete the handler from the other lists just yet. This is to ensure
		// the main handler list's size doesn't change when iterating over the handlers.
		// Any disabled handlers don't get executed, and will be removed during the next
		// cleanup pass.
	}

	log_handlers();
}

void event_handlers::clean_up_expired_handlers(const std::string& event_name)
{
	// First, remove all disabled handlers from the main list.
	auto to_remove = std::remove_if(active_.begin(), active_.end(),
		[](handler_ptr p) { return p->disabled(); }
	);

	active_.erase(to_remove, active_.end());

	// Then remove any now-unlockable weak_ptrs from the by-name list.
	// Might be more than one so we split.
	for(const std::string& name : utils::split(event_name)) {
		get(name).remove_if(
			[](weak_handler_ptr ptr) { return ptr.expired(); }
		);
	}

	// And finally remove any now-unlockable weak_ptrs from the with-variables name list.
	dynamic_.remove_if(
		[](weak_handler_ptr ptr) { return ptr.expired(); }
	);
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
