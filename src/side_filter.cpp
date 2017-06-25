/*
   Copyright (C) 2010 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config.hpp"
#include "display_context.hpp"
#include "filter_context.hpp"
#include "log.hpp"
#include "recall_list_manager.hpp"
#include "side_filter.hpp"
#include "variable.hpp"
#include "team.hpp"
#include "serialization/string_utils.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "synced_context.hpp"
#include "units/unit.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"
#include "variable.hpp"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/function_gamestate.hpp"

static lg::log_domain log_engine_sf("engine/side_filter");
#define ERR_NG LOG_STREAM(err, log_engine_sf)

side_filter::~side_filter() {}

side_filter::side_filter(const vconfig& cfg, const filter_context * fc,  bool flat_tod)
	: cfg_(cfg)
	, flat_(flat_tod)
	, side_string_()
	, fc_(fc)
{
}

side_filter::side_filter(const std::string &side_string, const filter_context * fc, bool flat_tod)
	: cfg_(vconfig::empty_vconfig()), flat_(flat_tod), side_string_(side_string), fc_(fc)
{
}

std::vector<int> side_filter::get_teams() const
{
	assert(fc_);
	//@todo: replace with better implementation
	std::vector<int> result;
	for(const team &t : fc_->get_disp_context().teams()) {
		if (match(t)) {
			result.push_back(t.side());
		}
	}
	return result;
}

static bool check_side_number(const team &t, const std::string &str)
{
		std::vector<std::pair<int,int> > ranges = utils::parse_ranges(str);
		int side_number = t.side();

		std::vector<std::pair<int,int> >::const_iterator range, range_end = ranges.end();
		for (range = ranges.begin(); range != range_end; ++range) {
			if(side_number >= range->first && side_number <= range->second) {
				return true;
			}
		}
		return false;
}

bool side_filter::match_internal(const team &t) const
{
	assert(fc_);

	if (cfg_.has_attribute("side_in")) {
		if (!check_side_number(t,cfg_["side_in"])) {
			return false;
		}
	}
	if (cfg_.has_attribute("side")) {
		if (!check_side_number(t,cfg_["side"])) {
			return false;
		}
	}
	if (!side_string_.empty()) {
		if (!check_side_number(t,side_string_)) {
			return false;
		}
	}

	config::attribute_value cfg_team_name = cfg_["team_name"];
	if (!cfg_team_name.blank()) {
		const std::string& that_team_name = cfg_team_name;
		const std::string& this_team_name = t.team_name();

		if(std::find(this_team_name.begin(), this_team_name.end(), ',') == this_team_name.end()) {
			if(this_team_name != that_team_name) return false;
		}
		else {
			const std::vector<std::string>& these_team_names = utils::split(this_team_name);
			bool search_futile = true;
			for(const std::string& this_single_team_name : these_team_names) {
				if(this_single_team_name == that_team_name) {
					search_futile = false;
					break;
				}
			}
			if(search_futile) return false;
		}
	}

	//Allow filtering on units
	if(cfg_.has_child("has_unit")) {
		const vconfig & ufilt_cfg = cfg_.child("has_unit");
		if (!ufilter_)
			ufilter_.reset(new unit_filter(ufilt_cfg, fc_, flat_));
		bool found = false;
		for(const unit &u : fc_->get_disp_context().units()) {
			if (u.side() != t.side()) {
				continue;
			}
			if (ufilter_->matches(u)) {
				found = true;
				break;
			}
		}
		if(!found && ufilt_cfg["search_recall_list"].to_bool(false)) {
			for(const unit_const_ptr & u : t.recall_list()) {
				scoped_recall_unit this_unit("this_unit", t.save_id(),t.recall_list().find_index(u->id()));
				if(ufilter_->matches(*u)) {
					found = true;
					break;
				}
			}
		}
		if (!found) {
			return false;
		}
	}

	const vconfig& enemy_of = cfg_.child("enemy_of");
	if(!enemy_of.null()) {
		if (!enemy_filter_)
			enemy_filter_.reset(new side_filter(enemy_of, fc_));
		const std::vector<int>& teams = enemy_filter_->get_teams();
		if(teams.empty()) return false;
		for(const int side : teams) {
			if(!(fc_->get_disp_context().teams())[side - 1].is_enemy(t.side()))
				return false;
		}
	}

	const vconfig& allied_with = cfg_.child("allied_with");
	if(!allied_with.null()) {
		if (!allied_filter_)
			allied_filter_.reset(new side_filter(allied_with, fc_));
		const std::vector<int>& teams = allied_filter_->get_teams();
		if(teams.empty()) return false;
		for(const int side : teams) {
			if((fc_->get_disp_context().teams())[side - 1].is_enemy(t.side()))
				return false;
		}
	}

	const vconfig& has_enemy = cfg_.child("has_enemy");
	if(!has_enemy.null()) {
		if (!has_enemy_filter_)
			has_enemy_filter_.reset(new side_filter(has_enemy, fc_));
		const std::vector<int>& teams = has_enemy_filter_->get_teams();
		bool found = false;
		for(const int side : teams) {
			if((fc_->get_disp_context().teams())[side - 1].is_enemy(t.side()))
			{
				found = true;
				break;
			}
		}
		if (!found) return false;
	}

	const vconfig& has_ally = cfg_.child("has_ally");
	if(!has_ally.null()) {
		if (!has_ally_filter_)
			has_ally_filter_.reset(new side_filter(has_ally, fc_));
		const std::vector<int>& teams = has_ally_filter_->get_teams();
		bool found = false;
		for(const int side : teams) {
			if(!(fc_->get_disp_context().teams())[side - 1].is_enemy(t.side()))
			{
				found = true;
				break;
			}
		}
		if (!found) return false;
	}


	const config::attribute_value cfg_controller = cfg_["controller"];
	if (!cfg_controller.blank())
	{
		if (resources::controller->is_networked_mp() && synced_context::is_synced()) {
			ERR_NG << "ignoring controller= in SSF due to danger of OOS errors" << std::endl;
		}
		else {
			bool found = false;
			for(const std::string& controller : utils::split(cfg_controller))
			{
				if(t.controller().to_string() == controller) {
					found = true;
				}
			}
			if(!found) {
				return false;
			}
		}
	}

	if (cfg_.has_attribute("formula")) {
		try {
			const wfl::team_callable callable(t);
			const wfl::formula form(cfg_["formula"], new wfl::gamestate_function_symbol_table);
			if(!form.evaluate(callable).as_bool()) {
				return false;
			}
			return true;
		} catch(wfl::formula_error& e) {
			lg::wml_error() << "Formula error in side filter: " << e.type << " at " << e.filename << ':' << e.line << ")\n";
			// Formulae with syntax errors match nothing
			return false;
		}
	}

	return true;
}

bool side_filter::match(int side) const
{
	assert(fc_);
	return this->match((fc_->get_disp_context().teams())[side-1]);
}

bool side_filter::match(const team& t) const
{
	bool matches = match_internal(t);

	//handle [and], [or], and [not] with in-order precedence
	vconfig::all_children_iterator cond = cfg_.ordered_begin();
	vconfig::all_children_iterator cond_end = cfg_.ordered_end();
	while (cond != cond_end) {
		const std::string& cond_name = cond.get_key();
		const vconfig& cond_cfg = cond.get_child();

		//handle [and]
		if(cond_name == "and")
		{
			matches = matches && side_filter(cond_cfg, fc_, flat_).match(t);
		}
		//handle [or]
		else if(cond_name == "or")
		{
			matches = matches || side_filter(cond_cfg, fc_, flat_).match(t);
		}
		//handle [not]
		else if(cond_name == "not")
		{
			matches = matches && !side_filter(cond_cfg, fc_, flat_).match(t);
		}
			++cond;
	}
	return matches;
}
