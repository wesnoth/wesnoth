/*
   Copyright (C) 2014 - 2018 by Nathan Walker <nathan.b.walker@vanderbilt.edu>
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

#include "configure_engine.hpp"
#include "connect_engine.hpp"
#include "create_engine.hpp"
#include "game_launcher.hpp"

namespace sp
{
bool enter_create_mode(saved_game& state, jump_to_campaign_info jump_to);

bool enter_configure_mode(saved_game& state, ng::create_engine& create_eng);

void enter_connect_mode(saved_game& state);

} // end namespace sp
