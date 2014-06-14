#include "unit_formula_manager.hpp"

#include "callable_objects.hpp"
#include "config.hpp"
#include "formula.hpp"
#include "formula_string_utils.hpp"
#include "map_location.hpp"

#include <boost/foreach.hpp>

bool unit_formula_manager::matches_filter(const std::string & cfg_formula, const map_location & loc, const unit & me)
{
	const unit_callable callable(loc,me);
	const game_logic::formula form(cfg_formula);
	if(!form.evaluate(callable).as_bool()) {///@todo use formula_ai
		return false;
	}
	return true;
}

void unit_formula_manager::add_formula_var(std::string str, variant var)
{
	if(!formula_vars_) formula_vars_ = new game_logic::map_formula_callable;
	formula_vars_->add(str, var);
}

void unit_formula_manager::read(const config & ai)
{
	unit_formula_ = ai["formula"].str();
	unit_loop_formula_ = ai["loop_formula"].str();
	unit_priority_formula_ = ai["priority"].str();

	if (const config &ai_vars = ai.child("vars"))
	{
		formula_vars_ = new game_logic::map_formula_callable;

		variant var;
		BOOST_FOREACH(const config::attribute &i, ai_vars.attribute_range()) {
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

