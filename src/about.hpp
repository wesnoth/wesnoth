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

class config;

#include <vector>
#include <string>

namespace about
{

/**
 * General getter methods for the credits config and image lists by campaign id
 */
const config& get_about_config();

std::vector<std::string> get_background_images(const std::string& campaign);

/**
 * Regenerates the credits config
 */
void set_about(const config& cfg);

}
