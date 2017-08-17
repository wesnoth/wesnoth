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

#include "units/filter.hpp"

#include "log.hpp"

#include "config.hpp"
#include "game_data.hpp"
#include "map/map.hpp"
#include "map/location.hpp"
#include "scripting/game_lua_kernel.hpp" //Needed for lua kernel
#include "side_filter.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "tod_manager.hpp"
#include "units/unit.hpp"
#include "units/formula_manager.hpp"
#include "units/types.hpp"
#include "variable.hpp" // needed for vconfig, scoped unit
#include "wml_exception.hpp" // needed for FAIL
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/function_gamestate.hpp"
#include "formula/string_utils.hpp"
#include "resources.hpp"

#include <boost/optional.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

using namespace unit_filter_impl;

namespace {

struct unit_filter_xy : public unit_filter_base
{
	unit_filter_xy(const std::string& x, const std::string& y) : x_(x), y_(y) {}

	virtual bool matches(const unit_filter_args& u) const override
	{
		std::string x = utils::interpolate_variables_into_string(x_, *(resources::gamedata));
		std::string y = utils::interpolate_variables_into_string(y_, *(resources::gamedata));

		if(x == "recall" && y == "recall") {
			return !u.fc->get_disp_context().map().on_board(u.loc);
		}
		else {
			return u.loc.matches_range(x, y);
		}
	}

	const std::string x_;
	const std::string y_;
};

struct unit_filter_adjacent : public unit_filter_base
{
	unit_filter_adjacent(const vconfig& cfg)
		: child_(cfg)
		, cfg_(cfg)
	{
	}

	virtual bool matches(const unit_filter_args& u) const override
	{
		const unit_map& units = u.fc->get_disp_context().units();
		map_location adjacent[6];
		get_adjacent_tiles(u.loc, adjacent);
		int match_count=0;

		config::attribute_value i_adjacent = cfg_["adjacent"];
		std::vector<map_location::DIRECTION> dirs;
		if (i_adjacent.empty()) {
			dirs = map_location::default_dirs();
		} else {
			dirs = map_location::parse_directions(i_adjacent);
		}
		for (map_location::DIRECTION dir : dirs) {
			unit_map::const_iterator unit_itor = units.find(adjacent[dir]);			
			if (unit_itor == units.end() || !child_.matches(unit_filter_args{*unit_itor, unit_itor->get_location(), &u.u, u.fc, u.use_flat_tod} )) {
				continue;
			}
			auto is_enemy = cfg_["is_enemy"];
			if (!is_enemy.empty() && is_enemy.to_bool() != u.fc->get_disp_context().get_team(u.u.side()).is_enemy(unit_itor->side())) {
				continue;
			}
			++match_count;
		}

		static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
		config::attribute_value i_count = cfg_["count"];
		return in_ranges(match_count, !i_count.blank() ? utils::parse_ranges(i_count) : default_counts);
	}

	const unit_filter_compound child_;
	const vconfig cfg_;
};


template<typename F>
struct unit_filter_child_literal : public unit_filter_base
{
	unit_filter_child_literal(const vconfig& v, const F& f) : v_(v) , f_(f) {}
	virtual bool matches(const unit_filter_args& u) const override
	{
		return f_(v_, u);
	}
	vconfig v_;
	F f_;
};

template<typename T, typename F>
struct unit_filter_attribute_parsed : public unit_filter_base
{
	unit_filter_attribute_parsed(T&& v, F&& f) : v_(std::move(v)), f_(std::move(f)) {}
	virtual bool matches(const unit_filter_args& u) const override
	{
		return f_(v_, u);
	}
	T v_;
	F f_;
};

template<typename C, typename F>
struct unit_filter_attribute_literal : public unit_filter_base
{
	unit_filter_attribute_literal(std::string&& v, C&& c, F&& f) : v_(std::move(v)), c_(std::move(c)), f_(std::move(f)) {}
	virtual bool matches(const unit_filter_args& u) const override
	{
		config::attribute_value v;
		v = utils::interpolate_variables_into_string(v_, *(resources::gamedata));
		return f_(c_(v), u);
	}
	std::string v_;
	C c_;
	F f_;
};

class contains_dollar_visitor : public boost::static_visitor<bool>
{
public:
	contains_dollar_visitor() {}

	
	template<typename T>
	bool operator()(T const &) const { return false; }

