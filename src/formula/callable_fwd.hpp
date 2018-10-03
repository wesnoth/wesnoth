/*
   Copyright (C) 2008 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <memory>
#include <vector>
#include <string>

namespace wfl
{
class formula_callable;
class formula_debugger;

struct callable_die_subscriber {
	virtual void notify_dead() {}
	virtual ~callable_die_subscriber() {}
};

enum FORMULA_ACCESS_TYPE { FORMULA_READ_ONLY, FORMULA_WRITE_ONLY, FORMULA_READ_WRITE };

struct formula_input {
	explicit formula_input(const std::string& name, FORMULA_ACCESS_TYPE access = FORMULA_READ_WRITE)
		: name(name), access(access) {}

	std::string name;
	FORMULA_ACCESS_TYPE access;
};

using formula_input_vector = std::vector<formula_input>;
using formula_callable_ptr       = std::shared_ptr<formula_callable>;
using const_formula_callable_ptr = std::shared_ptr<const formula_callable>;
using formula_seen_stack = std::vector<const_formula_callable_ptr>;

}
