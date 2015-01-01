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
 * Define the game's event mechanism.
 *
 * Events might be units moving or fighting, or when victory or defeat occurs.
 * A scenario's configuration file will define actions to take when certain events occur.
 * This module is responsible for the processing of events.
 *
 * Note that game events have nothing to do with SDL events,
 * like mouse movement, keyboard events, etc.
 * See events.hpp for how they are handled.
 */

#ifndef GAME_EVENTS_PUMP_H_INCLUDED
#define GAME_EVENTS_PUMP_H_INCLUDED

#include "entity_location.hpp"
#include "../config.hpp"

class vconfig;


namespace game_events
{
	struct queued_event {
		queued_event(const std::string& name, const entity_location& loc1,
		             const entity_location& loc2, const config& data)
			: name(name), loc1(loc1), loc2(loc2), data(data)
		{}

		std::string name;
		entity_location loc1;
		entity_location loc2;
		config data;
	};

	/// The general environment within which events are processed.
	class context {
		/// State when processing a particular flight of events or commands.
		struct event_context {
			bool mutated;
			bool skip_messages;

			explicit event_context(bool s) : mutated(true), skip_messages(s) {}
		};

	public:
		/// Context state with automatic lifetime handling.
		class scoped {
		public:
			scoped();
			~scoped();

		private:
			context::event_context *old_context_;
			context::event_context new_context_;
		};
		friend class scoped;

	public:
		// No constructor needed since this is a static-only class for now.

		/// Returns whether or not we believe WML might have changed something.
		static bool mutated()	{ return current_context_->mutated; }
		/// Sets whether or not we believe WML might have changed something.
		static void mutated(bool mutated)	{ current_context_->mutated = mutated; }
		/// Returns whether or not the screen (map visuals) needs to be rebuilt.
		static bool screen_needs_rebuild()	{ return rebuild_screen_; }
		/// Sets whether or not the screen (map visuals) needs to be rebuilt.
		static void screen_needs_rebuild(bool rebuild)	{ rebuild_screen_ = rebuild; }
		/// Returns whether or not we are skipping messages.
		static bool skip_messages()	{ return current_context_->skip_messages; }
		/// Sets whether or not we are skipping messages.
		static void skip_messages(bool skip)	{ current_context_->skip_messages = skip; }

	private:
		static event_context * current_context_;
		static bool rebuild_screen_;
		/// A default value used to avoid NULL pointers.
		static event_context default_context_;
	};


	/// Helper function which determines whether a wml_message text can
	/// really be pushed into the wml_messages_stream, and does it.
	void put_wml_message(const std::string& logger, const std::string& message, bool in_chat);

	/**
	 * Directly runs the lua command(s) @a lua_code
	 */
	void run_lua_commands(char const *lua_code);

	/**
	 * Runs the action handler associated to the command sequence @a cfg.
	 */
	void handle_event_commands(const queued_event &event_info, const vconfig &cfg);

	/**
	 * Runs the action handler associated to @a cmd with parameters @a cfg.
	 */
	void handle_event_command(const std::string &cmd,
	                          const queued_event &event_info, const vconfig &cfg);


	/**
	 * Function to fire an event.
	 *
	 * Events may have up to two arguments, both of which must be locations.
	 */
	bool fire(const std::string& event,
	          const entity_location& loc1=entity_location::null_entity,
	          const entity_location& loc2=entity_location::null_entity,
	          const config& data=config());

	void raise(const std::string& event,
	           const entity_location& loc1=entity_location::null_entity,
	           const entity_location& loc2=entity_location::null_entity,
	           const config& data=config());

	bool pump();

	/// Clears all events tha have been raised (and not pumped).
	void clear_events();

	/**
	 * Flushes WML messages and errors.
	 */
	void flush_messages();

	/**
	 * This function can be used to detect when no WML/Lua has been executed.
	 */
	size_t wml_tracking();

}

#endif // GAME_EVENTS_PUMP_H_INCLUDED

