/*
   Copyright (C) 2006 - 2016 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "config.hpp"
#include "make_enum.hpp"

#include <string>
#include <exception>
#include <boost/optional.hpp>

MAKE_ENUM(LEVEL_RESULT,
	(VICTORY,      "victory")
	(DEFEAT,       "defeat")
	(QUIT,         "quit")
	(OBSERVER_END, "observer_end")
)

/**
 * Exception used to escape form the ai or ui code to playsingle_controller::play_side.
 * Never thrown during replays.
 */
class return_to_play_side_exception : public tlua_jailbreak_exception, public std::exception
{
public:

	return_to_play_side_exception()
		: tlua_jailbreak_exception()
		, std::exception()
	{
	}
	const char * what() const throw() { return "return_to_play_side_exception"; }
private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(return_to_play_side_exception)
};

class quit_game_exception
	: public tlua_jailbreak_exception
	, public std::exception
{
public:

	quit_game_exception()
		: tlua_jailbreak_exception()
		, std::exception()
	{
	}
	const char * what() const throw() { return "quit_game_exception"; }
private:
	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(quit_game_exception)
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
};

/**
 * Additional information on the game outcome which can be provided by WML.
 */
struct end_level_data
{
	end_level_data();


	bool prescenario_save;             /**< Should a prescenario be created the next game? */
	bool replay_save;                  /**< Should a replay save be made? */
	bool proceed_to_next_level;        /**< whether to proceed to the next scenario, equals is_victory in sp. We need to save this in saves during linger mode. > */
	bool is_victory;
	transient_end_level transient;
	void write(config& cfg) const;

	void read(const config& cfg);

	config to_config() const
	{
		config r;
		write(r);
		return r;
	}
};
inline void throw_quit_game_exception()
{
	// Distinguish 'Quit' from 'Regular' end_level_exceptions to solve the following problem:
	//   If a player quits the game during an event after an [endlevel] occurs, the game won't
	//   Quit but continue with the [endlevel] instead.
	throw quit_game_exception();
}
#endif /* ! GAME_END_EXCEPTIONS_HPP_INCLUDED */
