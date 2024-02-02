/*
	Copyright (C) 2008 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "enum_base.hpp"

namespace unit_filter_impl
{

struct conditional_type_defines
{
	static constexpr const char* const filter_and = "and";
	static constexpr const char* const filter_or = "or";
	static constexpr const char* const filter_not = "not";

	ENUM_AND_ARRAY(filter_and, filter_or, filter_not)
};
using conditional_type = string_enums::enum_base<conditional_type_defines>;

}
