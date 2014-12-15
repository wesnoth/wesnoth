/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "game_events/manager.hpp"

#include "handlers.hpp"
#include "manager_impl.hpp"
#include "menu_item.hpp"
#include "pump.hpp"

#include "filter_context.hpp"
#include "formula_string_utils.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "reports.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "serialization/string_utils.hpp"
#include "soundsource.hpp"
#include "util.hpp"

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>


static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_event_handler("event_handler");
#define DBG_EH LOG_STREAM(debug, log_event_handler)

namespace game_events {

t_context::t_context(game_lua_kernel * lk, filter_context * fc, game_display * sc, game_data * gd, unit_map * um, boost::function<void()> wb_callback, boost::function<int()> current_side_accessor)
	: lua_kernel(lk)
	, filter_con(fc)
	, screen(sc)
	, gamedata(gd)
	, units(um)
	, on_gamestate_change(wb_callback)
	, current_side(current_side_accessor)
{}

/** Create an event handler. */
void manager::add_event_handler(const config & handler, bool is_menu_item)
{
	event_handlers_->add_event_handler(handler, *this, is_menu_item);
}

/**
 * Checks if an item has been used.
 * (An empty id will never be considered used.)
 */
bool manager::item_used(const std::string & id)
{
	return !id.empty()  &&  used_items_.count(id) > 0;
}

/** Records if an item has been used. */
void manager::item_used(const std::string & id, bool used)
{
	// Empty IDs are not tracked.
	if ( id.empty() )
		return;

	if ( used )
		used_items_.insert(id);
	else
		used_items_.erase(id);
}

/** Removes an event handler. */
void manager::remove_event_handler(const std::string & id)
{
	event_handlers_->remove_event_handler(id);
}


/* ** manager ** */

manager::manager(const config& cfg, const t_context & res)
	: event_handlers_(new t_event_handlers())
	, unit_wml_ids_()
	, used_items_()
	, pump_(new game_events::t_pump(*this, res))
	, resources_(res)
{
	BOOST_FOREACH(const config &ev, cfg.child_range("event")) {
		add_event_handler(ev);
	}
	BOOST_FOREACH(const std::string &id, utils::split(cfg["unit_wml_ids"])) {
		unit_wml_ids_.insert(id);
	}

	// Guard against a memory leak (now) / memory corruption (when this is deleted).
	// This is why creating multiple manager objects is prohibited.
	assert(resources_.lua_kernel != NULL);

	wml_action::map::const_iterator action_end = wml_action::end();
	wml_action::map::const_iterator action_cur = wml_action::begin();
	for ( ; action_cur != action_end; ++action_cur ) {
		resources_.lua_kernel->set_wml_action(action_cur->first, action_cur->second);
	}

	const std::string used = cfg["used_items"];
	if(!used.empty()) {
		const std::vector<std::string>& v = utils::split(used);
		for(std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
			item_used(*i, true);
		}
	}

	// Create the event handlers for menu items.
	resources_.gamedata->get_wml_menu_items().init_handlers();
}

manager::~manager() {}

/* ** manager::iteration ** */

/**
 * Event-specific constructor.
 * This iteration will go through all event handlers matching the given name
 * (including those defined via menu items).
 * An empty @a event_name will automatically match nothing.
 */
manager::iteration::iteration(const std::string & event_name, manager & man) :
	main_list_(man.event_handlers_->get(event_name)),
	var_list_(man.event_handlers_->get()),
	event_name_(event_name),
	end_(man.event_handlers_->size()),
	current_is_known_(false),
	main_is_current_(false),
	main_it_(main_list_.begin()),
	var_it_(event_name.empty() ? var_list_.end() : var_list_.begin())
{
}

/**
 * Increment
 * Incrementing guarantees that the next dereference will differ from the
 * previous derference (unless the iteration is exhausted). However, multiple
 * increments between dereferences are allowed to have the same effect as a
 * single increment.
 */
manager::iteration & manager::iteration::operator++()
{
	if ( !current_is_known_ )
		// Either *this has never been dereferenced, or we already incremented
		// since the last dereference. We are allowed to ignore this increment.
		return *this;

	// Guarantee a different element next dereference.
	if ( main_is_current_ )
		++main_it_;
	else
		++var_it_; // (We'll check for a name match when we dereference.)

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
	while ( var_ptr  &&  var_ptr->index() < main_index  &&
	        !var_ptr->matches_name(event_name_) )
		var_ptr = *++var_it_;
	handler_vec::size_type var_index = ptr_index(var_ptr);

	// Which list? (Index ties go to the main list.)
	current_is_known_ = main_index < end_  ||  var_index < end_;
	main_is_current_ = main_index <= var_index;

	if ( !current_is_known_ )
		return handler_ptr(); // End of list; return a null pointer.
	else
		return main_is_current_ ? main_ptr : var_ptr;
}


void manager::add_events(const config::const_child_itors &cfgs, const std::string& type)
{
	if(!type.empty()) {
		if(std::find(unit_wml_ids_.begin(),unit_wml_ids_.end(),type) != unit_wml_ids_.end()) return;
		unit_wml_ids_.insert(type);
	}
	BOOST_FOREACH(const config &new_ev, cfgs) {
		if(type.empty() && new_ev["id"].empty())
		{
			WRN_NG << "attempt to add an [event] with empty id=, ignoring " << std::endl;
			continue;
		}
		add_event_handler(new_ev);
	}
}

void manager::write_events(config& cfg)
{
	BOOST_FOREACH(const handler_ptr &eh, *event_handlers_) {
		if ( !eh || eh->is_menu_item() ) {
			continue;
		}
		cfg.add_child("event", eh->get_config());
	}

	cfg["used_items"] = utils::join(used_items_);
	cfg["unit_wml_ids"] = utils::join(unit_wml_ids_);
}

game_events::t_pump & manager::pump()
{
	return *pump_;
}

} //end namespace game_events
