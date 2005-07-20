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

#include "player.hpp"

player::player(const std::string& n, config& cfg) : name_(n), cfg_(cfg)
{
	cfg_["name"] = n;
	mark_available(true);
}

void player::mark_available(bool val)
{
	cfg_.values["available"] = (val ? "yes" : "no");
}

const std::string& player::name() const
{
	return name_;
}

config* player::config_address()
{
	return &cfg_;
}
