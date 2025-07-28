/*
	Copyright (C) 2014 - 2025
	by Nathan Walker <nathan.b.walker@vanderbilt.edu>
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

struct jump_to_campaign_info;
class saved_game;

namespace ng
{
class create_engine;
}

namespace sp
{
bool select_campaign(saved_game& state, jump_to_campaign_info jump_to);

bool configure_campaign(saved_game& state, ng::create_engine& create_eng);

} // end namespace sp
