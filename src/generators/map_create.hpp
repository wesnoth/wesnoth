/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

class config;
class map_generator;

#include <string>

std::string random_generate_map(const std::string& parms, const config &cfg);
config random_generate_scenario(const std::string& parms, const config &cfg);

map_generator* create_map_generator(const std::string& name, const config &cfg);
