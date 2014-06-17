/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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
 *
 * This wrapper class should be held by the display object when it needs to draw a unit.
 * The purpose of this is to improve encapsulation -- other parts of the engine like AI
 * don't need to be exposed to the unit drawing code, and encapsulation like this will
 * help us to reduce unnecessary includes.
 *
 **/

#ifndef DRAWABLE_UNIT_H_INCLUDED
#define DRAWABLE_UNIT_H_INCLUDED

#include "unit.hpp"

class display;
class game_display;

class drawable_unit : protected unit //TODO: Get rid of inheritance and use composition instead.
{				     //IMO, it would be better for drawable unit to hold a unit reference, and be marked as a friend class. 
				     //But I don't want to rewrite the redraw() function right now.
	/** draw a unit.  */
	void redraw_unit() const;

	friend class display;
	friend class game_display;
};
#endif
