/*
	Copyright (C) 2020 - 2021
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

#include "gettext.hpp"
#include "utils/make_enum.hpp"

MAKE_ENUM (UNIT_ALIGNMENT,
	(LAWFUL, N_("lawful"))
	(NEUTRAL, N_("neutral"))
	(CHAOTIC, N_("chaotic"))
	(LIMINAL, N_("liminal"))
)