	bool operator()(t_string const&)    const { return true; }

	bool operator()(const std::string& s) const
	{
		return s.find('$') != std::string::npos;
	}
};

}


unit_filter_compound::unit_filter_compound(vconfig cfg)
	: children_()
	, cond_children_()
{
	fill(cfg);
}

bool unit_filter_compound::matches(const unit_filter_args& u) const
{
	bool res;

	if(u.loc.valid()) {
		scoped_xy_unit auto_store("this_unit", u.loc, u.fc->get_disp_context().units());
		if (u.u2) {
			const map_location& loc2 = u.u2->get_location();
			scoped_xy_unit u2_auto_store("other_unit", loc2, u.fc->get_disp_context().units());
			res = filter_impl(u);
		} else {
			res = filter_impl(u);
		}
	} else {
		// If loc is invalid, then this is a recall list unit (already been scoped)
		res = filter_impl(u);
	}

	// Handle [and], [or], and [not] with in-order precedence
	for(const auto & filter : cond_children_) {
		bool child_res = filter.second.matches(u);
		
		switch (filter.first.v) {
		case CONDITIONAL_TYPE::AND:
			res = res && child_res;
			break;
		case CONDITIONAL_TYPE::OR:
			res = res || child_res;
			break;
		case CONDITIONAL_TYPE::NOT:
			res = res && !child_res;
			break;
		}
	}
	return res;
}

bool unit_filter_compound::filter_impl(const unit_filter_args& u) const
{
	for(const auto & filter : children_) {
		if (!filter->matches(u)) {
			return false;
		}
	}
	return true;
}

template<typename F>
void unit_filter_compound::create_child(const vconfig& c, F func)
{
	children_.emplace_back(new unit_filter_child_literal<F>(c, func));
}

template<typename C, typename F>
void unit_filter_compound::create_attribute(const config::attribute_value v, C conv, F func)
{
	if(v.empty()) {
	}
	else if(v.apply_visitor(contains_dollar_visitor())) {
		children_.emplace_back(new unit_filter_attribute_literal<C, F>(std::move(v.str()), std::move(conv), std::move(func)));
	}
	else {
		children_.emplace_back(new unit_filter_attribute_parsed<decltype(conv(v)), F>(std::move(conv(v)), std::move(func)));
	}
}

