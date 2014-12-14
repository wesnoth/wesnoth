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
	namespace context {
		/// Returns whether or not we believe WML might have changed something.
		bool mutated();
		/// Sets whether or not we believe WML might have changed something.
		void mutated(bool mutated);
		/// Returns whether or not we are skipping messages.
		bool skip_messages();
		/// Sets whether or not we are skipping messages.
		void skip_messages(bool skip);
	}

	/// Helper function which determines whether a wml_message text can
	/// really be pushed into the wml_messages_stream, and does it.
	void put_wml_message(const std::string& logger, const std::string& message, bool in_chat);

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

