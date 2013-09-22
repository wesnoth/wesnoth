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

#include "global.hpp"
#include "handlers.hpp"
#include "pump.hpp"

#include "../formula_string_utils.hpp"
#include "../gamestatus.hpp"
#include "../log.hpp"
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
		typedef manager::t_active t_active;
	public:
		typedef t_active::iterator iterator;
		typedef t_active::const_iterator const_iterator;

	private:
		t_active active_; ///Active event handlers
		t_active insert_buffer_; ///Event handlers added while pumping events
		std::set<std::string> remove_buffer_; ///Event handlers removed while pumping events
		bool buffering_;


		void log_handler(std::stringstream& ss,
		                 const std::vector<event_handler>& handlers,
		                 const std::string& msg);
		void log_handlers();

	public:

		t_event_handlers()
			: active_(), insert_buffer_(), remove_buffer_(), buffering_(false)
		{}

		/// Adds an event handler.
		void add_event_handler(const config & cfg, bool is_menu_item=false);
		/// Removes an event handler, identified by its ID.
		void remove_event_handler(std::string const & id);
		/// Starts buffering.
		void start_buffering();
		void stop_buffering();
		/// Commits all buffered events.
		void commit_buffer();
		void clear();

		iterator begin() { return active_.begin(); }
		const_iterator begin() const { return active_.begin(); }

		iterator end() { return active_.end(); }
		const_iterator end() const { return active_.end(); }
	};//t_event_handlers

	void t_event_handlers::log_handler(std::stringstream& ss,
	                 const std::vector<event_handler>& handlers,
	                 const std::string& msg)
	{
		BOOST_FOREACH(const event_handler& h, handlers){
			const config& cfg = h.get_config();
			ss << "name=" << cfg["name"] << ", with id=" << cfg["id"] << "; ";
		}
		DBG_EH << msg << " handlers are now " << ss.str() << "\n";
		ss.str(std::string());
	}

	void t_event_handlers::log_handlers()
	{
		if(lg::debug.dont_log("event_handler")) return;

		std::stringstream ss;
		log_handler(ss, active_, "active");
		log_handler(ss, insert_buffer_, "insert buffered");
		BOOST_FOREACH(const std::string& h, remove_buffer_){
			ss << "id=" << h << "; ";
		}
		DBG_EH << "remove buffered handlers are now " << ss.str() << "\n";
	}

	/**
	 * Adds an event handler.
	 * An event with a nonempty ID will not be added if an event with that
	 * ID already exists.  This method respects this class's buffering
	 * functionality.
	 */
	void t_event_handlers::add_event_handler(const config & cfg, bool is_menu_item)
	{
		if(buffering_) {
			DBG_EH << "buffering event handler for name=" << cfg["name"] <<
			" with id " << cfg["id"] << "\n";
			insert_buffer_.push_back(event_handler(cfg, is_menu_item));
			log_handlers();
		}
		else {
			std::string id = cfg["id"];
			if(!id.empty()) {
				BOOST_FOREACH( event_handler const & eh, active_ ) {
					config const & temp_config(eh.get_config());
					if(id == temp_config["id"]) {
						DBG_EH << "ignoring event handler for name=" << cfg["name"] <<
							" with id " << id << "\n";
						return;
					}
				}
			}
			DBG_EH << "inserting event handler for name=" << cfg["name"] <<
				" with id=" << id << "\n";
			active_.push_back(event_handler(cfg, is_menu_item));
			log_handlers();
		}
	}

	/**
	 * Removes an event handler, identified by its ID.
	 * Events with empty IDs cannot be removed.  This method respects this
	 * class's buffering functionality.
	 */
	void t_event_handlers::remove_event_handler(std::string const & id)
	{
		if(id == "") { return; }

		DBG_EH << "removing event handler with id " << id << "\n";

		if(buffering_) { remove_buffer_.insert(id); }

		t_active &temp = buffering_ ? insert_buffer_ : active_;

		t_active::iterator i = temp.begin();
		while(i < temp.end()) {
			config const & temp_config = (*i).get_config();
			std::string event_id = temp_config["id"];
			if(event_id != "" && event_id == id) {
				i = temp.erase(i); }
			else {
				++i; }
		}
		log_handlers();
	}

	/**
	 * Starts buffering.
	 * While buffering, any calls to add_event_handler() and
	 * remove_event_handler() will not take effect until commit_buffer()
	 * is called.  This function is idempotent - starting a buffer
	 * when already buffering will not start a second buffer.
	 */
	void t_event_handlers::start_buffering()
	{
		buffering_ = true;
		DBG_EH << "starting buffering...\n";
	}

	void t_event_handlers::stop_buffering()
	{
		DBG_EH << "stopping buffering...\n";
		buffering_ = false;
	}

	/**
	 * Commits all buffered events.
	 */
	void t_event_handlers::commit_buffer()
	{
		DBG_EH << "committing buffered event handlers, buffering: " << buffering_ << "\n";
		if(buffering_)
			return;

		// Commit any event removals
		BOOST_FOREACH( std::string const & i, remove_buffer_ ){
			remove_event_handler(i); }
		remove_buffer_.clear();

		// Commit any spawned events-within-events
		BOOST_FOREACH( event_handler const & i, insert_buffer_ ){
			add_event_handler(i.get_config(), i.is_menu_item()); }
		insert_buffer_.clear();

		log_handlers();
	}

	void t_event_handlers::clear()
	{
		active_.clear();
		insert_buffer_.clear();
		remove_buffer_.clear();
		buffering_ = false;
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
	// Commit WML Menu Item command changes
	while(wmi_command_changes.size() > 0) {
		wmi_command_change wcc = wmi_command_changes.front();
		const bool is_empty_command = wcc.second->empty();

		wml_menu_item & item = resources::gamedata->get_wml_menu_items().get_item(wcc.first);
		const std::string & event_name = item.event_name();

		config::attribute_value & event_id = (*wcc.second)["id"];
		if ( event_id.empty() && !wcc.first.empty() ) {
			event_id = wcc.first;
		}
		(*wcc.second)["name"] = event_name;
		(*wcc.second)["first_time_only"] = false;

		if ( !item.command().empty() ) {
			BOOST_FOREACH(event_handler& hand, event_handlers) {
				if ( hand.is_menu_item() && hand.matches_name(event_name) ) {
					LOG_NG << "changing command for " << event_name << " to:\n" << *wcc.second;
					hand = event_handler(*wcc.second, true);
				}
			}
		} else if(!is_empty_command) {
			LOG_NG << "setting command for " << event_name << " to:\n" << *wcc.second;
			add_event_handler(*wcc.second, true);
		}

		item.set_command(*wcc.second);
		delete wcc.second;
		wmi_command_changes.erase(wmi_command_changes.begin());
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
	int wmi_count = 0;
	BOOST_FOREACH( const wml_menu_item & wmi, resources::gamedata->get_wml_menu_items() ) {
		const config & wmi_command = wmi.command();
		if ( !wmi_command.empty() ) {
			add_event_handler(wmi_command, true);
		}
		++wmi_count;
	}
	if(wmi_count > 0) {
		LOG_NG << wmi_count << " WML menu items found, loaded." << std::endl;
	}
}

manager::~manager() {
	clear_events();
	event_handlers.clear();
	reports::reset_generators();
	delete resources::lua_kernel;
	resources::lua_kernel = NULL;
	unit_wml_ids.clear();
	used_items.clear();
}

/** Returns an iterator to the first event handler. */
manager::iterator manager::begin()
{
	return event_handlers.begin();
}

/** Returns an iterator to one past the last event handler. */
manager::iterator manager::end()
{
	return event_handlers.end();
}

/** Starts buffering event handler creation. */
void manager::start_buffering()
{
	event_handlers.start_buffering();
}

/** Ends buffering event handler creation. */
void manager::stop_buffering()
{
	event_handlers.stop_buffering();
}

/** Commits the event handlers that were buffered. */
void manager::commit_buffer()
{
	event_handlers.commit_buffer();
}


event_handler::event_handler(const config &cfg, bool imi) :
	first_time_only_(cfg["first_time_only"].to_bool(true)),
	disabled_(false), is_menu_item_(imi), cfg_(cfg)
{}

void event_handler::handle_event(const queued_event& event_info)
{
	if (first_time_only_)
	{
		disabled_ = true;
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
	BOOST_FOREACH(const event_handler &eh, event_handlers) {
		if ( eh.disabled() || eh.is_menu_item() ) {
			continue;
		}
		cfg.add_child("event", eh.get_config());
	}

	cfg["used_items"] = utils::join(used_items);
	cfg["unit_wml_ids"] = utils::join(unit_wml_ids);

	if (resources::soundsources)
		resources::soundsources->write_sourcespecs(cfg);

	assert(resources::lua_kernel != NULL);
	resources::lua_kernel->save_game(cfg);
}

} // end namespace game_events

