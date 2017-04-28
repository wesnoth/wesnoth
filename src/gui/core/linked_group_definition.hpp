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

#ifndef GUI_CORE_LINKED_GROUP_DEFINITION_HPP_INCLUDED
#define GUI_CORE_LINKED_GROUP_DEFINITION_HPP_INCLUDED

#include "config.hpp"
#include <string>
#include <vector>

namespace gui2
{

struct linked_group_definition
{
	linked_group_definition() : id(), fixed_width(false), fixed_height(false)
	{
	}

	linked_group_definition(const linked_group_definition& other)
		: id(other.id)
		, fixed_width(other.fixed_width)
		, fixed_height(other.fixed_height)
	{
	}

	std::string id;
	bool fixed_width;
	bool fixed_height;
};

std::vector<linked_group_definition> parse_linked_group_definitions(const config& cfg);

}

#endif
