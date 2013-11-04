/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
 * The structure that tracks WML event handlers.
 * (Typically, handlers are defined by [event] tags.)
 */

#include "../global.hpp"
#include "handlers.hpp"
#include "menu_item.hpp"
#include "pump.hpp"

#include "../formula_string_utils.hpp"
#include "../gamestatus.hpp"
#include "../hotkeys.hpp"
#include "../log.hpp"
#include "../preferences.hpp"
#include "../reports.hpp"
#include "../resources.hpp"
#include "../scripting/lua.hpp"
#include "../serialization/string_utils.hpp"
#include "../soundsource.hpp"

#include <boost/foreach.hpp>
#include <iostream>


static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_event_handler("event_handler");
#define DBG_EH LOG_STREAM(debug, log_event_handler)


// This file is in the game_events namespace.
namespace game_events {

namespace { // Types
	typedef std::pair< std::string, config* > wmi_command_change;

	class t_event_handlers {
	public:
		typedef handler_vec::iterator iterator;
		typedef handler_vec::const_iterator const_iterator;

	private:
		handler_vec active_; ///Active event handlers. Will not have elements removed unless the t_event_handlers is clear()ed.


		void log_handlers();

	public:
		typedef handler_vec::size_type size_type;

		t_event_handlers()
			: active_()
		{}

		/// Adds an event handler.
		void add_event_handler(const config & cfg, bool is_menu_item=false);
		/// Removes an event handler, identified by its ID.
		void remove_event_handler(std::string const & id);
		void clear();

		iterator begin() { return active_.begin(); }
		const_iterator begin() const { return active_.begin(); }

		iterator end() { return active_.end(); }
		const_iterator end() const { return active_.end(); }

		/// The number of active event handlers.
		size_type size() const { return active_.size(); }
		/// Access to active event handlers by index.
		handler_ptr & operator[](size_type index) { return active_[index]; }
	};//t_event_handlers

	void t_event_handlers::log_handlers()
	{
		if(lg::debug.dont_log("event_handler")) return;

		std::stringstream ss;

		BOOST_FOREACH( const handler_ptr & h, active_ ) {
			if ( !h )
				continue;
			const config& cfg = h->get_config();
			ss << "name=" << cfg["name"] << ", with id=" << cfg["id"] << "; ";
		}
		DBG_EH << "active handlers are now " << ss.str() << "\n";
	}

	/**
	 * Adds an event handler.
	 * An event with a nonempty ID will not be added if an event with that
	 * ID already exists.
	 */
	void t_event_handlers::add_event_handler(const config & cfg, bool is_menu_item)
	{
		std::string id = cfg["id"];
		if(!id.empty()) {
			BOOST_FOREACH( handler_ptr const & eh, active_ ) {
				if ( !eh )
					continue;
				config const & temp_config(eh->get_config());
				if(id == temp_config["id"]) {
					DBG_EH << "ignoring event handler for name=" << cfg["name"] <<
						" with id " << id << "\n";
					return;
				}
			}
		}
		DBG_EH << "inserting event handler for name=" << cfg["name"] <<
			" with id=" << id << "\n";
		handler_ptr new_handler(new event_handler(cfg, is_menu_item));
		new_handler->set_index(active_.size());
		active_.push_back(new_handler);
		log_handlers();
	}

	/**
	 * Removes an event handler, identified by its ID.
	 * Events with empty IDs cannot be removed.
	 */
	void t_event_handlers::remove_event_handler(std::string const & id)
	{
		if ( id.empty() )
			return;

		DBG_EH << "removing event handler with id " << id << "\n";

		// Loop through the active handler_vec.
		for ( handler_vec::iterator i = active_.begin(); i != active_.end(); ++i ) {
			if ( !*i )
				continue;
			// Try to match the id
			std::string event_id = (*i)->get_config()["id"];
			if ( event_id == id )
				i->reset();
		}
		log_handlers();
	}

