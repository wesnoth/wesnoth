/*
   Copyright (C) 2021 by demario
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

#include "config.hpp"

namespace savegame {
/** converts saves from older versions of wesnoth*/
void convert_earlier_version_saves(config& cfg);

std::string append_suffix_to_era_id(config& cfg);

config& replay_start_config(config& cfg);

} //end of namespace savegame
