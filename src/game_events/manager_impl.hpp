/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GAME_EVENTS_MANAGER_IMPL_HPP
#define GAME_EVENTS_MANAGER_IMPL_HPP

#include "game_events/handlers.hpp"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

namespace game_events {

	//t_event_handlers is essentially the implementation details of the manager
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
		void add_event_handler(const config & cfg, manager & man, bool is_menu_item=false);
		/// Removes an event handler, identified by its ID.
		void remove_event_handler(std::string const & id);

		iterator begin() { return active_.begin(); }
		const_iterator begin() const { return active_.begin(); }

		iterator end() { return active_.end(); }
		const_iterator end() const { return active_.end(); }

		/// The number of active event handlers.
		size_type size() const { return active_.size(); }
		/// Access to active event handlers by index.
		handler_ptr & operator[](size_type index) { return active_[index]; }
	};//t_event_handlers

} //end namespace game_events

#endif
