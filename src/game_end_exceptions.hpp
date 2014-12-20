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

#include "config.hpp"
#include "make_enum.hpp"

#include <string>

#include <boost/optional.hpp>
#include <boost/variant/variant.hpp>

MAKE_ENUM(LEVEL_RESULT,
	(NONE,		"none")
	(VICTORY,	"victory")
	(DEFEAT,	"defeat")
	(QUIT,		"quit")
	(OBSERVER_END,	"observer_end")
	(SKIP_TO_LINGER,"skip_to_linger")
)
MAKE_ENUM_STREAM_OPS1(LEVEL_RESULT)

/**
 * Struct used to transmit info caught from an end_turn_exception.
 */
struct end_turn_struct {
	unsigned redo;
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

	end_turn_struct to_struct() {
		end_turn_struct ets = {redo};
		return ets;
	}

private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(end_turn_exception)
};

/**
 * Struct used to transmit info caught from an end_turn_exception.
 */
struct end_level_struct {
	LEVEL_RESULT result;
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

	end_level_struct to_struct() {
		end_level_struct els = {result};
		return els;
	}

private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(end_level_exception)
};

/**
 * The two end_*_exceptions are caught and transformed to this signaling object
 */
typedef boost::optional<boost::variant<end_turn_struct, end_level_struct> > possible_end_play_signal;

#define HANDLE_END_PLAY_SIGNAL( X )\
do\
{\
	try {\
		X;\
	} catch (end_level_exception & e) {\
		return possible_end_play_signal(e.to_struct());\
	} catch (end_turn_exception & e) {\
		return possible_end_play_signal(e.to_struct());\
	}\
}\
while(0)



#define PROPOGATE_END_PLAY_SIGNAL( X )\
do\
{\
	possible_end_play_signal temp;\
	temp = X;\
	if (temp) {\
		return temp;\
	}\
}\
while(0)



#define HANDLE_AND_PROPOGATE_END_PLAY_SIGNAL( X )\
do\
{\
	possible_end_play_signal temp;\
	HANDLE_END_PLAY_SIGNAL( temp = X );\
	if (temp) {\
		return temp;\
	}\
}\
while(0)


enum END_PLAY_SIGNAL_TYPE {END_TURN, END_LEVEL};

class get_signal_type : public boost::static_visitor<END_PLAY_SIGNAL_TYPE> {
public:
	END_PLAY_SIGNAL_TYPE operator()(end_turn_struct &) const
	{
		return END_TURN;
	}

	END_PLAY_SIGNAL_TYPE operator()(end_level_struct& ) const
	{
		return END_LEVEL;
	}
};

class get_redo : public boost::static_visitor<unsigned> {
public:
	unsigned operator()(end_turn_struct & s) const
	{
		return s.redo;
	}

	unsigned operator()(end_level_struct &) const
	{
		return 0;
	}
};

class get_result : public boost::static_visitor<LEVEL_RESULT> {
public:
	LEVEL_RESULT operator()(end_turn_struct & ) const
	{
		return NONE;
	}

	LEVEL_RESULT operator()(end_level_struct & s) const
	{
		return s.result;
	}
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
	bool proceed_to_next_level;        /**< whether to proceed to the next scenario, equals (res == VICTORY) in sp. We need to save this in saves during linger mode. > */
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

#endif /* ! GAME_END_EXCEPTIONS_HPP_INCLUDED */
