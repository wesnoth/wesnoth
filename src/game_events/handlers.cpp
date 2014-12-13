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
#include "../game_data.hpp"
#include "../log.hpp"
#include "../reports.hpp"
#include "../resources.hpp"
#include "../scripting/game_lua_kernel.hpp"
#include "../serialization/string_utils.hpp"
#include "../soundsource.hpp"
#include "../util.hpp"

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
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
	class t_event_handlers {
		typedef boost::unordered_map<std::string, handler_list> map_t;
		typedef boost::unordered_map<std::string, boost::weak_ptr<event_handler> > id_map_t;

	public:
		typedef handler_vec::iterator iterator;
		typedef handler_vec::const_iterator const_iterator;

	private:
		handler_vec  active_;  /// Active event handlers. Will not have elements removed unless the t_event_handlers is clear()ed.
		map_t        by_name_; /// Active event handlers with fixed event names, organized by event name.
		handler_list dynamic_; /// Active event handlers with variables in their event names.
		id_map_t     id_map_;  /// Allows quick locating of handlers by id.


		void log_handlers();
		/// Utility to standardize the event names used in by_name_.
		static std::string standardize_name(const std::string & name);

	public:
		typedef handler_vec::size_type size_type;

		t_event_handlers()
			: active_()
			, by_name_()
			, dynamic_()
			, id_map_()
		{}

		/// Read-only access to the handlers with varying event names.
		const handler_list & get() const { return dynamic_; }
		/// Read-only access to the handlers with fixed event names, by event name.
		const handler_list & get(const std::string & name) const;

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
	 * Utility to standardize the event names used in by_name_.
	 * This means stripping leading and trailing spaces, and converting internal
	 * spaces to underscores.
	 */
	std::string t_event_handlers::standardize_name(const std::string & name)
	{
		std::string retval;
		size_t name_index = 0;
		size_t name_size = name.size();

		// Trim trailing spaces off the name.
		while ( name_size > 0  &&  name[name_size-1] == ' ' )
			--name_size	;

		// Trim leading spaces off the name.
		while ( name_index < name_size  &&  name[name_index] == ' ' )
			++name_index;

		// Copy the rest, converting any remaining spaces to underscores.
		retval.reserve(name_size - name_index);
		while ( name_index < name_size ) {
			char c = name[name_index++];
			retval.push_back(c == ' ' ? '_' : c);
		}

		return retval;
	}

	/**
	 * Read-only access to the handlers with fixed event names, by event name.
	 */
	const handler_list & t_event_handlers::get(const std::string & name) const
	{
		// Empty list for the "not found" case.
		static const handler_list empty_list;

		// Look for the name in the name map.
		map_t::const_iterator find_it = by_name_.find(standardize_name(name));
		return find_it == by_name_.end() ? empty_list : find_it->second;
	}

	/**
	 * Adds an event handler.
	 * An event with a nonempty ID will not be added if an event with that
	 * ID already exists.
	 */
	void t_event_handlers::add_event_handler(const config & cfg, bool is_menu_item)
	{
		const std::string name = cfg["name"];
		std::string id = cfg["id"];

		if(!id.empty()) {
			// Ignore this handler if there is already one with this ID.
			id_map_t::iterator find_it = id_map_.find(id);
			if ( find_it != id_map_.end()  &&  !find_it->second.expired() ) {
				DBG_EH << "ignoring event handler for name='" << name
				       << "' with id '" << id << "'\n";
				return;
			}
		}

		// Create a new handler.
		DBG_EH << "inserting event handler for name=" << name <<
			" with id=" << id << "\n";
		handler_ptr new_handler(new event_handler(cfg, is_menu_item, active_.size()));
		active_.push_back(new_handler);

		// File by name.
		if ( utils::might_contain_variables(name) )
			dynamic_.push_back(new_handler);
		else {
			std::vector<std::string> name_list = utils::split(name);
			BOOST_FOREACH( const std::string & single_name, name_list )
				by_name_[standardize_name(single_name)].push_back(new_handler);
		}
		// File by ID.
		if ( !id.empty() )
			id_map_[id] = new_handler;

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

		// Find the existing handler with this ID.
		id_map_t::iterator find_it = id_map_.find(id);
		if ( find_it != id_map_.end() ) {
			handler_ptr handler = find_it->second.lock();
			// Remove handler.
			if ( handler )
				handler->disable();
			id_map_.erase(find_it); // Do this even if the lock failed.
			// The index by name will self-adjust later. No need to adjust it now.
		}

		log_handlers();
	}

	void t_event_handlers::clear()
	{
		active_.clear();
		by_name_.clear();
		dynamic_.clear();
		id_map_.clear();
	}

}// end anonymous namespace (types)

