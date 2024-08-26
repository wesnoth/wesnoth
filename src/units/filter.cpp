/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "display.hpp"
#include "display_context.hpp"
#include "config.hpp"
#include "game_data.hpp"
#include "map/map.hpp"
#include "map/location.hpp"
#include "scripting/game_lua_kernel.hpp" //Needed for lua kernel
#include "side_filter.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/types.hpp"
#include "variable.hpp" // needed for vconfig, scoped unit
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/function_gamestate.hpp"
#include "formula/string_utils.hpp"
#include "resources.hpp"

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)

using namespace unit_filter_impl;

unit_filter::unit_filter(vconfig cfg)
	: cfg_(cfg)
	, fc_(resources::filter_con)
	, use_flat_tod_(false)
	, impl_(cfg_)
	, max_matches_(-1)
{
}

bool unit_filter::matches(const unit& u) const {
	return impl_.matches(unit_filter_impl::unit_filter_args{u, u.get_location(), nullptr, fc_, use_flat_tod_});
}

bool unit_filter::matches(const unit & u, const unit & u2) const {
	return impl_.matches(unit_filter_impl::unit_filter_args{u, u.get_location(), &u2, fc_, use_flat_tod_});
}

std::vector<const unit *> unit_filter::all_matches_on_map(const map_location* loc, const unit* other_unit) const
{
	std::vector<const unit *> ret;
	int max_matches = max_matches_;

	for (const unit & u : fc_->get_disp_context().units()) {
		if (impl_.matches(unit_filter_impl::unit_filter_args{u, loc ? *loc : u.get_location(), other_unit, fc_, use_flat_tod_})) {
			if(max_matches == 0) {
				return ret;
			}
			--max_matches;
			ret.push_back(&u);
		}
	}
	return ret;
}

unit_const_ptr unit_filter::first_match_on_map() const {
	const unit_map & units = fc_->get_disp_context().units();
	for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
		if (matches(*u, u->get_location())) {
			return u.get_shared_ptr();
		}
	}
	return unit_const_ptr();
}

namespace {

struct unit_filter_xy : public unit_filter_base
{
	unit_filter_xy(const std::string& x, const std::string& y) : x_(x), y_(y) {}

