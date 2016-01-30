/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

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
 * Exit exception.
 */

#ifndef WESMAGE_EXIT_HPP_INCLUDED
#define WESMAGE_EXIT_HPP_INCLUDED

/**
 * This exception when throw should terminate the application.
 *
 * The application should terminate with @ref texit::status as its exit
 * status.
 */
struct texit
{
	texit(const int status__);

	/** The exit status for the application. */
	int status;
};


#endif