	void t_event_handlers::clear()
	{
		active_.clear();
	}

}// end anonymous namespace (types)

namespace { // Variables
	t_event_handlers event_handlers;
	/** Map of the default action handlers known of the engine. */
	std::set<std::string> unit_wml_ids;
	std::set<std::string> used_items;
	std::vector< wmi_command_change > wmi_command_changes;
}// end anonymous namespace (variables)


/** Create an event handler. */
void add_event_handler(const config & handler, bool is_menu_item)
{
	event_handlers.add_event_handler(handler, is_menu_item);
}

/** Add a pending menu item command change. */
void add_wmi_change(const std::string & id, const config & new_command)
{
	wmi_command_changes.push_back(wmi_command_change(id, new config(new_command)));
}

/** Handles all the different types of actions that can be triggered by an event. */
void commit_wmi_commands()
{
	bool hotkeys_changed = false;
	// Commit WML Menu Item command changes
	while(wmi_command_changes.size() > 0) {
		wmi_command_change wcc = wmi_command_changes.front();

		if ( resources::gamedata->get_wml_menu_items().commit_change(wcc.first, *wcc.second) )
			hotkeys_changed = true;

		delete wcc.second;
		wmi_command_changes.erase(wmi_command_changes.begin());
	}
	if(hotkeys_changed)
	{
		preferences::save_hotkeys();
	}
}

/**
 * Checks if an item has been used.
 * (An empty id will never be considered used.)
 */
bool item_used(const std::string & id)
{
	return !id.empty()  &&  used_items.count(id) > 0;
}

/** Records if an item has been used. */
void item_used(const std::string & id, bool used)
{
	// Empty IDs are not tracked.
	if ( id.empty() )
		return;

	if ( used )
		used_items.insert(id);
	else
		used_items.erase(id);
}

/** Removes a pending menu item command change. */
void remove_wmi_change(const std::string & id)
{
	std::vector<wmi_command_change>::iterator wcc = wmi_command_changes.begin();
	while ( wcc != wmi_command_changes.end() ) {
		if ( wcc->first != id ) {
			++wcc;
			continue;
		}
		delete wcc->second;
		wcc->second = NULL;
		wcc = wmi_command_changes.erase(wcc);
	}
}

/** Removes an event handler. */
void remove_event_handler(const std::string & id)
{
	event_handlers.remove_event_handler(id);
}


/* ** manager ** */

manager::manager(const config& cfg)
{
	BOOST_FOREACH(const config &ev, cfg.child_range("event")) {
		add_event_handler(ev);
	}
	BOOST_FOREACH(const std::string &id, utils::split(cfg["unit_wml_ids"])) {
		unit_wml_ids.insert(id);
	}

	// Guard against a memory leak (now) / memory corruption (when this is deleted).
	// This is why creating multiple manager objects is prohibited.
	assert(resources::lua_kernel == NULL);
	resources::lua_kernel = new LuaKernel(cfg);

	wml_action::map::const_iterator action_end = wml_action::end();
	wml_action::map::const_iterator action_cur = wml_action::begin();
	for ( ; action_cur != action_end; ++action_cur ) {
		resources::lua_kernel->set_wml_action(action_cur->first, action_cur->second);
	}

	const std::string used = cfg["used_items"];
	if(!used.empty()) {
		const std::vector<std::string>& v = utils::split(used);
		for(std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
			item_used(*i, true);
		}
	}

	// Create the event handlers for menu items.
	resources::gamedata->get_wml_menu_items().init_handlers();
}

manager::~manager() {
	clear_events();
	event_handlers.clear();
	hotkey::delete_all_wml_hotkeys();
	reports::reset_generators();
	delete resources::lua_kernel;
	resources::lua_kernel = NULL;
	unit_wml_ids.clear();
	used_items.clear();
}


/* ** manager::iteration ** */

/**
 * Event-specific constructor.
 * This iteration will go through all event handlers matching the given name
 * (including those defined via menu items).
 * An empty @a event_name will automatically match nothing.
 */
manager::iteration::iteration(const std::string & event_name) :
	event_name_(event_name),
	end_(event_handlers.size()),
	index_(event_name.empty() ? end_ : 0),
	data_()
{
	// Look for the first handler that matches the provided name.
	while ( is_name_mismatch() )
		++index_;

	// Set the pointer?
	if ( index_ < end_ )
		data_ = event_handlers[index_];
}


/**
 * Increment
 */
manager::iteration & manager::iteration::operator++()
{
	// Look for the next handler that matches our stored name.
	do
		++index_;
	while ( is_name_mismatch() );

	// Set the pointer.
	if ( index_ < end_ )
		data_ = event_handlers[index_];
	else
		data_.reset();

	// Done.
	return *this;
}


/**
 * Tests index_ for being skippable when looking for an event name.
 */
bool manager::iteration::is_name_mismatch() const
{
	return index_ < end_  &&
		       (!event_handlers[index_]  ||
		        !event_handlers[index_]->matches_name(event_name_));
}


/* ** event_handler ** */

event_handler::event_handler(const config &cfg, bool imi) :
	first_time_only_(cfg["first_time_only"].to_bool(true)),
	is_menu_item_(imi), index_(-1), cfg_(cfg)
{}

/**
 * Handles the queued event, according to our WML instructions.
 * WARNING: *this may be destroyed at the end of this call, unless
 *          the caller maintains a handler_ptr to this.
 */
void event_handler::handle_event(const queued_event& event_info)
{
	handler_ptr preservative;

	if (first_time_only_)
	{
		// We should only be handling events if we've been added to the
		// active handlers.
		assert ( index_ < event_handlers.size() );
		// Prevent self-destructing mid-function.
		preservative = event_handlers[index_];
		// Disable this handler.
		event_handlers[index_].reset();
	}

	if (is_menu_item_) {
		DBG_NG << cfg_["name"] << " will now invoke the following command(s):\n" << cfg_;
	}

	handle_event_commands(event_info, vconfig(cfg_));
}

bool event_handler::matches_name(const std::string &name) const
{
	const std::string my_names =
		utils::interpolate_variables_into_string(cfg_["name"], *(resources::gamedata));
	std::string::const_iterator itor,
		it_begin = my_names.begin(),
		it_end = my_names.end(),
		match_it = name.begin(),
		match_begin = name.begin(),
		match_end = name.end();
	int skip_count = 0;
	for(itor = it_begin; itor != it_end; ++itor) {
		bool do_eat = false,
			do_skip = false;
		switch(*itor) {
		case ',':
			if(itor - it_begin - skip_count == match_it - match_begin && match_it == match_end) {
				return true;
			}
			it_begin = itor + 1;
			match_it = match_begin;
			skip_count = 0;
			continue;
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '\v':
			do_skip = (match_it == match_begin || match_it == match_end);
			break;
		case ' ':
			do_skip = (match_it == match_begin || match_it == match_end);
			// fall through to case '_'
		case '_':
			do_eat = (match_it != match_end && (*match_it == ' ' || *match_it == '_'));
			break;
		default:
			do_eat = (match_it != match_end && *match_it == *itor);
			break;
		}
		if(do_eat) {
			++match_it;
		} else if(do_skip) {
			++skip_count;
		} else {
			itor = std::find(itor, it_end, ',');
			if(itor == it_end) {
				return false;
			}
			it_begin = itor + 1;
			match_it = match_begin;
			skip_count = 0;
		}
	}
	if(itor - it_begin - skip_count == match_it - match_begin && match_it == match_end) {
		return true;
	}
	return false;
}


void add_events(const config::const_child_itors &cfgs, const std::string& type)
{
	if(!type.empty()) {
		if(std::find(unit_wml_ids.begin(),unit_wml_ids.end(),type) != unit_wml_ids.end()) return;
		unit_wml_ids.insert(type);
	}
	BOOST_FOREACH(const config &new_ev, cfgs) {
		if(type.empty() && new_ev["id"].empty())
		{
			WRN_NG << "attempt to add an [event] with empty id=, ignoring \n";
			continue;
		}
		add_event_handler(new_ev);
	}
}

void write_events(config& cfg)
{
	BOOST_FOREACH(const handler_ptr &eh, event_handlers) {
		if ( !eh || eh->is_menu_item() ) {
			continue;
		}
		cfg.add_child("event", eh->get_config());
	}

	cfg["used_items"] = utils::join(used_items);
	cfg["unit_wml_ids"] = utils::join(unit_wml_ids);

	if (resources::soundsources)
		resources::soundsources->write_sourcespecs(cfg);

	assert(resources::lua_kernel != NULL);
	resources::lua_kernel->save_game(cfg);
}

} // end namespace game_events

