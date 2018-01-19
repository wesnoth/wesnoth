/*
   Copyright (C) 2010 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "variable.hpp"

#include <set>
#include <string>
#include <vector>

class config;
class filter_context;
class unit;
class unit_filter;
class team;

//side_filter: a class that implements the Standard Side Filter
class side_filter {
public:

	~side_filter();

	side_filter(const std::string &side_string, const filter_context * fc, bool flat_tod = false);
	side_filter(const vconfig &cfg, const filter_context * fc, bool flat_tod = false);

	//match: returns true if and only if the given team matches this filter
	bool match(const team& t) const;
	bool match(const int side) const;
	std::vector<int> get_teams() const;

private:
	side_filter(const side_filter &other);
	side_filter& operator=(const side_filter &other);

	bool match_internal(const team& t) const;

	const vconfig cfg_; //config contains WML for a Standard Side Filter

	bool flat_;
	std::string side_string_;

	const filter_context * fc_; //!< The filter context for this filter. It should be a pointer because otherwise the default ctor doesn't work

	mutable std::unique_ptr<unit_filter> ufilter_;
	mutable std::unique_ptr<side_filter> allied_filter_;
	mutable std::unique_ptr<side_filter> enemy_filter_;
	mutable std::unique_ptr<side_filter> has_ally_filter_;
	mutable std::unique_ptr<side_filter> has_enemy_filter_;
};
