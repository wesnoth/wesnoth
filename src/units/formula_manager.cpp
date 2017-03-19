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

#include "units/formula_manager.hpp"

#include "formula/callable_objects.hpp"
#include "config.hpp"
#include "formula/formula.hpp"
#include "formula/string_utils.hpp"
#include "map/location.hpp"
#include "log.hpp"

void unit_formula_manager::add_formula_var(std::string str, variant var)
{
	if(!formula_vars_) formula_vars_ = std::make_shared<game_logic::map_formula_callable>();
	formula_vars_->add(str, var);
}

void unit_formula_manager::read(const config & ai)
{
	unit_formula_ = ai["formula"].str();
	unit_loop_formula_ = ai["loop_formula"].str();
	unit_priority_formula_ = ai["priority"].str();

	if (const config &ai_vars = ai.child("vars"))
	{
		formula_vars_ = std::make_shared<game_logic::map_formula_callable>();

		variant var;
		for (const config::attribute &i : ai_vars.attribute_range()) {
			var.serialize_from_string(i.second);
			formula_vars_->add(i.first, var);
		}
	} else {
		formula_vars_ = game_logic::map_formula_callable_ptr();
	}
}

void unit_formula_manager::write(config & cfg)
{
	if ( has_formula() || has_loop_formula() || (formula_vars_ && formula_vars_->empty() == false) ) {

		config &ai = cfg.add_child("ai");

		if (has_formula())
			ai["formula"] = unit_formula_;

		if (has_loop_formula())
			ai["loop_formula"] = unit_loop_formula_;

		if (has_priority_formula())
			ai["priority"] = unit_priority_formula_;


		if (formula_vars_ && formula_vars_->empty() == false)
		{
			config &ai_vars = ai.add_child("vars");

			std::string str;
			for(game_logic::map_formula_callable::const_iterator i = formula_vars_->begin(); i != formula_vars_->end(); ++i)
			{
				i->second.serialize_to_string(str);
				if (!str.empty())
				{
					ai_vars[i->first] = str;
					str.clear();
				}
			}
		}
	}
}

