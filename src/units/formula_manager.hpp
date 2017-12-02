/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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

#include "formula/callable.hpp"

#include<string>

class config;
class unit;

class unit_formula_manager {
public:
	unit_formula_manager() {}
	unit_formula_manager(const unit_formula_manager & o) :
		unit_formula_(o.unit_formula_),
		unit_loop_formula_(o.unit_loop_formula_),
		unit_priority_formula_(o.unit_priority_formula_),
		formula_vars_(o.formula_vars_ ? std::make_shared<wfl::map_formula_callable>(*o.formula_vars_) : o.formula_vars_)
	{}

	const wfl::map_formula_callable_ptr& formula_vars() const { return formula_vars_; }
	void add_formula_var(std::string str, wfl::variant var);
	bool has_formula() const { return !unit_formula_.empty(); }
	bool has_loop_formula() const { return !unit_loop_formula_.empty(); }
	bool has_priority_formula() const { return !unit_priority_formula_.empty(); }
	const std::string& get_formula() const { return unit_formula_; }
	const std::string& get_loop_formula() const { return unit_loop_formula_; }
	const std::string& get_priority_formula() const { return unit_priority_formula_; }

	void read(const config & ai);
	void write(config & cfg);

private:
	std::string unit_formula_;
	std::string unit_loop_formula_;
	std::string unit_priority_formula_;
	wfl::map_formula_callable_ptr formula_vars_;
};
