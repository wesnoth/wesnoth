/*
   Copyright (C) 2010 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "global.hpp"

#include "config.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "side_filter.hpp"
#include "variable.hpp"
#include "team.hpp"
#include "serialization/string_utils.hpp"
#include "network.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine_sf("engine/side_filter");
#define ERR_NG LOG_STREAM(err, log_engine_sf)

#ifdef _MSC_VER
// This is a workaround for a VC bug; this constructor is never called
// and so we don't care about the warnings this quick fix generates
#pragma warning(push)
#pragma warning(disable:4413)
side_filter::side_filter():
	cfg_(vconfig::unconstructed_vconfig()),
	flat_(),
	side_string_()
{
	assert(false);
}
#pragma warning(pop)
#endif


side_filter::side_filter(const vconfig& cfg, bool flat_tod) :
	cfg_(cfg),
	flat_(flat_tod),
	side_string_()
{
}

side_filter::side_filter(const std::string &side_string, bool flat_tod)
	: cfg_(vconfig::empty_vconfig()), flat_(flat_tod), side_string_(side_string)
{
}

std::vector<int> side_filter::get_teams() const
{
	//@todo: replace with better implementation
	std::vector<int> result;
	BOOST_FOREACH(const team &t, *resources::teams) {
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
			BOOST_FOREACH(const std::string& this_single_team_name, these_team_names) {
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
		const vconfig& unit_filter = cfg_.child("has_unit");
		bool found = false;
		BOOST_FOREACH(unit &u, *resources::units) {
			if (u.side() != t.side()) {
				continue;
			}
			if (u.matches_filter(unit_filter, u.get_location(), flat_)) {
				found = true;
				break;
			}
		}
		if(!found && unit_filter["search_recall_list"].to_bool(false)) {
			const std::vector<unit>& recall_list = t.recall_list();
			BOOST_FOREACH(const unit& u, recall_list) {
				scoped_recall_unit this_unit("this_unit", t.save_id(), &u - &recall_list[0]);
				if(u.matches_filter(unit_filter, u.get_location(), flat_)) {
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
		side_filter s_filter(enemy_of);
		const std::vector<int>& teams = s_filter.get_teams();
		if(teams.empty()) return false;
		BOOST_FOREACH(const int side, teams) {
			if(!(*resources::teams)[side - 1].is_enemy(t.side()))
				return false;
		}
	}

	const vconfig& allied_with = cfg_.child("allied_with");
	if(!allied_with.null()) {
		side_filter s_filter(allied_with);
		const std::vector<int>& teams = s_filter.get_teams();
		if(teams.empty()) return false;
		BOOST_FOREACH(const int side, teams) {
			if((*resources::teams)[side - 1].is_enemy(t.side()))
				return false;
		}
	}

	const config::attribute_value cfg_controller = cfg_["controller"];
	if (!cfg_controller.blank())
	{
		if (network::nconnections() > 0)
			ERR_NG << "ignoring controller= in SSF due to danger of OOS errors" << std::endl;
		else
		{
			if(cfg_controller.str() != team::CONTROLLER_to_string (t.controller()))
				return false;
		}
	}

	return true;
}

bool side_filter::match(int side) const
{
	return this->match((*resources::teams)[side-1]);
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
			matches = matches && side_filter(cond_cfg, flat_).match(t);
		}
		//handle [or]
		else if(cond_name == "or")
		{
			matches = matches || side_filter(cond_cfg, flat_).match(t);
		}
		//handle [not]
		else if(cond_name == "not")
		{
			matches = matches && !side_filter(cond_cfg, flat_).match(t);
		}
			++cond;
	}
	return matches;
}
