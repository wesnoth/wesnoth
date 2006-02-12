/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ABOUT_H_INCLUDED
#define ABOUT_H_INCLUDED

#include "global.hpp"
#include "config.hpp"
class display;

#include <vector>
#include <string>

namespace about
{

extern config about_list;

void show_about(display &disp);
void set_about(const config& cfg);
std::vector<std::string> get_text();

}

#endif
