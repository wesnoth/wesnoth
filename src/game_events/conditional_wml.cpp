/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Implementations of conditional action WML tags.
 */

#include "game_events/conditional_wml.hpp"

#include "config.hpp"
#include "game_board.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "recall_list_manager.hpp"
#include "resources.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "serialization/string_utils.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"
#include "variable.hpp"

static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)


// This file is in the game_events namespace.
namespace game_events {

namespace builtin_conditions {
	std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-99999");

	bool have_unit(const vconfig& cfg)
	{
		if(!resources::gameboard) {
			return false;
		}
		std::vector<std::pair<int,int> > counts = cfg.has_attribute("count")
			? utils::parse_ranges(cfg["count"]) : default_counts;
		int match_count = 0;
		const unit_filter ufilt(cfg, resources::filter_con);
		for(const unit &i : resources::gameboard->units()) {
			if(i.hitpoints() > 0 && ufilt(i)) {
				++match_count;
				if(counts == default_counts) {
					// by default a single match is enough, so avoid extra work
					break;
				}
			}
		}
		if(cfg["search_recall_list"].to_bool()) {
			for(const team& team : resources::gameboard->teams()) {
				if(counts == default_counts && match_count) {
					break;
				}
				for(size_t t = 0; t < team.recall_list().size(); ++t) {
					if(counts == default_counts && match_count) {
						break;
					}
					scoped_recall_unit auto_store("this_unit", team.save_id(), t);
					if(ufilt(*team.recall_list()[t])) {
						++match_count;
					}
				}
			}
		}
		return in_ranges(match_count, counts);
	}

	bool have_location(const vconfig& cfg)
	{
		std::set<map_location> res;
		terrain_filter(cfg, resources::filter_con).get_locations(res);

		std::vector<std::pair<int,int> > counts = cfg.has_attribute("count")
		? utils::parse_ranges(cfg["count"]) : default_counts;
		return in_ranges<int>(res.size(), counts);
	}

	bool variable_matches(const vconfig& values)
	{
		const std::string name = values["name"];
		config::attribute_value value = resources::gamedata->get_variable_const(name);

#define TEST_STR_ATTR(name, test) \
		do { \
			if (values.has_attribute(name)) { \
				std::string attr_str = values[name].str(); \
				std::string str_value = value.str(); \
				if (!(test)) return false; \
			} \
		} while (0)

#define TEST_NUM_ATTR(name, test) \
		do { \
			if (values.has_attribute(name)) { \
				double attr_num = values[name].to_double(); \
				double num_value = value.to_double(); \
				if (!(test)) return false; \
			} \
		} while (0)

#define TEST_BOL_ATTR(name, test) \
		do { \
			if (values.has_attribute(name)) { \
				bool attr_bool = values[name].to_bool(); \
				bool bool_value = value.to_bool(); \
				if (!(test)) return false; \
			} \
		} while (0)

		TEST_STR_ATTR("equals",                str_value == attr_str);
		TEST_STR_ATTR("not_equals",            str_value != attr_str);
		TEST_NUM_ATTR("numerical_equals",      num_value == attr_num);
		TEST_NUM_ATTR("numerical_not_equals",  num_value != attr_num);
		TEST_NUM_ATTR("greater_than",          num_value >  attr_num);
		TEST_NUM_ATTR("less_than",             num_value <  attr_num);
		TEST_NUM_ATTR("greater_than_equal_to", num_value >= attr_num);
		TEST_NUM_ATTR("less_than_equal_to",    num_value <= attr_num);
		TEST_BOL_ATTR("boolean_equals",       bool_value == attr_bool);
		TEST_BOL_ATTR("boolean_not_equals",   bool_value != attr_bool);
		TEST_STR_ATTR("contains", str_value.find(attr_str) != std::string::npos);

#undef TEST_STR_ATTR
#undef TEST_NUM_ATTR
#undef TEST_BOL_ATTR
		return true;
	}
}

namespace { // Support functions
	bool internal_conditional_passed(const vconfig& cond)
	{
		if(cond.has_child("true")) {
			return true;
		}
		if(cond.has_child("false")) {
			return false;
		}

		vconfig::all_children_iterator cond_end = cond.ordered_end();
		static const std::set<std::string> skip =
			{"then", "else", "elseif", "not", "and", "or", "do"};

		for(vconfig::all_children_iterator it = cond.ordered_begin(); it != cond_end; ++it) {
			std::string key = it.get_key();
			bool result = true;
			if(std::find(skip.begin(), skip.end(), key) == skip.end()) {
				assert(resources::lua_kernel);
				result = resources::lua_kernel->run_wml_conditional(key, it.get_child());
			}
			if (!result) {
				return false;
			}
		}

		return true;
	}

} // end anonymous namespace (support functions)


bool conditional_passed(const vconfig& cond)
{
	bool matches = internal_conditional_passed(cond);

	// Handle [and], [or], and [not] with in-order precedence
	vconfig::all_children_iterator cond_i = cond.ordered_begin();
	vconfig::all_children_iterator cond_end = cond.ordered_end();
	while(cond_i != cond_end)
	{
		const std::string& cond_name = cond_i.get_key();
		const vconfig& cond_filter = cond_i.get_child();

		// Handle [and]
		if(cond_name == "and")
		{
			matches = matches && conditional_passed(cond_filter);
		}
		// Handle [or]
		else if(cond_name == "or")
		{
			matches = matches || conditional_passed(cond_filter);
		}
		// Handle [not]
		else if(cond_name == "not")
		{
			matches = matches && !conditional_passed(cond_filter);
		}
		++cond_i;
	}
	return matches;
}

bool matches_special_filter(const config &cfg, const vconfig& filter)
{
	if (!cfg) {
		WRN_NG << "attempt to filter attack for an event with no attack data." << std::endl;
		// better to not execute the event (so the problem is more obvious)
		return false;
	}
	// Though it may seem wasteful to put this on the heap, it's necessary.
	// matches_filter() could potentially call a WFL formula, which would call shared_from_this().
	auto attack = std::make_shared<const attack_type>(cfg);
	return attack->matches_filter(filter.get_parsed_config());
}

} // end namespace game_events

