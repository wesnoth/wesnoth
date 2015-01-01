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

#ifndef REPORTS_HPP_INCLUDED
#define REPORTS_HPP_INCLUDED

#include "map_location.hpp"

//this module is responsible for outputting textual reports of
//various game and unit statistics
namespace reports {

struct generator
{
	virtual config generate() = 0;
	virtual ~generator() {}
};

void reset_generators();
void register_generator(const std::string &name, generator *);

config generate_report(const std::string &name, bool only_static = false);

const std::set<std::string> &report_list();
}

#endif
