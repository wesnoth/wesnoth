/*
   Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gettext.hpp"
#include "tstring.hpp"

struct terrain_movement
{
	const t_string& name;
	int moves;

	terrain_movement(const t_string& name_, int moves_)
		: name(name_), moves(moves_)
	{}

	bool operator<(const terrain_movement& other) const
	{
		return translation::icompare(name, other.name) < 0;
	}
};
