/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/game_info.hpp
 * Game information for the AI
 */

#ifndef AI_GAME_INFO_HPP_INCLUDED
#define AI_GAME_INFO_HPP_INCLUDED

class game_display;
class gamemap;

#include "../gamestatus.hpp"
#include "../playturn.hpp"

/**
 * info is structure which holds references to all the important objects
 * that an AI might need access to, in order to make and implement its
 * decisions.
 */
class ai_game_info {
public:

		ai_game_info(game_display& disp, gamemap& map, unit_map& units,
			std::vector<team>& teams, gamestatus& state, class game_state& game_state)
			: disp(disp), map(map), units(units), teams(teams),
			   state(state), game_state_(game_state)
		{}

		/** The display object, used to draw the moves the AI makes. */
		game_display& disp;

		/** The map of the game -- use this object to find the terrain at any location. */
		gamemap& map;

		/** The map of units. It maps locations -> units. */
		unit_map& units;

		/** A list of the teams in the game. */
		std::vector<team>& teams;

		/** Information about what turn it is, and what time of day. */
		gamestatus& state;

		/** The global game state, because we may set the completion field. */
		class game_state& game_state_;
};

#endif
