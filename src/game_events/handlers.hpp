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
 * Define the handlers for the game's events mechanism.
 *
 * Events might be units moving or fighting, or when victory or defeat occurs.
 * A scenario's configuration file will define actions to take when certain events occur.
 * This module is responsible for tracking these definitions.
 */

#ifndef GAME_EVENTS_HANDLERS_H_INCLUDED
#define GAME_EVENTS_HANDLERS_H_INCLUDED

#include "../config.hpp"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


namespace game_events
{
	struct queued_event;


	class event_handler
	{
		public:
			explicit event_handler(const config &cfg, bool is_menu_item = false);

			/// Allows the event_handlers object to record the index of *this.
			void set_index(size_t index) { index_ = index; }

			bool matches_name(const std::string& name) const;

			bool is_menu_item() const { return is_menu_item_; }

			void handle_event(const queued_event& event_info);

			const config &get_config() const { return cfg_; }

		private:
			bool first_time_only_;
			bool is_menu_item_;
			size_t index_;
			config cfg_;
	};
	/// Shared pointer to handler objects.
	typedef boost::shared_ptr<event_handler> handler_ptr;
	/// Storage of event handlers.
	typedef std::vector<handler_ptr> handler_vec;


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
			const handler_ptr & operator*();

		private: // functions
			/// Tests index_ for being skippable in this iteration.
			bool is_name_mismatch() const;

		private: // data
			/// The event name for this iteration.
			const std::string event_name_;
			/// The end of this iteration. We intentionally exclude handlers
			/// added after *this is constructed.
			const handler_vec::size_type end_;

			/// The current index.
			handler_vec::size_type index_;
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