	virtual bool matches(const unit_filter_args& args) const override
	{
		std::string x = utils::interpolate_variables_into_string(x_, *(resources::gamedata));
		std::string y = utils::interpolate_variables_into_string(y_, *(resources::gamedata));

		if(x.empty() && y.empty()) {
			return false;
		}
		else if(x == "recall" && y == "recall") {
			return !args.context().get_disp_context().map().on_board(args.loc);
		}
		else {
			return args.loc.matches_range(x, y);
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

	virtual bool matches(const unit_filter_args& args) const override
	{
		const unit_map& units = args.context().get_disp_context().units();
		const auto adjacent = get_adjacent_tiles(args.loc);
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
			if (unit_itor == units.end() || !child_.matches(unit_filter_args{*unit_itor, unit_itor->get_location(), &args.u, args.fc, args.use_flat_tod} )) {
				continue;
			}
			auto is_enemy = cfg_["is_enemy"];
			if (!is_enemy.empty() && is_enemy.to_bool() != args.context().get_disp_context().get_team(args.u.side()).is_enemy(unit_itor->side())) {
				continue;
			}
			++match_count;
		}

		static std::vector<std::pair<int,int>> default_counts = utils::parse_ranges_unsigned("1-6");
		config::attribute_value i_count = cfg_["count"];
		return in_ranges(match_count, !i_count.blank() ? utils::parse_ranges_unsigned(i_count) : default_counts);
	}

	const unit_filter_compound child_;
	const vconfig cfg_;
};


template<typename F>
struct unit_filter_child_literal : public unit_filter_base
{
	unit_filter_child_literal(const vconfig& v, const F& f) : v_(v) , f_(f) {}
	virtual bool matches(const unit_filter_args& args) const override
	{
		return f_(v_, args);
	}
	vconfig v_;
	F f_;
};

template<typename T, typename F>
struct unit_filter_attribute_parsed : public unit_filter_base
{
	unit_filter_attribute_parsed(T&& v, F&& f) : v_(std::move(v)), f_(std::move(f)) {}
	virtual bool matches(const unit_filter_args& args) const override
	{
		return f_(v_, args);
	}
	T v_;
	F f_;
};

template<typename C, typename F>
struct unit_filter_attribute_literal : public unit_filter_base
{
	unit_filter_attribute_literal(std::string&& v, C&& c, F&& f) : v_(std::move(v)), c_(std::move(c)), f_(std::move(f)) {}
	virtual bool matches(const unit_filter_args& args) const override
	{
		config::attribute_value v;
		v = utils::interpolate_variables_into_string(v_, *(resources::gamedata));
		return f_(c_(v), args);
	}
	std::string v_;
	C c_;
	F f_;
};

class contains_dollar_visitor
#ifdef USING_BOOST_VARIANT
	: public boost::static_visitor<bool>
#endif
{
public:
	contains_dollar_visitor() {}


	template<typename T>
	bool operator()(const T&) const { return false; }

	bool operator()(const t_string&)    const { return true; }

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

bool unit_filter_compound::matches(const unit_filter_args& args) const
{
	bool res;

	if(args.loc.valid()) {
		scoped_xy_unit auto_store("this_unit", args.u.get_location(), args.context().get_disp_context().units());
		if (args.u2) {
			const map_location& loc2 = args.u2->get_location();
			scoped_xy_unit u2_auto_store("other_unit", loc2, args.context().get_disp_context().units());
			res = filter_impl(args);
		} else {
			res = filter_impl(args);
		}
	} else {
		// If loc is invalid, then this is a recall list unit (already been scoped)
		res = filter_impl(args);
	}

	// Handle [and], [or], and [not] with in-order precedence
	for(const auto & filter : cond_children_) {
		switch (filter.first) {
		case conditional_type::type::filter_and:
			res = res && filter.second.matches(args);
			break;
		case conditional_type::type::filter_or:
			res = res || filter.second.matches(args);
			break;
		case conditional_type::type::filter_not:
			res = res && !filter.second.matches(args);
			break;
		}
	}
	return res;
}

bool unit_filter_compound::filter_impl(const unit_filter_args& args) const
{
	for(const auto & filter : children_) {
		if (!filter->matches(args)) {
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
	if(v.blank()) {
	}
	else if(v.apply_visitor(contains_dollar_visitor())) {
		children_.emplace_back(new unit_filter_attribute_literal<C, F>(std::move(v.str()), std::move(conv), std::move(func)));
	}
	else {
		children_.emplace_back(new unit_filter_attribute_parsed<decltype(conv(v)), F>(std::move(conv(v)), std::move(func)));
	}
}

namespace {

	struct ability_match
	{
		std::string tag_name;
		const config* cfg;
	};

	void get_ability_children_id(std::vector<ability_match>& id_result,
	                           const config& parent, const std::string& id) {
		for (const config::any_child sp : parent.all_children_range())
		{
			if(sp.cfg["id"] == id) {
				ability_match special = { sp.key, &sp.cfg };
				id_result.push_back(special);
			}
		}
	}
}

void unit_filter_compound::fill(vconfig cfg)
	{
		const config& literal = cfg.get_config();

		//optimisation
		if(literal.empty()) { return; }

		create_attribute(literal["name"],
			[](const config::attribute_value& c) { return c.t_str(); },
			[](const t_string& str, const unit_filter_args& args) { return str == args.u.name(); }
		);

		create_attribute(literal["id"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& id_list, const unit_filter_args& args)
			{
				return std::find(id_list.begin(), id_list.end(), args.u.id()) != id_list.end();
			}
		);

		create_attribute(literal["type"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& types, const unit_filter_args& args)
			{
				return std::find(types.begin(), types.end(), args.u.type_id()) != types.end();
			}
		);

		create_attribute(literal["type_adv_tree"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& types, const unit_filter_args& args)
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
				return types_expanded.find(args.u.type_id()) != types_expanded.end();
			}
		);

		create_attribute(literal["variation"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& types, const unit_filter_args& args)
			{
				return std::find(types.begin(), types.end(), args.u.variation()) != types.end();
			}
		);

		create_attribute(literal["has_variation"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& types, const unit_filter_args& args)
			{
				// If this unit is a variation itself then search in the base unit's variations.
				const unit_type* const type = args.u.variation().empty() ? &args.u.type() : unit_types.find(args.u.type().parent_id());
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
			[](const std::vector<std::string>& abilities, const unit_filter_args& args)
			{
				for(const std::string& ability_id : abilities) {
					if (args.u.has_ability_by_id(ability_id)) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["ability_type"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& abilities, const unit_filter_args& args)
			{
				for(const std::string& ability : abilities) {
					if (args.u.has_ability_type(ability)) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["ability_id_active"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& abilities, const unit_filter_args& args)
			{
				assert(display::get_singleton());
				const unit_map& units = display::get_singleton()->get_units();
				for(const std::string& ability : abilities) {
					std::vector<ability_match> ability_id_matches_self;
					get_ability_children_id(ability_id_matches_self, args.u.abilities(), ability);
					for(const ability_match& entry : ability_id_matches_self) {
						if (args.u.get_self_ability_bool(*entry.cfg, entry.tag_name, args.loc)) {
							return true;
						}
					}

					const auto adjacent = get_adjacent_tiles(args.loc);
					for(unsigned i = 0; i < adjacent.size(); ++i) {
						const unit_map::const_iterator it = units.find(adjacent[i]);
						if (it == units.end() || it->incapacitated())
							continue;
						if (&*it == (args.u.shared_from_this()).get())
							continue;

						std::vector<ability_match> ability_id_matches_adj;
						get_ability_children_id(ability_id_matches_adj, it->abilities(), ability);
						for(const ability_match& entry : ability_id_matches_adj) {
							if (args.u.get_adj_ability_bool(*entry.cfg, entry.tag_name,i, args.loc, *it)) {
								return true;
							}
						}
					}
				}
				return false;
			}
		);

		create_attribute(literal["ability_type_active"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& abilities, const unit_filter_args& args)
			{
				for(const std::string& ability : abilities) {
					if (!args.u.get_abilities(ability, args.loc).empty()) {
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
			[](const std::vector<std::string>& check_traits, const unit_filter_args& args)
			{

				std::vector<std::string> have_traits = args.u.get_traits_list();
				std::vector<std::string> isect;
				std::sort(have_traits.begin(), have_traits.end());
				std::set_intersection(check_traits.begin(), check_traits.end(), have_traits.begin(), have_traits.end(), std::back_inserter(isect));
				return !isect.empty();
			}
		);

		create_attribute(literal["race"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& races, const unit_filter_args& args)
			{
				return std::find(races.begin(), races.end(), args.u.race()->id()) != races.end();
			}
		);

		create_attribute(literal["gender"],
			[](const config::attribute_value& c) { return string_gender(c.str()); },
			[](unit_race::GENDER gender, const unit_filter_args& args)
			{
				return gender == args.u.gender();
			}
		);

		create_attribute(literal["upkeep"],
			[](const config::attribute_value& c) -> unit::upkeep_t
			{
				try {
					return c.apply_visitor(unit::upkeep_parser_visitor());
				} catch(std::invalid_argument&) {
					return unit::upkeep_full();
				}
			},
			[](unit::upkeep_t upkeep, const unit_filter_args& args)
			{
				return args.u.upkeep() == utils::visit(unit::upkeep_value_visitor{args.u}, upkeep);
			}
		);

		create_attribute(literal["side"],
			[](const config::attribute_value& c)
			{
				std::vector<int> res;
				for(const std::string& s : utils::split(c.str())) {
					try {
						res.push_back(std::stoi(s));
					} catch(std::invalid_argument&) {
						WRN_CF << "ignored invalid side='" << s << "' in filter";
					}
				}
				return res;
			},
			[](const std::vector<int>& sides, const unit_filter_args& args)
			{
				return std::find(sides.begin(), sides.end(), args.u.side()) != sides.end();
			}
		);

		create_attribute(literal["status"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& statuses, const unit_filter_args& args)
			{
				for(const std::string& status : statuses) {
					if (args.u.get_state(status)) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["has_weapon"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& weapon, const unit_filter_args& args)
			{

				for(const attack_type& a : args.u.attacks()) {
					if(a.id() == weapon) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["role"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& role, const unit_filter_args& args)
			{
				return args.u.get_role() == role;
			}
		);

		create_attribute(literal["alignment"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& alignment, const unit_filter_args& args)
			{
				return unit_alignments::get_string(args.u.alignment()) == alignment;
			}
		);

		create_attribute(literal["ai_special"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& ai_special, const unit_filter_args& args)
			{
				return (ai_special == "guardian") == args.u.get_state(unit::STATE_GUARDIAN);
			}
		);

		create_attribute(literal["usage"],
			[](const config::attribute_value& c) { return utils::split(c.str()); },
			[](const std::vector<std::string>& usages, const unit_filter_args& args)
			{
				for(const std::string& usage : usages) {
					if(args.u.usage() == usage) {
						return true;
					}
				}
				return false;
			}
		);

		create_attribute(literal["canrecruit"],
			[](const config::attribute_value& c) { return c.to_bool(); },
			[](bool canrecruit, const unit_filter_args& args)
			{
				return args.u.can_recruit() == canrecruit;
			}
		);

		create_attribute(literal["recall_cost"],
			[](const config::attribute_value& c) { return utils::parse_ranges_unsigned(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& args)
			{
				return in_ranges(args.u.recall_cost(), ranges);
			}
		);

		create_attribute(literal["level"],
			[](const config::attribute_value& c) { return utils::parse_ranges_unsigned(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& args)
			{
				return in_ranges(args.u.level(), ranges);
			}
		);

		create_attribute(literal["defense"],
			[](const config::attribute_value& c) { return utils::parse_ranges_unsigned(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& args)
			{
				int actual_defense = args.u.defense_modifier(args.context().get_disp_context().map().get_terrain(args.loc));
				return in_ranges(actual_defense, ranges);
			}
		);

		create_attribute(literal["movement_cost"],
			[](const config::attribute_value& c) { return utils::parse_ranges_unsigned(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& args)
			{
				int actual_cost = args.u.movement_cost(args.context().get_disp_context().map().get_terrain(args.loc));
				return in_ranges(actual_cost, ranges);
			}
		);

		create_attribute(literal["vision_cost"],
			[](const config::attribute_value& c) { return utils::parse_ranges_unsigned(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& args)
			{
				int actual_cost = args.u.vision_cost(args.context().get_disp_context().map().get_terrain(args.loc));
				return in_ranges(actual_cost, ranges);
			}
		);

		create_attribute(literal["jamming_cost"],
			[](const config::attribute_value& c) { return utils::parse_ranges_unsigned(c.str()); },
			[](const std::vector<std::pair<int,int>>& ranges, const unit_filter_args& args)
			{
				int actual_cost = args.u.jamming_cost(args.context().get_disp_context().map().get_terrain(args.loc));
				return in_ranges(actual_cost, ranges);
			}
		);

		create_attribute(literal["lua_function"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& lua_function, const unit_filter_args& args)
			{
				if (game_lua_kernel * lk = args.context().get_lua_kernel()) {
					return lk->run_filter(lua_function.c_str(), args.u);
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
			[](const wfl::formula& form, const unit_filter_args& args)
			{
				try {
					const wfl::unit_callable main(args.loc, args.u);
					wfl::map_formula_callable callable(main.fake_ptr());
					if (args.u2) {
						auto secondary = std::make_shared<wfl::unit_callable>(*args.u2);
						callable.add("other", wfl::variant(secondary));
						// It's not destroyed upon scope exit because the variant holds a reference
					}
					if(!form.evaluate(callable).as_bool()) {
						return false;
					}
					return true;
				} catch(const wfl::formula_error& e) {
					lg::log_to_chat() << "Formula error in unit filter: " << e.type << " at " << e.filename << ':' << e.line << ")\n";
					ERR_WML << "Formula error in unit filter: " << e.type << " at " << e.filename << ':' << e.line << ")";
					// Formulae with syntax errors match nothing
					return false;
				}
			}
		);

		create_attribute(literal["find_in"],
			[](const config::attribute_value& c) { return c.str(); },
			[](const std::string& find_in, const unit_filter_args& args)
			{
				// Allow filtering by searching a stored variable of units
				if (const game_data * gd = args.context().get_game_data()) {
					try
					{
						for (const config& c : gd->get_variable_access_read(find_in).as_array())
						{
							if(c["id"] == args.u.id()) {
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

		if (!literal["x"].blank() || !literal["y"].blank()) {
			children_.emplace_back(new unit_filter_xy(literal["x"], literal["y"]));
		}

		for(auto child : cfg.all_ordered()) {
			auto cond = conditional_type::get_enum(child.first);
			if(cond) {
				cond_children_.emplace_back(std::piecewise_construct_t(), std::tuple(*cond), std::tuple(child.second));
			}
			else if (child.first == "filter_wml") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& args) {
					config fwml = c.get_parsed_config();

					/* Check if the filter only cares about variables.
					   If so, no need to serialize the whole unit. */
					config::all_children_itors ci = fwml.all_children_range();
					if (fwml.all_children_count() == 1 && fwml.attribute_count() == 1 && ci.front().key == "variables") {
						return args.u.variables().matches(ci.front().cfg);
					} else {
						config ucfg;
						args.u.write(ucfg);
						return ucfg.matches(fwml);
					}
				});
			}
			else if (child.first == "filter_vision") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& args) {
					std::set<int> viewers;
					side_filter ssf(c, args.fc);
					std::vector<int> sides = ssf.get_teams();
					viewers.insert(sides.begin(), sides.end());

					for (const int viewer : viewers) {
						bool fogged = args.context().get_disp_context().get_team(viewer).fogged(args.loc);
						// Check is_enemy() before invisible() to prevent infinite recursion in [abilities][hides][filter_self][filter_vision]
						bool hiding = args.context().get_disp_context().get_team(viewer).is_enemy(args.u.side()) && args.u.invisible(args.loc);
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
				create_child(child.second, [](const vconfig& c, const unit_filter_args& args) {
					return terrain_filter(c, args.fc, args.use_flat_tod).match(args.loc);
				});
			}
			else if (child.first == "filter_side") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& args) {
					return side_filter(c, args.fc).match(args.u.side());
				});
			}
			else if (child.first == "experimental_filter_ability") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& args) {
					for(const config::any_child ab : args.u.abilities().all_children_range()) {
						if(args.u.ability_matches_filter(ab.cfg, ab.key, c.get_parsed_config())) {
							return true;
						}
					}
					return false;
				});
			}
			else if (child.first == "experimental_filter_ability_active") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& args) {
					if(!display::get_singleton()){
						return false;
					}
					const unit_map& units = display::get_singleton()->get_units();
					for(const config::any_child ab : args.u.abilities().all_children_range()) {
						if(args.u.ability_matches_filter(ab.cfg, ab.key, c.get_parsed_config())) {
							if (args.u.get_self_ability_bool(ab.cfg, ab.key, args.loc)) {
								return true;
							}
						}
					}

					const auto adjacent = get_adjacent_tiles(args.loc);
					for(unsigned i = 0; i < adjacent.size(); ++i) {
						const unit_map::const_iterator it = units.find(adjacent[i]);
						if (it == units.end() || it->incapacitated())
							continue;
						if (&*it == (args.u.shared_from_this()).get())
							continue;

						for(const config::any_child ab : it->abilities().all_children_range()) {
							if(it->ability_matches_filter(ab.cfg, ab.key, c.get_parsed_config())) {
								if (args.u.get_adj_ability_bool(ab.cfg, ab.key, i, args.loc, *it)) {
									return true;
								}
							}
						}
					}
				return false;
				});
			}
			else if (child.first == "has_attack") {
				create_child(child.second, [](const vconfig& c, const unit_filter_args& args) {
					for(const attack_type& a : args.u.attacks()) {
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
				DBG_CF << errmsg.str();
			}

		}
	}
