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
#ifndef VARIABLE_H_INCLUDED
#define VARIABLE_H_INCLUDED

#include "config.hpp"

namespace game_events {
std::string& get_variable(const std::string& varname);
const std::string& get_variable_const(const std::string& varname);
config& get_variable_cfg(const std::string& varname);

void set_variable(const std::string& varname, const std::string& value);
}

#endif
