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

#include "game_events/manager.hpp"

#include "game_events/handlers.hpp"
#include "game_events/manager_impl.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/pump.hpp"

#include "formula/string_utils.hpp"
#include "game_data.hpp"
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
		if(eh && !eh->is_menu_item() && !eh->disabled()) {
			cfg.add_child("event", eh->get_config());;
		}
	}

	cfg["unit_wml_ids"] = utils::join(unit_wml_ids_);
	wml_menu_items_.to_config(cfg);
}

void manager::execute_on_events(const std::string& event_id, manager::event_func_t func)
{
	const std::string standardized_event_id = event_handlers::standardize_name(event_id);
	const game_data* gd = resources::gamedata;
	auto& active_handlers = event_handlers_->get_active();

	// Save the end outside the loop so the end point remains constant,
	// even if new events are added to the queue.
	const unsigned saved_end = active_handlers.size();

	// It's possible for this function to call itself again. If list cleanup occurred
	// after each recursive call, the saved_end variable in the call above it would no
	// longer be a valid end index. To work around that, we delay cleanup until the
	// toplevel call is done running through the events (ie, when recursion_depth is 0).
	static unsigned recursion_depth = 0;
	++recursion_depth;

	for(unsigned i = 0; i < saved_end; ++i) {
		handler_ptr handler = nullptr;

		try {
			handler = active_handlers.at(i);
		} catch(const std::out_of_range&) {
			continue;
		}

		// Shouldn't happen, but we're just being safe.
		if(!handler || handler->disabled()) {
			continue;
		}

		// Could be more than one.
		for(const std::string& name : handler->names()) {
			bool matches = false;

			if(utils::might_contain_variables(name)) {
				// If we don't have gamedata, we can't interpolate variables, so there's
				// no way the name will match. Move on to the next one in that case.
				if(!gd) {
					continue;
				}

				matches = standardized_event_id ==
					event_handlers::standardize_name(utils::interpolate_variables_into_string(name, *gd));
			} else {
				matches = standardized_event_id == name;
			}

			if(matches) {
				func(*this, handler);
				break;
			}
		}
	}

	--recursion_depth;

	// Clean up expired ptrs. This saves us effort later since it ensures every ptr is valid.
	if(recursion_depth == 0) {
		event_handlers_->clean_up_expired_handlers(standardized_event_id);
	}
}

game_events::wml_event_pump& manager::pump()
{
	return *pump_;
}

} // end namespace game_events
