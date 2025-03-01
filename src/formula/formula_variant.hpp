/*
	Copyright (C) 2008 - 2025
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

namespace wfl
{

struct formula_variant_defines
{
	static constexpr const char* const null = "null";
	static constexpr const char* const integer = "int";
	static constexpr const char* const decimal = "decimal";
	static constexpr const char* const object = "object";
	static constexpr const char* const list = "list";
	static constexpr const char* const string = "string";
	static constexpr const char* const map = "map";

	ENUM_AND_ARRAY(null, integer, decimal, object, list, string, map)
};
using formula_variant = string_enums::enum_base<formula_variant_defines>;

}