void unit_filter_compound::fill(vconfig cfg)
	{
		const config& literal = cfg.get_config();

		//optimisation
		if(literal.empty()) { return; }

		create_attribute(literal["name"],
			[](const config::attribute_value& c) { return c.t_str(); },
			[](const t_string& str, const unit_filter_args& u) { return str == u.u.name(); }
		);

		create_attribute(literal["id"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& id_list, const unit_filter_args& u)
			{
				return std::find(id_list.begin(), id_list.end(), u.u.id()) != id_list.end();
			}
		);

		create_attribute(literal["speaker"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& speaker, const unit_filter_args& u)
			{
				return speaker == u.u.id();
			}
		);

		create_attribute(literal["type"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& types, const unit_filter_args& u)
			{
				return std::find(types.begin(), types.end(), u.u.type_id()) != types.end();
			}
		);

		create_attribute(literal["type_adv_tree"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& types, const unit_filter_args& u)
			{
				std::set<std::string> types_expanded;
				for(const std::string& type : types) {
					if(types_expanded.count(type)) {
						continue;
					}
					if(const unit_type* ut = unit_types.find(type)) {
						const auto& tree = ut->advancement_tree();
						types_expanded.insert(tree.begin(), tree.end());
						types_expanded.insert(type);
					}
				}
				return types_expanded.find(u.u.type_id()) != types_expanded.end();
			}
		);

		create_attribute(literal["variation"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& types, const unit_filter_args& u)
			{
				return std::find(types.begin(), types.end(), u.u.variation()) != types.end();
			}
		);

		create_attribute(literal["has_variation"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& types, const unit_filter_args& u)
			{
				// If this unit is a variation itself then search in the base unit's variations.
				const unit_type* const type = u.u.variation().empty() ? &u.u.type() : unit_types.find(u.u.type().base_id());
				assert(type);

				for(const std::string& variation_id : types) {
					if (type->has_variation(variation_id)) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["ability"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& abilities, const unit_filter_args& u)
			{
				for(const std::string& ability_id : abilities) {
					if (u.u.has_ability_by_id(ability_id)) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["ability_type"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& abilities, const unit_filter_args& u)
			{
				for(const std::string& ability : abilities) {
					if (u.u.has_ability_type(ability)) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["ability_type_active"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& abilities, const unit_filter_args& u)
			{
				for(const std::string& ability : abilities) {
					if (!u.u.get_abilities(ability, u.loc).empty()) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["trait"],
			[](const config::attribute_value& c)
			{
				auto res = utils::split(c.str());
				std::sort(res.begin(), res.end());
				return res;

			},
			[](const std::vector<std::string>& check_traits, const unit_filter_args& u)
			{

				std::vector<std::string> have_traits = u.u.get_traits_list();
				std::vector<std::string> isect;
				std::sort(have_traits.begin(), have_traits.end());
				std::set_intersection(check_traits.begin(), check_traits.end(), have_traits.begin(), have_traits.end(), std::back_inserter(isect));
				return !isect.empty();
			}
		);

		create_attribute(literal["race"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& races, const unit_filter_args& u)
			{
				return std::find(races.begin(), races.end(), u.u.race()->id()) != races.end();
			}
		);

		create_attribute(literal["gender"],
			[](const config::attribute_value& c) { return string_gender(c.str()); },
			[](unit_race::GENDER gender, const unit_filter_args& u)
			{
				return gender == u.u.gender();
			}
		);

		create_attribute(literal["side"],
			[](const config::attribute_value& c)
			{
				std::vector<int> res;
				for(const std::string& s : utils::split(c.str())) {
					res.push_back(std::stoi(s));
				}
				return res;
			},
			[](const std::vector<int>& sides, const unit_filter_args& u)
			{
				return std::find(sides.begin(), sides.end(), u.u.side()) != sides.end();
			}
		);

		create_attribute(literal["status"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& statuses, const unit_filter_args& u)
			{
				for(const std::string& status : statuses) {
					if (u.u.get_state(status)) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["has_weapon"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& weapon, const unit_filter_args& u)
			{

				for(const attack_type& a : u.u.attacks()) {
					if(a.id() == weapon) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["role"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& role, const unit_filter_args& u)
			{
				return u.u.get_role() == role;
			}
		);

		create_attribute(literal["ai_special"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& ai_special, const unit_filter_args& u)
			{
				return (ai_special == "guardian") == u.u.get_state(unit::STATE_GUARDIAN);
			}
		);

		create_attribute(literal["canrecruit"],
			[](const config::attribute_value& c) { return c.to_bool(); },
			[](bool canrecruit, const unit_filter_args& u)
			{
				return u.u.can_recruit() == canrecruit;
			}
		);

		create_attribute(literal["recall_cost"],
			[](const config::attribute_value& c) { return utils::parse_ranges(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& u)
			{
				for(auto cost : ranges) {
					if(cost.first <= u.u.recall_cost() && u.u.recall_cost() <= cost.second) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["level"],
			[](const config::attribute_value& c) { return utils::parse_ranges(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& u)
			{
				for(auto lvl : ranges) {
					if(lvl.first <= u.u.level() && u.u.level() <= lvl.second) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["defense"],
			[](const config::attribute_value& c) { return utils::parse_ranges(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& u)
			{
				int actual_defense = u.u.defense_modifier(u.fc->get_disp_context().map().get_terrain(u.loc));
				for(auto def : ranges) {
					if(def.first <= actual_defense && actual_defense <= def.second) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["movement_cost"],
			[](const config::attribute_value& c) { return utils::parse_ranges(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& u)
			{
				int actual_cost = u.u.movement_cost(u.fc->get_disp_context().map().get_terrain(u.loc));
				for(auto cost : ranges) {
					if(cost.first <= actual_cost && actual_cost <= cost.second) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["vision_cost"],
			[](const config::attribute_value& c) { return utils::parse_ranges(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& u)
			{
				int actual_cost = u.u.vision_cost(u.fc->get_disp_context().map().get_terrain(u.loc));
				for(auto cost : ranges) {
					if(cost.first <= actual_cost && actual_cost <= cost.second) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["jamming_cost"],
			[](const config::attribute_value& c) { return utils::parse_ranges(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& u)
			{
				int actual_cost = u.u.jamming_cost(u.fc->get_disp_context().map().get_terrain(u.loc));
				for(auto cost : ranges) {
					if(cost.first <= actual_cost && actual_cost <= cost.second) {
						return true;
					}
				}
				return false;
			}
		);
		
		create_attribute(literal["lua_function"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& lua_function, const unit_filter_args& u)
			{
				if (game_lua_kernel * lk = u.fc->get_lua_kernel()) {
					return lk->run_filter(lua_function.c_str(), u.u);
				}
				return true;
			}
		);

		create_attribute(literal["formula"],
			[](const config::attribute_value& c)
			{
				//TODO: catch syntax error.
				return wfl::formula(c, new wfl::gamestate_function_symbol_table());
			},
			[](const wfl::formula& form, const unit_filter_args& u)
			{
				try {
					const wfl::unit_callable main(u.loc, u.u);
					wfl::map_formula_callable callable(main.fake_ptr());
					if (u.u2) {
						std::shared_ptr<wfl::unit_callable> secondary(new wfl::unit_callable(*u.u2));
						callable.add("other", wfl::variant(secondary));
						// It's not destroyed upon scope exit because the variant holds a reference
					}
					if(!form.evaluate(callable).as_bool()) {
						return false;
					}
					return true;
				} catch(wfl::formula_error& e) {
					lg::wml_error() << "Formula error in unit filter: " << e.type << " at " << e.filename << ':' << e.line << ")\n";
					// Formulae with syntax errors match nothing
					return false;
				}
			}
		);
		
		create_attribute(literal["find_in"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& find_in, const unit_filter_args& u)
			{
				// Allow filtering by searching a stored variable of units
				if (const game_data * gd = u.fc->get_game_data()) {
					try
					{
						for (const config& c : gd->get_variable_access_read(find_in).as_array())
						{
							if(c["id"] == u.u.id()) {
								return true;
							}
						}
						return false;
					}
					catch(const invalid_variablename_exception&)
					{
						return false;
					}
				}
				return true;
			}
		);

		if (!literal["x"].empty() || !literal["y"].empty()) {
			children_.emplace_back(new unit_filter_xy(literal["x"], literal["y"]));
		}
		
		for(auto child : cfg.all_ordered()) {
			CONDITIONAL_TYPE cond;
			if(cond.parse(child.first)) {
				cond_children_.emplace_back(std::piecewise_construct_t(), std::make_tuple(cond), std::make_tuple(child.second));
			}
			else if (child.first == "filter_wml") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& u) {
					config fwml = c.get_parsed_config();

					/* Check if the filter only cares about variables.
					   If so, no need to serialize the whole unit. */
					config::all_children_itors ci = fwml.all_children_range();
					if (fwml.all_children_count() == 1 && fwml.attribute_count() == 1 && ci.front().key == "variables") {
						return u.u.variables().matches(ci.front().cfg);
					} else {
						config ucfg;
						u.u.write(ucfg);
						return ucfg.matches(fwml);
					}
				});
			}
			else if (child.first == "filter_vision") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& u) {
					std::set<int> viewers;
					side_filter ssf(c, u.fc);
					std::vector<int> sides = ssf.get_teams();
					viewers.insert(sides.begin(), sides.end());

					for (const int viewer : viewers) {
						bool fogged = u.fc->get_disp_context().get_team(viewer).fogged(u.loc);
						bool hiding = u.u.invisible(u.loc, u.fc->get_disp_context()) && u.fc->get_disp_context().get_team(viewer).is_enemy(u.u.side());
						bool unit_hidden = fogged || hiding;
						if (c["visible"].to_bool(true) != unit_hidden) {
							return true;
						}
					}
					return false;
				});
			}
			else if (child.first == "filter_adjacent") {
				children_.emplace_back(new unit_filter_adjacent(child.second));
			}
			else if (child.first == "filter_location") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& u) {
					return terrain_filter(c, u.fc, u.use_flat_tod).match(u.loc);
				});
			}
			else if (child.first == "filter_side") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& u) {
					return side_filter(c, u.fc).match(u.u.side());
				});
			}
			else if (child.first == "has_attack") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& u) {
					for(const attack_type& a : u.u.attacks()) {
						if(a.matches_filter(c.get_parsed_config())) {
							return true;
						}
					}
					return false;
				});
			}
			else {
				std::stringstream errmsg;
				errmsg << "encountered a child [" << child.first << "] of a standard unit filter, it is being ignored";
				DBG_CF << errmsg.str() << std::endl;
			}
			
		}
	}



