/* $Id$ */
/*
   Copyright (C) 2006 - 2008 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file game_end_exceptions.hpp
 * Contains the exception interfaces used to signal
 * completion of a scenario, campaign or turn.
 */

#ifndef GAME_END_EXCEPTIONS_HPP_INCLUDED
#define GAME_END_EXCEPTIONS_HPP_INCLUDED

#include <string>

enum LEVEL_RESULT {
	VICTORY,
	DEFEAT,
	QUIT,
	OBSERVER_END,
	SKIP_TO_LINGER
};

#define DELAY_END_LEVEL(end_ptr, code) try { \
	code; \
	} catch ( end_level_exception &e) { \
		if (end_ptr == 0) { \
			end_ptr = new end_level_exception(e); \
		} \
	}

#define THROW_END_LEVEL_DELETE(end_ptr) if (end_ptr) {\
	end_level_exception temp_exception(*end_ptr);\
	delete end_ptr; \
	throw temp_exception; \
	}

#define THROW_END_LEVEL(end_ptr) if (end_ptr) {\
	throw end_level_exception(*end_ptr); \
	}

/**
 * Exception used to signal the end of a scenario.
 * It also conveys additional information on the game
 * outcomes which can be provided by WML.
 */
struct end_level_exception {
	end_level_exception(
		LEVEL_RESULT res,
		const std::string& endlevel_music_list="",
		const int percentage = -1,
		const bool add = false,
		const bool bonus=true,
		const bool report=true,
		const bool prescenario_save=true,
		const bool linger=true
	)
		: result(res)
		, carryover_report(report)
		, save(prescenario_save)
		, linger_mode(linger)
		, gold_bonus(bonus)
		, carryover_percentage(percentage)
		, carryover_add(add)
		, custom_endlevel_music(endlevel_music_list)
	{}

	LEVEL_RESULT result;               /**< Game outcome. */
	bool carryover_report;			   /**< Should a summary of the scenario outcome be displayed?*/
	bool save;						   /**< Should a prescenario be created the next game?*/
	bool linger_mode;				   /**< Should linger mode be invoked?*/
	bool gold_bonus;                   /**< Should early-finish bonus be applied? */
	int carryover_percentage;          /**< How much gold is carried over to next scenario. */
	bool carryover_add;                /**< Add or replace next scenario's minimum starting gold. */
	std::string custom_endlevel_music; /**< Custom short music played at the end. */
};

/**
 * Signals the end of a player turn.
 */
struct end_turn_exception {
	end_turn_exception(unsigned int r = 0): redo(r) {}
	unsigned int redo;
};

#endif /* ! GAME_END_EXCEPTIONS_HPP_INCLUDED */
