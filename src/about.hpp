/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ABOUT_H_INCLUDED
#define ABOUT_H_INCLUDED

class display;

#include <vector>
#include <string>

namespace about
{

void show_about(display& disp);
std::vector<std::string> get_text();

}

#endif
