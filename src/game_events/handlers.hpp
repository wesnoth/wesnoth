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


namespace game_events
{
	struct queued_event;


	class event_handler
	{
		public:
			explicit event_handler(const config &cfg, bool is_menu_item = false);

			bool matches_name(const std::string& name) const;

			bool disabled() const { return disabled_; }
			bool is_menu_item() const { return is_menu_item_; }

			void handle_event(const queued_event& event_info);

			const config &get_config() const { return cfg_; }

		private:
			bool first_time_only_;
			bool disabled_;
			bool is_menu_item_;
			config cfg_;
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
	/// This struct is responsible for setting and clearing resources::lua_kernel.
	struct manager : boost::noncopyable {
		/// Note that references will be maintained,
		/// and must remain valid for the life of the object.
		explicit manager(const config& scenario_cfg);
		~manager();

		// Allow iterating over the active handlers.
		typedef std::vector<event_handler> t_active;
		typedef t_active::iterator iterator;
		static iterator begin();
		static iterator end();

		/// Starts buffering event handler creation.
		static void start_buffering();
		/// Ends buffering event handler creation.
		static void stop_buffering();
		/// Commits the event handlers that were buffered.
		static void commit_buffer();
	};

	/// Create an event handler.
	void add_event_handler(const config & handler, bool is_menu_item=false);
	/// Add a pending menu item command change.
	void add_wmi_change(const std::string & id, const config & new_command);
	/// Handles all the different types of actions that can be triggered by an event.
	void commit_wmi_commands();
	/// Checks if an item has been used.
	bool item_used(const std::string & id);
	/// Records if an item has been used.
	void item_used(const std::string & id, bool used);
	/// Removes an event handler.
	void remove_event_handler(const std::string & id);
	/// Removes a pending menu item command change.
	void remove_wmi_change(const std::string & id);

	void add_events(const config::const_child_itors &cfgs,
	                const std::string& type = std::string());
	void write_events(config& cfg);
}

#endif // GAME_EVENTS_HANDLERS_H_INCLUDED

