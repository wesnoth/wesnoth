/*
   Copyright (C) 2008 - 2016 by mark de wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PORTRAIT_HPP_INCLUDED
#define PORTRAIT_HPP_INCLUDED

class config;

#include <string>

/**
 * @todo this is a proof-of-concept version and undocumented. It should be
 * documented later and also allow WML to modify the portraits. This all
 * starts to make sense when the new dialogs become mainline.
 */

/** Contains the definition of a portrait for a unit(type). */
struct tportrait {

	tportrait(const config& cfg);

	enum tside { LEFT, RIGHT, BOTH };

	std::string image;
	tside side;
	unsigned size;
	bool mirror;
};

#endif
