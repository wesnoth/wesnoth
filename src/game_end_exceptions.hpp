/*
   Copyright (C) 2006 - 2014 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
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
 * Contains the exception interfaces used to signal
 * completion of a scenario, campaign or turn.
 */

#ifndef GAME_END_EXCEPTIONS_HPP_INCLUDED
#define GAME_END_EXCEPTIONS_HPP_INCLUDED

#include "lua_jailbreak_exception.hpp"

#include <string>

class config;

enum LEVEL_RESULT {
	NONE,
	VICTORY,
	DEFEAT,
	QUIT,
	OBSERVER_END,
	SKIP_TO_LINGER
};

/**
 * Exception used to signal the end of a player turn.
 */
class end_turn_exception
	: public tlua_jailbreak_exception
{
public:

	end_turn_exception(unsigned r = 0)
		: tlua_jailbreak_exception()
		, redo(r)
	{
	}

	unsigned redo;

private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(end_turn_exception)
};

/**
 * Exception used to signal the end of a scenario.
 */
class end_level_exception
	: public tlua_jailbreak_exception
{
public:

	end_level_exception(LEVEL_RESULT res)
		: tlua_jailbreak_exception()
		, result(res)
	{
	}

	LEVEL_RESULT result;

private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(end_level_exception)
};

/**
 * The non-persistent part of end_level_data
 */
struct transient_end_level{

	transient_end_level();

	bool carryover_report;             /**< Should a summary of the scenario outcome be displayed? */
	bool linger_mode;                  /**< Should linger mode be invoked? */
	std::string custom_endlevel_music; /**< Custom short music played at the end. */
	bool reveal_map;                   /**< Should we reveal map when game is ended? (Multiplayer only) */
	bool disabled;                     /**< Limits execution of tag [endlevel] to a single time > */
};

/**
 * Additional information on the game outcome which can be provided by WML.
 */
struct end_level_data
{
	end_level_data();


	bool prescenario_save;             /**< Should a prescenario be created the next game? */
	bool replay_save;                  /**< Should a replay save be made? */
	bool gold_bonus;                   /**< Should early-finish bonus be applied? */
	int carryover_percentage;          /**< How much gold is carried over to next scenario. */
	bool carryover_add;                /**< Add or replace next scenario's minimum starting gold. */
	transient_end_level transient;

	void write(config& cfg) const;

	void read(const config& cfg);
};

#endif /* ! GAME_END_EXCEPTIONS_HPP_INCLUDED */
