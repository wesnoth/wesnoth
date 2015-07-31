/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ABOUT_H_INCLUDED
#define ABOUT_H_INCLUDED

#include "global.hpp"

class display;
class config;

#include <vector>
#include <string>

namespace about
{

void show_about(display &disp, const std::string &campaign = std::string());
void set_about(const config& cfg);
std::vector<std::string> get_text(const std::string &campaign = std::string(), bool split_multiline_headers = false);

}

#endif
