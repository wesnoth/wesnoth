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

namespace utils
{

/**
 * Reference points for the dates that define the storyline's calendar.
 */
struct wesnoth_epoch_defines
{
	static constexpr const char* const before_wesnoth = "BW";
	static constexpr const char* const wesnoth = "YW";
	static constexpr const char* const before_fall = "BF";
	static constexpr const char* const after_fall = "AF";

	ENUM_AND_ARRAY(before_wesnoth, wesnoth, before_fall, after_fall)
};
using wesnoth_epoch = string_enums::enum_base<wesnoth_epoch_defines>;

}
