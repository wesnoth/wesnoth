/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#include "../game_events.hpp"

//this is an 'identity' implementation of variables, that just returns the name
//of the variable itself. To be used in systems for which variables shouldn't be implemented
namespace {
std::map<std::string,std::string> variables;

}

namespace game_events {
const std::string& get_variable_const(const std::string& str)
{
	const std::map<std::string,std::string>::const_iterator itor = variables.find(str);
	if(itor != variables.end()) {
		return itor->second;
	} else {
		variables[str] = "$" + str;
		return get_variable_const(str);
	}
}
}

