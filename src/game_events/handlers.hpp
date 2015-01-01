/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
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
 * Define the handlers for the game's events mechanism.
 *
 * Events might be units moving or fighting, or when victory or defeat occurs.
 * A scenario's configuration file will define actions to take when certain events occur.
 * This module is responsible for tracking these definitions.
 */

#ifndef GAME_EVENTS_HANDLERS_H_INCLUDED
#define GAME_EVENTS_HANDLERS_H_INCLUDED

#include "../config.hpp"
#include "../utils/smart_list.hpp"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace game_events
{
	struct queued_event;
	class event_handler;  // Defined a few lines down.


	/// Shared pointer to handler objects.
	typedef boost::shared_ptr<event_handler> handler_ptr;
	/// Storage of event handlers.
	typedef std::vector<handler_ptr> handler_vec;


	class event_handler
	{
		public:
			event_handler(const config &cfg, bool is_menu_item,
			              handler_vec::size_type index);

			/// The index of *this should only be of interest when controlling iterations.
			handler_vec::size_type index() const { return index_; }

			bool matches_name(const std::string& name) const;

			bool is_menu_item() const { return is_menu_item_; }

			/// Disables *this, removing it from the game.
			void disable();
			void handle_event(const queued_event& event_info, handler_ptr& handler_p);

			const config &get_config() const { return cfg_; }

		private:
			bool first_time_only_;
			bool is_menu_item_;
			handler_vec::size_type index_;
			config cfg_;
	};


	/// This is a wrapper for a list of weak pointers to handlers. It allows forward
	/// iterations of the list, with each element returned as a shared pointer.
	/// (Weak pointers that fail to lock are silently removed from the list.) These
	/// iterations can be used recursively, even when the innermost iteration might
	/// erase arbitrary elements from the list.
	///
	/// The interface is not the standard list interface because that would be
	/// inconvenient. The functionality implemented is that required by Wesnoth.
	class  handler_list
	{
		/// The weak pointers that are used internally.
		typedef boost::weak_ptr<event_handler> internal_ptr;
		/// The underlying list.
		typedef utils::smart_list<internal_ptr> list_t;
		
	public: // types
		/// Handler list iterators are rather limited. They can be constructed
		/// from a reference iterator (not default constructed), incremented,
		/// and dereferenced. Consecutive dereferences are not guaranteed to
		/// return the same element (if the list mutates between them, the next
		/// element might be returned). An increment guarantees that the next
		/// dereference will differ from the previous (unless at the end of the
		/// list). The end of the list is indicated by dereferencing to a null
		/// pointer.
		class iterator
		{
			/// The current element.
			list_t::iterator iter_;

		public:
			/// Initialized constructor (to be called by handler_list).
			explicit iterator(const list_t::iterator & base_iter) :
				iter_(base_iter)
			{}

			/// Increment.
			iterator & operator++()            { ++iter_; return *this; }
			/// Dereference.
			handler_ptr operator*();
		};
		friend class iterator;
		typedef iterator const_iterator;

	public: // functions
		/// Default constructor.
		/// Note: This explicit definition is required (by the more pedantic
		///       compilers) in order to declare a default-constructed, static,
		///       and const variable in t_event_handlers::get(), in handlers.cpp.
		handler_list() : data_() {}

		const_iterator begin() const           { return iterator(const_cast<list_t &>(data_).begin()); }
		// The above const_cast is so the iterator can remove obsolete entries.
		const_iterator end() const             { return iterator(const_cast<list_t &>(data_).end()); }

		// push_front() is probably unneeded, but I'll leave the code here, just in case.
		// (These lists must be maintained in index order, which means pushing to the back.)
		void push_front(const handler_ptr & p) { data_.push_front(internal_ptr(p)); }
		void push_back(const handler_ptr & p)  { data_.push_back(internal_ptr(p)); }

		void clear()                           { data_.clear(); }

	private:
		/// No implementation of operator=() since smart_list does not support it.
		handler_list & operator=(const handler_list &);

		/// The actual list.
		list_t data_;
	};


	/// The game event manager loads the scenario configuration object,
	/// and ensures that events are handled according to the
	/// scenario configuration for its lifetime.
	///
	/// Thus, a manager object should be created when a scenario is played,
	/// and destroyed at the end of the scenario.
	/// If a second manager object is created before destroying the previous
	/// one, the game will crash with an assertion failure.
	///
	/// This class is responsible for setting and clearing resources::lua_kernel.
	class manager : boost::noncopyable {
	public:
		/// This class is similar to an input iterator through event handlers,
		/// except each instance knows its own end (determined when constructed).
		/// Subsequent dereferences are not guaranteed to return the same element,
		/// so it is important to assign a dereference to a variable if you want
		/// to use it more than once. On the other hand, a dereference will not
		/// return a null pointer until the end of the iteration is reached (and
		/// this is how to detect the end of the iteration).
		///
		/// For simplicity, this class is neither assignable nor equality
		/// comparable nor default constructable, and there is no postincrement.
		/// Typedefs are also skipped.
		class iteration
		{
		public:
			/// Event-specific constructor.
			explicit iteration(const std::string & event_name);

			// Increment:
			iteration & operator++();
			// Dereference:
			handler_ptr operator*();

		private: // functions
			/// Gets the index from a pointer, capped at end_.
			handler_vec::size_type ptr_index(const handler_ptr & ptr) const
			{ return !bool(ptr) ? end_ : std::min(ptr->index(), end_); }

		private: // data
			/// The fixed-name event handlers for this iteration.
			const handler_list & main_list_;
			/// The varying-name event handlers for this iteration.
			const handler_list & var_list_;
			/// The event name for this iteration.
			const std::string event_name_;
			/// The end of this iteration. We intentionally exclude handlers
			/// added after *this is constructed.
			const handler_vec::size_type end_;

			/// Set to true upon dereferencing.
			bool current_is_known_;
			/// true if the most recent dereference was taken from main_list_.
			bool main_is_current_;
			/// The current (or next) element from main_list_.
			handler_list::iterator main_it_;
			/// The current (or next) element from var_list_.
			handler_list::iterator var_it_;
		};

	public:
		/// Note that references will be maintained,
		/// and must remain valid for the life of the object.
		explicit manager(const config& scenario_cfg);
		~manager();
	};

	/// Create an event handler.
	void add_event_handler(const config & handler, bool is_menu_item=false);
	/// Checks if an item has been used.
	bool item_used(const std::string & id);
	/// Records if an item has been used.
	void item_used(const std::string & id, bool used);
	/// Removes an event handler.
	void remove_event_handler(const std::string & id);

	void add_events(const config::const_child_itors &cfgs,
	                const std::string& type = std::string());
	void write_events(config& cfg);
}

#endif // GAME_EVENTS_HANDLERS_H_INCLUDED

