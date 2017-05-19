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

#include "game_events/manager.hpp"

#include "game_events/handlers.hpp"
#include "game_events/manager_impl.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/pump.hpp"

#include "filter_context.hpp"
#include "formula/string_utils.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "reports.hpp"
#include "resources.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "serialization/string_utils.hpp"
#include "soundsource.hpp"

#include <iostream>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_event_handler("event_handler");
#define DBG_EH LOG_STREAM(debug, log_event_handler)

namespace game_events
{
/** Create an event handler. */
void manager::add_event_handler(const config& handler, bool is_menu_item)
{
	event_handlers_->add_event_handler(handler, *this, is_menu_item);
}

/** Removes an event handler. */
void manager::remove_event_handler(const std::string& id)
{
	event_handlers_->remove_event_handler(id);
}

/** Gets an event handler by id */
const handler_ptr manager::get_event_handler_by_id(const std::string& id)
{
	return event_handlers_->get_event_handler_by_id(id);
}

/* ** manager ** */

manager::manager()
	: event_handlers_(new event_handlers())
	, unit_wml_ids_()
	, pump_(new game_events::wml_event_pump(*this))
	, wml_menu_items_()
{
}

void manager::read_scenario(const config& scenario_cfg)
{
	for(const config& ev : scenario_cfg.child_range("event")) {
		add_event_handler(ev);
	}

	for(const std::string& id : utils::split(scenario_cfg["unit_wml_ids"])) {
		unit_wml_ids_.insert(id);
	}

	wml_menu_items_.set_menu_items(scenario_cfg);

	// Create the event handlers for menu items.
	wml_menu_items_.init_handlers();
}

manager::~manager()
{
}

/* ** manager::iteration ** */

/**
 * Event-specific constructor.
 * This iteration will go through all event handlers matching the given name
 * (including those defined via menu items).
 * An empty @a event_name will automatically match nothing.
 */
manager::iteration::iteration(const std::string& event_name, manager& man)
	: main_list_(man.event_handlers_->get(event_name))
	, var_list_(man.event_handlers_->get())
	, event_name_(event_name)
	, end_(man.event_handlers_->size())
	, current_is_known_(false)
	, main_is_current_(false)
	, main_it_(main_list_.begin())
	, var_it_(event_name.empty() ? var_list_.end() : var_list_.begin())
	, gamedata_(resources::gamedata)
{
}

/**
 * Increment
 * Incrementing guarantees that the next dereference will differ from the
 * previous derference (unless the iteration is exhausted). However, multiple
 * increments between dereferences are allowed to have the same effect as a
 * single increment.
 */
manager::iteration& manager::iteration::operator++()
{
	if(!current_is_known_) {
		// Either *this has never been dereferenced, or we already incremented
		// since the last dereference. We are allowed to ignore this increment.
		return *this;
	}

	// Guarantee a different element next dereference.
	if(main_is_current_) {
		++main_it_;
	} else {
		++var_it_; // (We'll check for a name match when we dereference.)
	}

	// We no longer know which list is current.
	current_is_known_ = false;

	// Done.
	return *this;
}

/**
 * Dereference
 * Will return a null pointer when the end of the iteration is reached.
 */
handler_ptr manager::iteration::operator*()
{
	// Get the candidate for the current element from the main list.
	handler_ptr main_ptr = *main_it_;
	handler_vec::size_type main_index = ptr_index(main_ptr);

	// Get the candidate for the current element from the var list.
	handler_ptr var_ptr = *var_it_;

	// (Loop while var_ptr would be chosen over main_ptr, but the name does not match.)
	while(var_ptr && var_ptr->index() < main_index && !var_ptr->matches_name(event_name_, gamedata_)) {
		var_ptr = *++var_it_;
	}

	handler_vec::size_type var_index = ptr_index(var_ptr);

	// Which list? (Index ties go to the main list.)
	current_is_known_ = main_index < end_ || var_index < end_;
	main_is_current_ = main_index <= var_index;

	if(!current_is_known_) {
		return handler_ptr(); // End of list; return a null pointer.
	}

	return main_is_current_ ? main_ptr : var_ptr;
}

void manager::add_events(const config::const_child_itors& cfgs, const std::string& type)
{
	if(!type.empty()) {
		if(std::find(unit_wml_ids_.begin(), unit_wml_ids_.end(), type) != unit_wml_ids_.end()) {
			return;
		}

		unit_wml_ids_.insert(type);
	}

	for(const config& new_ev : cfgs) {
		if(type.empty() && new_ev["id"].empty()) {
			WRN_NG << "attempt to add an [event] with empty id=, ignoring " << std::endl;
			continue;
		}

		add_event_handler(new_ev);
	}
}

void manager::write_events(config& cfg) const
{
	for(const handler_ptr& eh : *event_handlers_) {
		if(!eh || eh->is_menu_item()) {
			continue;
		}

		cfg.add_child("event", eh->get_config());
	}

	cfg["unit_wml_ids"] = utils::join(unit_wml_ids_);
	wml_menu_items_.to_config(cfg);
}

game_events::wml_event_pump& manager::pump()
{
	return *pump_;
}

} // end namespace game_events
