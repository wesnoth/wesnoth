/* $Id$ */
/*
   Copyright (C) 2006 by Benoit Timbert <benoit.timbert@free.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "unit_abilities.hpp"

#include "config.hpp"
#include "game_config.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"

#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

/// filter ///
ability::filter::filter()
{
}

bool ability::filter::matches_filter(const std::string& terrain, int lawful_bonus) const
{
	const std::string& tod_string = lawful_bonus > 0 ? "lawful" : (lawful_bonus < 0 ? "chaotic" : "neutral");
	if(filters.empty()) {
		return true;
	}
	for(std::vector<config>::const_iterator i=filters.begin(); i != filters.end(); ++i) {
		std::vector<std::string> terrain_filters;
		std::vector<std::string> tod_filters;
		if(std::find(terrain_filters.begin(),terrain_filters.end(),",")!=terrain_filters.end() && (*i)["terrain"].str().find(terrain)) {
			terrain_filters = utils::split((*i)["terrain"]);
		} else if((*i)["terrain"] != "") {
			terrain_filters.push_back((*i)["terrain"]);
		}
		if(std::find(tod_filters.begin(),tod_filters.end(),",")!=tod_filters.end() && (*i)["tod"].str().find(tod_string)) {
			tod_filters = utils::split((*i)["tod"]);
		} else if((*i)["tod"] != "") {
			tod_filters.push_back((*i)["tod"]);
		}
		if(terrain_filters.size()) {
			bool matches_filter = false;
			for (std::string::const_iterator t = terrain.begin(); t != terrain.end(); ++i) {
				std::string ts(1,*t);
				matches_filter |= (std::find(terrain_filters.begin(),terrain_filters.end(),ts) == terrain_filters.end());
			}
			if(!matches_filter) {
				return false;
			}
		}
		if(tod_filters.size() && std::find(tod_filters.begin(),tod_filters.end(),tod_string) == tod_filters.end()) {
			return false;
		}
	}
	return true;
}

void ability::filter::unfilter()
{
	filters.clear();
}
	
void ability::filter::add_filters(const config* cfg)
{
	if (cfg) {
		filters.push_back(*cfg);
	} else {
		unfilter();
	}
}


/// ability ///
ability::ability(const config* cfg)
{
	if((*cfg)["description"] != "") {
		description_=((*cfg)["description"]);
	}
	filter.add_filters(cfg->child("filter"));
}

const std::string ability::description() const
{
	return description_;
}

/// heals_ability ///
heals_ability::heals_ability(const config* cfg)
: ability::ability(cfg)
{
	amount_=lexical_cast_default<int>((*cfg)["amount"],game_config::healer_heals_per_turn);
	max_=lexical_cast_default<int>((*cfg)["max"],game_config::heal_amount);
}

const int heals_ability::amount() const
{
	return amount_;
}

const int heals_ability::max() const
{
	return max_;
}

void heals_ability::set_heal(int amount, int max)
{
	amount_=amount;
	max_=max;
}

/// regenerates_ability ///
regenerates_ability::regenerates_ability(const config* cfg)
: ability::ability(cfg)
{
	regeneration_=lexical_cast_default<int>((*cfg)["amount"],game_config::cure_amount);
}

const int regenerates_ability::regeneration() const
{
	return regeneration_;
}

void regenerates_ability::set_regeneration(int reg)
{
	regeneration_=reg;
}

/// leadership_ability ///
leadership_ability::leadership_ability(const config* cfg)
: ability::ability(cfg)
{
	perlevel_bonus_=lexical_cast_default<int>((*cfg)["perlevel_bonus"],game_config::leadership_bonus);
}

const int leadership_ability::perlevel_bonus() const
{
	return perlevel_bonus_;
}

void leadership_ability::set_leadership(int perlevel_bonus)
{
	perlevel_bonus_=perlevel_bonus;
}

/// illuminates_ability ///
illuminates_ability::illuminates_ability(const config* cfg)
: ability::ability(cfg)
{
	level_=lexical_cast_default<int>((*cfg)["level"],1);
}

const int illuminates_ability::level() const
{
	return level_;
}

void illuminates_ability::set_illumination(int level)
{
	level_=level;
}

/// steadfast_ability ///
steadfast_ability::steadfast_ability(const config* cfg)
: ability::ability(cfg)
{
	std::string bonus_string = (*cfg)["bonus"];
	bonus_=lexical_cast_default<int>(bonus_string,100);
	max_=lexical_cast_default<int>((*cfg)["max"],50);
	if (bonus_string != "" && bonus_string[bonus_string.size()-1] == '%') {
		use_percent_ = true;
	} else {
		use_percent_ = false;
	}
}

const int steadfast_ability::bonus() const
{
	return bonus_;
}

const int steadfast_ability::max() const
{
	return max_;
}

const bool steadfast_ability::use_percent() const
{
	return use_percent_;
}

void steadfast_ability::set_steadfast(int bonus, int max, bool use_percent)
{
	bonus_=bonus;
	max_=max;
	use_percent_=use_percent;
}


