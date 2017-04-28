/*
   Copyright (C) 2013 - 2017 by Felix Bauer <fehlxb+wesnoth@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UNIT_ADVANCEMENT_ACPECT_H_INCLUDED
#define UNIT_ADVANCEMENT_ACPECT_H_INCLUDED

#include <string>
#include <vector>

#include "units/map.hpp"

struct lua_State;

namespace ai {

class unit_advancements_aspect
{
public:
	unit_advancements_aspect();
	unit_advancements_aspect(lua_State* L, int n);
	unit_advancements_aspect(const std::string& val);
	const std::vector<std::string> get_advancements(const unit_map::const_iterator& unit) const;
	virtual ~unit_advancements_aspect();
	const std::string get_value() const;

private:
	std::string val_;
	lua_State * L_;
	int ref_;
};
}

#endif
