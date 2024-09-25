/*
	Copyright (C) 2009 - 2024
	by Yurii Chernyi <terraninfo@terraninfo.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * Gather statistics important for AI testing and output them
 */

#pragma once


class ai_testing{
public:
	/*
	 * Log at start of the turn
	*/
	static void log_turn_start( unsigned int side );

	/*
	 * Log at end of the turn
	*/
	static void log_turn_end( unsigned int side );

	/*
	 * Log in case of draw
	 */
	static void log_draw();

	/*
	 * Log in case of victory
	 * teams vector of winner teams
	 */
	static void log_victory( const std::set<unsigned int>& teams );

	/*
	 * Log at game start
	 */
	static void log_game_start();

	/*
	 * Log at game end
	 */
	static void log_game_end();

protected:

	static void log_turn( const char *msg, unsigned int side );

};
