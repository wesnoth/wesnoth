/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/testing.hpp
 * Gather statistics important for AI testing and output them
 */

#ifndef AI_TESTING_HPP_INCLUDED
#define AI_TESTING_HPP_INCLUDED

#include "../global.hpp"

class ai_testing{
public:
	/*
	 * Log at start of the turn
	*/
	static void log_turn_start();


	/*
	 * Log in case of draw
	 */
	static void log_draw();

	
	/*
	 * Log in case of victory
	 */
	static void log_victory();


	/*
	 * Log in case of unknown error while playing level
	 */
	static void log_unknown_error_while_playing_level();
};

#endif
