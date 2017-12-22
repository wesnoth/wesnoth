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

#include "log.hpp"
#include "resources.hpp"
#include "serialization/string_utils.hpp"

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
	event_handlers_->add_event_handler(handler, is_menu_item);
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
	for(const handler_ptr& eh : event_handlers_->get_active()) {
		if(!eh || eh->is_menu_item()) {
			continue;
		}

		cfg.add_child("event", eh->get_config());
	}

	cfg["unit_wml_ids"] = utils::join(unit_wml_ids_);
	wml_menu_items_.to_config(cfg);
}

void manager::execute_on_events(const std::string& event_id, manager::event_func_t func)
{
	// Start with a list of weak_ptrs to all matching event names (ie, "moveto").
	handler_list matching_events = event_handlers_->get(event_id);

	// Then check events with variables in their names for a match.
	for(const auto& handler : event_handlers_->get_dynamic()) {
		if(handler_ptr ptr = handler.lock()) {
			if(ptr->matches_name(event_id, resources::gamedata)) {
				matching_events.push_back(handler);
			}
		}
	}

	for(auto& handler : matching_events) {
		if(handler_ptr ptr = handler.lock()) {
			func(*this, ptr);
		}
	}

	// Clean up expired ptrs. This saves us effort later since it ensures every ptr is valid.
	event_handlers_->clean_up_expired_handlers(event_id);
}

game_events::wml_event_pump& manager::pump()
{
	return *pump_;
}

} // end namespace game_events
