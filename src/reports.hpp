/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
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

struct report_data
{
	int viewing_side;
	int active_side;
	map_location displayed_unit_hex;
	bool show_everything;
};

struct generator
{
	virtual config generate(report_data const &) = 0;
	virtual ~generator() {}
};

void reset_generators();
void register_generator(const std::string &name, generator *, bool for_units);

config generate_report(const std::string &name, const report_data &data);

const std::set<std::string> &report_list(bool for_units);
}

#endif
