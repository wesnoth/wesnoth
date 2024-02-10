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

struct unit_alignments_defines
{
	static constexpr const char* const lawful = "lawful";
	static constexpr const char* const neutral = "neutral";
	static constexpr const char* const chaotic = "chaotic";
	static constexpr const char* const liminal = "liminal";

	ENUM_AND_ARRAY(lawful, neutral, chaotic, liminal)
};
using unit_alignments = string_enums::enum_base<unit_alignments_defines>;
