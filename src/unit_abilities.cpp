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
	// we add a null string to prevent the filter to be empty
	terrain_filter_chaotic.push_back("");
	terrain_filter_neutral.push_back("");
	terrain_filter_lawful.push_back("");
}

bool ability::filter::matches_filter(const std::string& terrain, int lawful_bonus) const
{		
	const std::vector<std::string>* terrain_filter;
	if (lawful_bonus < 0) {
		terrain_filter = &terrain_filter_chaotic;
	} else if (lawful_bonus == 0) {
		terrain_filter = &terrain_filter_neutral;
	} else {
		terrain_filter = &terrain_filter_lawful;
	}		
		
	if (terrain_filter->empty()) {
		return true;
	} else {
		return std::find(terrain_filter->begin(),terrain_filter->end(),terrain) != terrain_filter->end();
	}
}

void ability::filter::unfilter()
{
	terrain_filter_chaotic.clear();
	terrain_filter_neutral.clear();
	terrain_filter_lawful.clear();		
}

void ability::filter::add_terrain_filter(const std::string& terrains)
{
	std::vector<std::string> add_to_filter = utils::split(terrains);
	for (std::vector<std::string>::const_iterator t = add_to_filter.begin(); t != add_to_filter.end(); ++t) {
		terrain_filter_chaotic.push_back(*t);
		terrain_filter_neutral.push_back(*t);
		terrain_filter_lawful.push_back(*t);
	}
}

void ability::filter::add_tod_filter(const std::string& times)
{
	std::vector<std::string> add_to_filter = utils::split(times);
	for (std::vector<std::string>::const_iterator t = add_to_filter.begin(); t != add_to_filter.end(); ++t) {
		if (*t == "chaotic") {
			terrain_filter_chaotic.clear();
		} else if (*t == "neutral") {
			terrain_filter_neutral.clear();
		} else if (*t == "lawful") {
			terrain_filter_lawful.clear();		
		}
	}
}
	
void ability::filter::add_filters(const config* cfg)
{
	if (cfg) {
		std::string tods =(*cfg)["tod"];
		std::string terrains =(*cfg)["terrains"];
		if (tods == "" && terrains == "") {
			unfilter();
			return;
		} else if (tods == "") {
			add_terrain_filter(terrains);
			return;
		} else if (terrains == "") {
			add_tod_filter(tods);
			return;
		} else {
			std::vector<std::string> tod_slices = utils::split(tods);
			for (std::vector<std::string>::const_iterator td = tod_slices.begin(); td != tod_slices.end(); ++td) {
				std::vector<std::string>* terrain_filter;
				if (*td == "chaotic") {
					terrain_filter= &terrain_filter_chaotic;
				} else if (*td == "neutral") {
					terrain_filter= &terrain_filter_neutral;
				} else if (*td == "lawful") {
					terrain_filter= &terrain_filter_lawful;
				}
				std::vector<std::string> terrain_slices = utils::split(terrains);
				for (std::vector<std::string>::const_iterator te = terrain_slices.begin(); te != terrain_slices.end(); ++te) {
					terrain_filter->push_back(*te);
				}
			}
		}
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


