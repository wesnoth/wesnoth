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

#include "game_events/manager_impl.hpp"

#include "game_events/handlers.hpp"
#include "formula/string_utils.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "utils/general.hpp"

#include <boost/algorithm/string.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_event_handler("event_handler");
#define ERR_EH LOG_STREAM(err, log_event_handler)
#define LOG_EH LOG_STREAM(info, log_event_handler)
#define DBG_EH LOG_STREAM(debug, log_event_handler)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)
#define DBG_WML LOG_STREAM(debug, log_wml)

namespace game_events
{
void event_handlers::log_handlers()
{
	if(lg::debug().dont_log(log_event_handler)) {
		return;
	}

	std::stringstream ss;

	for(const handler_ptr& h : active_) {
		if(!h) {
			continue;
		}

		ss << "name=" << h->names_raw() << ", with id=" << h->id() << "; ";
	}

	DBG_EH << "active handlers are now " << ss.str();
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

bool event_handlers::cmp(const handler_ptr& lhs, const handler_ptr& rhs)
{
	return lhs->priority() < rhs->priority();
}

/**
 * Adds an event handler.
 * An event with a nonempty ID will not be added if an event with that
 * ID already exists.
 */
pending_event_handler event_handlers::add_event_handler(const std::string& name, const std::string& id, bool repeat, double priority, bool is_menu_item)
{
	if(!id.empty()) {
		// Ignore this handler if there is already one with this ID.
		auto find_it = id_map_.find(id);

		if(find_it != id_map_.end() && !find_it->second.expired()) {
			LOG_EH << "ignoring event handler for name='" << name << "' with id '" << id << "' because an event with that id already exists";
			return {*this, nullptr};
		}
	}

	if(name.empty() && id.empty()) {
		static const char* msg = "[event] is missing name or id field";
		lg::log_to_chat() << msg << "\n";
		if(lg::info().dont_log(log_event_handler)) {
			ERR_EH << msg << " (run with --log-info=event_handler for more info)";
		} else {
			ERR_EH << msg;
		}
		return {*this, nullptr};
	}

	// Create a new handler.
	auto handler = std::make_shared<event_handler>(name, id);
	handler->set_menu_item(is_menu_item);
	handler->set_priority(priority);
	handler->set_repeatable(repeat);
	return {*this, handler};
}

void event_handlers::finish_adding_event_handler(const handler_ptr& handler)
{
	// Someone decided to register an empty event... bail.
	if(handler->empty()) {
		return;
	}

	const std::string& names = handler->names_raw();
	const std::string& id = handler->id();

	// Register the new handler.
	// Do note active_ holds the main shared_ptr, and the other three containers
	// construct weak_ptrs from the shared one.
	DBG_EH << "inserting event handler for name=" << names << " with id=" << id;
	active_.emplace_back(handler);

	// File by name.
	if(utils::might_contain_variables(names)) {
		dynamic_.emplace_back(active_.back());
	} else {
		for(const std::string& single_name : handler->names(nullptr)) {
			by_name_[single_name].emplace_back(active_.back());
		}
	}

	// File by ID.
	if(!id.empty()) {
		id_map_[id] = active_.back();
	}

	std::stable_sort(active_.rbegin(), active_.rend(), cmp);
	log_handlers();
}

pending_event_handler::~pending_event_handler()
{
	if(valid()) list_.finish_adding_event_handler(handler_);
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

	DBG_EH << "removing event handler with id " << id;

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
	utils::erase_if(active_, [](const handler_ptr& p) { return p->disabled(); });

	// Then remove any now-unlockable weak_ptrs from the by-name list.
	// Might be more than one so we split.
	for(const std::string& name : utils::split(event_name)) {
		by_name_[standardize_name(name)].remove_if(
			[](const weak_handler_ptr& ptr) { return ptr.expired(); }
		);
	}

	// And finally remove any now-unlockable weak_ptrs from the with-variables name list.
	dynamic_.remove_if(
		[](const weak_handler_ptr& ptr) { return ptr.expired(); }
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
