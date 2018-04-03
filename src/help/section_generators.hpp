/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <string>

namespace help
{
class section;

void generate_races_sections(section& sec);
void generate_terrain_sections(section& sec);
void generate_era_sections(section& sec);
void generate_unit_sections(section& sec, const std::string& race);

} // namespace help
