/*
	Copyright (C) 2022 - 2022
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
class game_state;
/// calculates the amount of gold carried over for each team,
/// stores the data in the team object and shows the carryover message
void carryover_show_gold(game_state& state, bool hidden, bool is_observer, bool is_test);
