/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

namespace gui2
{

/**
 * Small abstract helper class.
 *
 * Parts of the engine inherit this class so we can have generic
 * widgets to select an integer value.
 */
class integer_selector
{
public:
	virtual ~integer_selector()
	{
	}

	/** Sets the selected value. */
	virtual void set_value(int value) = 0;

	/** Gets the selected value. */
	virtual int get_value() const = 0;

	/** Gets the minimum value. */
	virtual int get_minimum_value() const = 0;

	/** Gets the maximum value. */
	virtual int get_maximum_value() const = 0;
};

} // namespace gui2