namespace { // Variables
	t_event_handlers event_handlers;
	/** Map of the default action handlers known of the engine. */
	std::set<std::string> unit_wml_ids;
	std::set<std::string> used_items;
}// end anonymous namespace (variables)


/** Create an event handler. */
void add_event_handler(const config & handler, bool is_menu_item)
{
	event_handlers.add_event_handler(handler, is_menu_item);
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
	assert(resources::lua_kernel != NULL);

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
	main_list_(event_handlers.get(event_name)),
	var_list_(event_handlers.get()),
	event_name_(event_name),
	end_(event_handlers.size()),
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


/* ** handler_list::iterator ** */

/**
 * Dereference.
 * If the current element has become invalid, we will increment first.
 */
handler_ptr handler_list::iterator::operator*()
{
	// Check for an available handler.
	while ( iter_.derefable() ) {
		// Handler still accessible?
		if ( handler_ptr lock = iter_->lock() )
			return lock;
		else
			// Remove the now-defunct entry.
			iter_ = list_t::erase(iter_);
	}

	// End of the list.
	return handler_ptr();
}


/* ** event_handler ** */

event_handler::event_handler(const config &cfg, bool imi, handler_vec::size_type index) :
	first_time_only_(cfg["first_time_only"].to_bool(true)),
	is_menu_item_(imi), index_(index), cfg_(cfg)
{}

/**
 * Disables *this, removing it from the game.
 * (Technically, the handler is only removed once no one is hanging on to a
 * handler_ptr to *this. So be careful how long they persist.)
 *
 * WARNING: *this may be destroyed at the end of this call, unless
 *          the caller maintains a handler_ptr to this.
 */
void event_handler::disable()
{
	// Handlers must have an index after they're created.
	assert ( index_ < event_handlers.size() );
	// Disable this handler.
	event_handlers[index_].reset();
}

/**
 * Handles the queued event, according to our WML instructions.
 * WARNING: *this may be destroyed at the end of this call, unless
 *          the caller maintains a handler_ptr to this.
 *
 * @param[in]     event_info  Information about the event that needs handling.
 * @param[in,out] handler_p   The caller's smart pointer to *this. It may be
 *                            reset() during processing.
 */
void event_handler::handle_event(const queued_event& event_info, handler_ptr& handler_p)
{
	// We will need our config after possibly self-destructing. Make a copy now.
	vconfig vcfg(cfg_, true);

	if (is_menu_item_) {
		DBG_NG << cfg_["name"] << " will now invoke the following command(s):\n" << cfg_;
	}

	if (first_time_only_)
	{
		// Disable this handler.
		disable();
		// Also remove our caller's hold on us.
		handler_p.reset();
	}
	// *WARNING*: At this point, dereferencing this could be a memory violation!
	// ^ this comment does not refer to resources::lua_kernel below, but to the vconfig

	assert(resources::lua_kernel);
	resources::lua_kernel->run_wml_action("command", vcfg, event_info);
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
			WRN_NG << "attempt to add an [event] with empty id=, ignoring " << std::endl;
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
}

} // end namespace game_events

