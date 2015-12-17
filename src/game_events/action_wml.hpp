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
 * Define actions for the game's events mechanism.
 *
 * Events might be units moving or fighting, or when victory or defeat occurs.
 * A scenario's configuration file will define actions to take when certain events occur.
 * This module is responsible for implementing some of those actions (i.e. Action WML).
 * The actions not implemented in this module are implemented in Lua.
 */

#ifndef GAME_EVENTS_ACTION_WML_H_INCLUDED
#define GAME_EVENTS_ACTION_WML_H_INCLUDED

#include "terrain_type_data.hpp"
#include <map>
#include <string>

class  config;
struct map_location;
class  vconfig;

namespace t_translation {
	struct t_terrain;
}


namespace game_events
{
	struct queued_event;


	// Most of the functionality in the source file is accessed via callbacks,
	// accessed by iterating over wml_action.

	class wml_action {
	public:
		typedef void (*handler)(const queued_event &, const vconfig &);
		typedef std::map<std::string, handler> map;

		/// Using this constructor for a static object outside action_wml.cpp
		/// will likely lead to a static initialization fiasco.
		wml_action(const std::string & tag, handler function);

		/// The first registered action.
		static map::const_iterator begin()  { return registry_.begin(); }
		/// One past the last registered action.
		static map::const_iterator end()    { return registry_.end(); }
		static const map& registry() { return registry_; }
	private:
		/// Tracks the known action handlers.
		static map registry_;
	};


	/**
	 * Changes a terrain location.
	 * Ensures that villages are properly lost and that new terrains are discovered.
	 */
	void change_terrain(const map_location &loc, const t_translation::t_terrain &t,
	                    terrain_type_data::tmerge_mode mode, bool replace_if_failed);

	/** Used for [deprecated_message]. */
	void handle_deprecated_message(const config& cfg);
	/** Used for [wml_message]. */
	void handle_wml_log_message(const config& cfg);
}

#endif // GAME_EVENTS_ACTION_WML_H_INCLUDED

