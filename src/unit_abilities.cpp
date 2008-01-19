/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Dominic Bolin <dominic.bolin@exong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file unit_abilities.cpp
//! Manage unit-abilities, like heal, cure, and weapon_specials.

#include "unit.hpp"
#include "unit_abilities.hpp"

#include "log.hpp"
#include "pathutils.hpp"
#include "terrain_filter.hpp"
#include "variable.hpp"

#include <cassert>

#define LOG_NG LOG_STREAM(info, engine)

/*
 *
 * [abilities]
 * ...
 *
 * [heals]
 *	value=4
 *	max_value=8
 *	cumulative=no
 *	affect_allies=yes
 *	name= _ "heals"
// *	name_inactive=null
 *	description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
// *	description_inactive=null
 *	icon="misc/..."
// *	icon_inactive=null
 *	[adjacent_description]
 *		name= _ "heals"
// *		name_inactive=null
 *		description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
// *		description_inactive=null
 *		icon="misc/..."
// *		icon_inactive=null
 *	[/adjacent_description]
 *
 *	affect_self=yes
 *	[filter] // SUF
 *		...
 *	[/filter]
 *	[filter_location]
 *		terrain=f
 *		tod=lawful
 *	[/filter_location]
 *	[filter_self] // SUF
 *		...
 *	[/filter_self]
 *	[filter_adjacent] // SUF
 *		adjacent=n,ne,nw
 *		...
 *	[/filter_adjacent]
 *	[filter_adjacent_location]
 *		adjacent=n,ne,nw
 *		...
 *	[/filter_adjacent]
 *	[affect_adjacent]
 *		adjacent=n,ne,nw
 *		[filter] // SUF
 *			...
 *		[/filter]
 *	[/affect_adjacent]
 *	[affect_adjacent]
 *		adjacent=s,se,sw
 *		[filter] // SUF
 *			...
 *		[/filter]
 *	[/affect_adjacent]
 *
 * [/heals]
 *
 * ...
 * [/abilities]
 *
 */


namespace unit_abilities {

bool affects_side(const config& cfg, const std::vector<team>& teams, size_t side, size_t other_side)
{
	if (side == other_side)
		return utils::string_bool(cfg["affect_allies"], true);
	if (teams[side - 1].is_enemy(other_side))
		return utils::string_bool(cfg["affect_enemies"]);
	else
		return utils::string_bool(cfg["affect_allies"]);
}

}


bool unit::get_ability_bool(const std::string& ability, const gamemap::location& loc) const
{
	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_list& list = abilities->get_children(ability);
		for (config::child_list::const_iterator i = list.begin(),
		     i_end = list.end(); i != i_end; ++i) {
			if (ability_active(ability, **i, loc) &&
			    ability_affects_self(ability, **i, loc))
				return true;
		}
	}

	if(units_== NULL) std::cout<<"ability:"<<ability<<"\n";


	assert(units_ && teams_);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if (it == units_->end() || it->second.incapacitated())
			continue;
		const config* adj_abilities = it->second.cfg_.child("abilities");
		if (!adj_abilities)
			continue;
		const config::child_list& list = adj_abilities->get_children(ability);
		for (config::child_list::const_iterator j = list.begin(),
		     j_end = list.end(); j != j_end; ++j) {
			if (unit_abilities::affects_side(**j, *teams_, side(), it->second.side()) &&
			    it->second.ability_active(ability, **j, adjacent[i]) &&
			    ability_affects_adjacent(ability, **j, i, loc))
				return true;
		}
	}


	return false;
}
unit_ability_list unit::get_abilities(const std::string& ability, const gamemap::location& loc) const
{
	unit_ability_list res;

	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_list& list = abilities->get_children(ability);
		for (config::child_list::const_iterator i = list.begin(),
		     i_end = list.end(); i != i_end; ++i) {
			if (ability_active(ability, **i, loc) &&
			    ability_affects_self(ability, **i, loc))
				res.cfgs.push_back(std::pair<config*, gamemap::location>
					(*i, loc));
		}
	}

	assert(units_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if (it == units_->end() || it->second.incapacitated())
			continue;
		const config* adj_abilities = it->second.cfg_.child("abilities");
		if (!adj_abilities)
			continue;
		const config::child_list& list = adj_abilities->get_children(ability);
		for (config::child_list::const_iterator j = list.begin(),
		     j_end = list.end(); j != j_end; ++j) {
			if (unit_abilities::affects_side(**j, *teams_, side(), it->second.side()) &&
			    it->second.ability_active(ability, **j, adjacent[i]) &&
			    ability_affects_adjacent(ability, **j, i, loc))
				res.cfgs.push_back(std::pair<config*, gamemap::location>
					(*j, adjacent[i]));
		}
	}


	return res;
}

std::vector<std::string> unit::unit_ability_tooltips() const
{
	std::vector<std::string> res;

	const config* abilities = cfg_.child("abilities");
	if (!abilities) return res;
	const config::child_map& list_map = abilities->all_children();
	for (config::child_map::const_iterator i = list_map.begin(),
	     i_end = list_map.end(); i != i_end; ++i) {
		for (config::child_list::const_iterator j = i->second.begin(),
		     j_end = i->second.end(); j != j_end; ++j) {
			std::string const &name = (**j)["name"];
			if (!name.empty()) {
				res.push_back(name);
				res.push_back((**j)["description"]);
			}
		}
	}
	return res;
}

std::vector<std::string> unit::ability_tooltips(const gamemap::location& loc) const
{
	std::vector<std::string> res;

	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_map& list_map = abilities->all_children();
		for (config::child_map::const_iterator i = list_map.begin(),
		     i_end = list_map.end(); i != i_end; ++i) {
			for (config::child_list::const_iterator j = i->second.begin(),
			     j_end = i->second.end(); j != j_end; ++j) {
				if (ability_active(i->first, **j, loc)) {
					std::string const &name = (**j)["name"];
					if (!name.empty()) {
						res.push_back(name);
						res.push_back((**j)["description"]);
					}
				} else {
					std::string const &name = (**j)["name_inactive"];
					if (!name.empty()) {
						res.push_back(name);
						res.push_back((**j)["description_inactive"]);
					}
				}
			}
		}
	}
	/*
	assert(units_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if(it != units_->end() && 0 &&
		!it->second.incapacitated()) {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_map& adj_list_map = adj_abilities->all_children();
				for(config::child_map::const_iterator k = adj_list_map.begin(); k != adj_list_map.end(); ++k) {
					for(config::child_list::const_iterator j = k->second.begin(); j != k->second.end(); ++j) {
						if(unit_abilities::affects_side(**j,*teams_,side(),it->second.side())) {
							const config* adj_desc = (*j)->child("adjacent_description");
							if(ability_affects_adjacent(k->first,**j,i,adjacent[i])) {
								if(!adj_desc) {
									if(it->second.ability_active(k->first,**j,loc)) {
										if((**j)["name"] != "") {
											res.push_back((**j)["name"].str());
											res.push_back((**j)["description"].str());
										}
									} else {
										if((**j)["name_inactive"] != "") {
											res.push_back((**j)["name_inactive"].str());
											res.push_back((**j)["description_inactive"].str());
										}
									}
								} else {
									if(it->second.ability_active(k->first,**j,loc)) {
										if((*adj_desc)["name"] != "") {
											res.push_back((*adj_desc)["name"].str());
											res.push_back((*adj_desc)["description"].str());
										}
									} else {
										if((*adj_desc)["name_inactive"] != "") {
											res.push_back((*adj_desc)["name_inactive"].str());
											res.push_back((*adj_desc)["description_inacive"].str());
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	*/

	return res;
}

/*
 *
 * cfg: an ability WML structure
 *
 */
static bool cache_illuminates(int &cache, std::string const &ability)
{
	if (cache < 0)
		cache = (ability == "illuminates");
	return (cache != 0);
}

bool unit::ability_active(const std::string& ability,const config& cfg,const gamemap::location& loc) const
{
	int illuminates = -1;
	if (config const *filter = cfg.child("filter_self")) {
		if (!matches_filter(filter, loc, cache_illuminates(illuminates, ability)))
			return false;
	}
	assert(units_ && map_ && gamestatus_);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	config::child_list::const_iterator i, i_end;
	const config::child_list& adj_filt = cfg.get_children("filter_adjacent");
	for (i = adj_filt.begin(), i_end = adj_filt.end(); i != i_end; ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		for (std::vector<std::string>::const_iterator j = dirs.begin(),
		     j_end = dirs.end(); j != j_end; ++j) {
			gamemap::location::DIRECTION index =
				gamemap::location::parse_direction(*j);
			if (index == gamemap::location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = units_->find(adjacent[index]);
			if (unit == units_->end())
				return false;
			if (!unit->second.matches_filter(*i, unit->first,
				cache_illuminates(illuminates, ability)))
				return false;
		}
	}
	const config::child_list& adj_filt_loc = cfg.get_children("filter_adjacent_location");
	for (i = adj_filt_loc.begin(), i_end = adj_filt_loc.end(); i != i_end; ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		for (std::vector<std::string>::const_iterator j = dirs.begin(),
		     j_end = dirs.end(); j != j_end; ++j) {
			gamemap::location::DIRECTION index = gamemap::location::parse_direction(*j);
			if (index == gamemap::location::NDIRECTIONS) {
				continue;
			}
			/* GCC-3.3 doesn't accept vconfig(*i) in adj_filter */
			const vconfig& v = vconfig(*i);
			terrain_filter adj_filter(v, *map_, *gamestatus_, *units_);
			adj_filter.flatten(cache_illuminates(illuminates, ability));
			if(!adj_filter.match(adjacent[index])) {
				return false;
			}
		}
	}
	return true;
}
/*
 *
 * cfg: an ability WML structure
 *
 */
bool unit::ability_affects_adjacent(const std::string& ability,const config& cfg,int dir,const gamemap::location& loc) const
{
	assert(dir >=0 && dir <= 5);
	static const std::string adjacent_names[6] = {"n","ne","se","s","sw","nw"};
	const config::child_list& affect_adj = cfg.get_children("affect_adjacent");
	int illuminates = -1;
	for (config::child_list::const_iterator i = affect_adj.begin(),
	     i_end = affect_adj.end(); i != i_end; ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(std::find(dirs.begin(),dirs.end(),adjacent_names[dir]) != dirs.end()) {
			if (config const *filter = (*i)->child("filter")) {
				if (matches_filter(filter, loc,
					cache_illuminates(illuminates, ability)))
					return true;
			} else
				return true;
		}
	}
	return false;
}
/*
 *
 * cfg: an ability WML structure
 *
 */
bool unit::ability_affects_self(const std::string& ability,const config& cfg,const gamemap::location& loc) const
{
	config const *filter = cfg.child("filter");
	bool affect_self = utils::string_bool(cfg["affect_self"], true);
	if (filter == NULL || !affect_self) return affect_self;
	return matches_filter(filter, loc, ability == "illuminates");
}

bool unit::has_ability_type(const std::string& ability) const
{
	const config* list = cfg_.child("abilities");
	if(list) {
		return !list->get_children(ability).empty();
	}
	return false;
}


bool unit_ability_list::empty() const
{
	return cfgs.empty();
}

std::pair<int,gamemap::location> unit_ability_list::highest(const std::string& key, int def) const
{
	if(cfgs.empty()) {
		return std::make_pair(def,gamemap::location::null_location);
	}
	gamemap::location best_loc = gamemap::location::null_location;
	int abs_max = -10000;
	int flat = -10000;
	int stack = 0;
	for (std::vector< std::pair<config*, gamemap::location> >::const_iterator i = cfgs.begin(),
	     i_end = cfgs.end(); i != i_end; ++i) {
		std::string const &text = (*i->first)[key];
		int value = lexical_cast_default<int>(text);
		if (utils::string_bool((*i->first)["cumulative"])) {
			stack += value;
			if (value > abs_max) {
				abs_max = value;
				best_loc = i->second;
			}
		} else {
			int val = text.empty() ? def : value;
			flat = maximum<int>(flat,val);
			if (value > abs_max) {
				abs_max = value;
				best_loc = i->second;
			}
		}
	}
	return std::make_pair(flat + stack, best_loc);
}
std::pair<int,gamemap::location> unit_ability_list::lowest(const std::string& key, int def) const
{
	if(cfgs.empty()) {
		return std::make_pair(def,gamemap::location::null_location);
	}
	gamemap::location best_loc = gamemap::location::null_location;
	int abs_max = 10000;
	int flat = 10000;
	int stack = 0;
	for (std::vector< std::pair<config*, gamemap::location> >::const_iterator i = cfgs.begin(),
	     i_end = cfgs.end(); i != i_end; ++i) {
		std::string const &text = (*i->first)[key];
		int value = lexical_cast_default<int>(text);
		if (utils::string_bool((*i->first)["cumulative"])) {
			stack += value;
			if (value < abs_max) {
				abs_max = value;
				best_loc = i->second;
			}
		} else {
			int val = text.empty() ? def : value;
			flat = minimum<int>(flat,val);
			if (value < abs_max) {
				abs_max = value;
				best_loc = i->second;
			}
		}
	}
	return std::make_pair(flat + stack, best_loc);
}



/*
 *
 * [special]
 * [swarm]
 *	name= _ "swarm"
 *	name_inactive= _ ""
 *	description= _ ""
 *	description_inactive= _ ""
 *	cumulative=no
 *	apply_to=self  #self,opponent,defender,attacker
 *	#active_on=defend  .. offense
 *
 *	attacks_max=4
 *	attacks_min=2
 *
 *	[filter_self] // SUF
 *		...
 *	[/filter_self]
 *	[filter_opponent] // SUF
 *	[filter_attacker] // SUF
 *	[filter_defender] // SUF
 *	[filter_adjacent] // SAUF
 *	[filter_adjacent_location] // SAUF + locs
 * [/swarm]
 * [/special]
 *
 */


bool attack_type::get_special_bool(const std::string& special,bool force) const
{
//	log_scope("get_special_bool");
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_list& list = specials->get_children(special);
		if (!list.empty() && force) return true;
		for (config::child_list::const_iterator i = list.begin(),
		     i_end = list.end(); i != i_end; ++i) {
			if (special_active(**i, true))
				return true;
		}
	}
	if (force || !other_attack_) return false;
	specials = other_attack_->cfg_.child("specials");
	if (specials) {
		const config::child_list& list = specials->get_children(special);
		for (config::child_list::const_iterator i = list.begin(),
		     i_end = list.end(); i != i_end; ++i) {
			if (other_attack_->special_active(**i, false))
				return true;
		}
	}
	return false;
}
unit_ability_list attack_type::get_specials(const std::string& special) const
{
//	log_scope("get_specials");
	unit_ability_list res;
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_list& list = specials->get_children(special);
		for (config::child_list::const_iterator i = list.begin(),
		     i_end = list.end(); i != i_end; ++i) {
			if (special_active(**i, true))
				res.cfgs.push_back(std::pair<config*, gamemap::location>
					(*i, attacker_ ? aloc_ : dloc_));
		}
	}
	if (!other_attack_) return res;
	specials = other_attack_->cfg_.child("specials");
	if (specials) {
		const config::child_list& list = specials->get_children(special);
		for (config::child_list::const_iterator i = list.begin(),
		     i_end = list.end(); i != i_end; ++i) {
			if (other_attack_->special_active(**i, false))
				res.cfgs.push_back(std::pair<config*, gamemap::location>
					(*i, attacker_ ? dloc_ : aloc_));
		}
	}
	return res;
}
std::vector<std::string> attack_type::special_tooltips(bool force) const
{
//	log_scope("special_tooltips");
	std::vector<std::string> res;
	const config* specials = cfg_.child("specials");
	if (!specials) return res;

	const config::child_map& list_map = specials->all_children();
	for (config::child_map::const_iterator i = list_map.begin(),
	     i_end = list_map.end(); i != i_end; ++i) {
		for (config::child_list::const_iterator j = i->second.begin(),
		     j_end = i->second.end(); j != j_end; ++j) {
			if (force || special_active(**j, true)) {
				std::string const &name = (**j)["name"];
				if (!name.empty()) {
					res.push_back(name);
					res.push_back((**j)["description"]);
				}
			} else {
				std::string const &name = (**j)["name_inactive"];
				if (!name.empty()) {
					res.push_back(name);
					res.push_back((**j)["description_inactive"]);
				}
			}
		}
	}
	return res;
}
std::string attack_type::weapon_specials(bool force) const
{
//	log_scope("weapon_specials");
	std::string res;
	const config* specials = cfg_.child("specials");
	if (!specials) return res;

	const config::child_map& list_map = specials->all_children();
	for (config::child_map::const_iterator i = list_map.begin(),
	     i_end = list_map.end(); i != i_end; ++i) {
		for (config::child_list::const_iterator j = i->second.begin(),
		     j_end = i->second.end(); j != j_end; ++j) {
			char const *s = (force || special_active(**j, true, true))
				? "name" : "name_inactive";
			std::string const &name = (**j)[s];

			if (!name.empty()) {
				if (!res.empty()) res += ',';
				res += name;
			}
		}
	}

	return res;
}



/*
 *
 * cfg: a weapon special WML structure
 *
 */
bool attack_type::special_active(const config& cfg,bool self,bool report) const
{
//	log_scope("special_active");
	assert(unitmap_ != NULL);
	unit_map::const_iterator att = unitmap_->find(aloc_);
	unit_map::const_iterator def = unitmap_->find(dloc_);

	if(self) {
		if(!special_affects_self(cfg)) {
			return false;
		}
	} else {
		if(!special_affects_opponent(cfg)) {
			return false;
		}
	}

	if(attacker_) {
		if (!report) {
			std::string const &active = cfg["active_on"];
			if (!active.empty() && active != "offense")
				return false;
		}
		if (config const *filter_self = cfg.child("filter_self")) {
			if (att == unitmap_->end() ||
			    !att->second.matches_filter(filter_self, aloc_))
				return false;
			if (config const *filter_weapon = filter_self->child("filter_weapon")) {
				if (!matches_filter(*filter_weapon, true))
					return false;
			}
		}
		if (config const *filter_opponent = cfg.child("filter_opponent")) {
			if (def == unitmap_->end() ||
			    !def->second.matches_filter(filter_opponent, dloc_))
				return false;
			if (config const *filter_weapon = filter_opponent->child("filter_weapon")) {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(*filter_weapon, true))
					return false;
			}
		}
	} else {
		if (!report) {
			std::string const &active = cfg["active_on"];
			if (!active.empty() && active != "defense")
				return false;
		}
		if (config const *filter_self = cfg.child("filter_self")) {
			if (def == unitmap_->end() ||
			    !def->second.matches_filter(filter_self, dloc_))
				return false;
			if (config const *filter_weapon = filter_self->child("filter_weapon")) {
				if (!matches_filter(*filter_weapon, true))
					return false;
			}
		}
		if (config const *filter_opponent = cfg.child("filter_opponent")) {
			if (att == unitmap_->end() ||
			    !att->second.matches_filter(filter_opponent, aloc_))
				return false;
			if (config const *filter_weapon = filter_opponent->child("filter_weapon")) {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(*filter_weapon, true))
					return false;
			}
		}
	}
	if (config const *filter_attacker = cfg.child("filter_attacker")) {
		if (att == unitmap_->end() ||
		    !att->second.matches_filter(filter_attacker, aloc_))
			return false;
		if (config const *filter_weapon = filter_attacker->child("filter_weapon")) {
			if (attacker_) {
				if (!matches_filter(*filter_weapon, true))
					return false;
			} else {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(*filter_weapon, true))
					return false;
			}
		}
	}
	if (config const *filter_defender = cfg.child("filter_defender")) {
		if (def == unitmap_->end() ||
		    !def->second.matches_filter(filter_defender, dloc_))
			return false;
		if (config const *filter_weapon = filter_defender->child("filter_weapon")) {
			if (!attacker_) {
				if(!matches_filter(*filter_weapon, true))
					return false;
			} else {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(*filter_weapon, true))
					return false;
			}
		}
	}
	gamemap::location adjacent[6];
	if(attacker_) {
		get_adjacent_tiles(aloc_,adjacent);
	} else {
		get_adjacent_tiles(dloc_,adjacent);
	}
	const config::child_list& adj_filt = cfg.get_children("filter_adjacent");
	config::child_list::const_iterator i, i_end;
	for (i = adj_filt.begin(), i_end = adj_filt.end(); i != i_end; ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		for (std::vector<std::string>::const_iterator j = dirs.begin(),
		     j_end = dirs.end(); j != j_end; ++j) {
			gamemap::location::DIRECTION index =
				gamemap::location::parse_direction(*j);
			if (index == gamemap::location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = unitmap_->find(adjacent[index]);
			if (unit == unitmap_->end() ||
			    !unit->second.matches_filter(*i, unit->first))
				return false;
		}
	}
	assert(map_ && game_status_);
	const config::child_list& adj_filt_loc = cfg.get_children("filter_adjacent_location");
	for (i = adj_filt_loc.begin(), i_end = adj_filt_loc.end(); i != i_end; ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		for (std::vector<std::string>::const_iterator j = dirs.begin(),
		     j_end = dirs.end(); j != j_end; ++j) {
			gamemap::location::DIRECTION index =
				gamemap::location::parse_direction(*j);
			if (index == gamemap::location::NDIRECTIONS)
				continue;
			/* GCC-3.3 doesn't accept vconfig(*i) in adj_filter */
			const vconfig& v = vconfig(*i);
			terrain_filter adj_filter(v, *map_, *game_status_, *unitmap_);
			if(!adj_filter.match(adjacent[index])) {
				return false;
			}
		}
	}
	return true;
}
/*
 *
 * cfg: a weapon special WML structure
 *
 */
bool attack_type::special_affects_opponent(const config& cfg) const
{
//	log_scope("special_affects_opponent");
	std::string const &apply_to = cfg["apply_to"];
	if (apply_to.empty())
		return false;
	if (apply_to == "both")
		return true;
	if (apply_to == "opponent")
		return true;
	if (attacker_ && apply_to == "defender")
		return true;
	if (!attacker_ && apply_to == "attacker")
		return true;
	return false;
}
/*
 *
 * cfg: a weapon special WML structure
 *
 */
bool attack_type::special_affects_self(const config& cfg) const
{
//	log_scope("special_affects_self");
	std::string const &apply_to = cfg["apply_to"];
	if (apply_to.empty())
		return true;
	if (apply_to == "both")
		return true;
	if (apply_to == "self")
		return true;
	if (attacker_ && apply_to == "attacker")
		return true;
	if (!attacker_ && apply_to == "defender")
		return true;
	return false;
}
void attack_type::set_specials_context(const gamemap::location& aloc,const gamemap::location& dloc,
                              const game_data* gamedata, const unit_map* unitmap,
							  const gamemap* map, const gamestatus* game_status,
							  const std::vector<team>* teams, bool attacker,const attack_type* other_attack) const
{
	aloc_ = aloc;
	dloc_ = dloc;
	gamedata_ = gamedata;
	unitmap_ = unitmap;
	map_ = map;
	game_status_ = game_status;
	teams_ = teams;
	attacker_ = attacker;
	other_attack_ = other_attack;
}

void attack_type::set_specials_context(const gamemap::location& loc, const gamemap::location& dloc, const unit& un, bool attacker) const
{
	aloc_ = loc;
	dloc_ = dloc;
	gamedata_ = un.gamedata_;
	unitmap_ = un.units_;
	map_ = un.map_;
	game_status_ = un.gamestatus_;
	teams_ = un.teams_;
	attacker_ = attacker;
	other_attack_ = NULL;
}




namespace unit_abilities
{


individual_effect::individual_effect(value_modifier t,int val,config* abil,const gamemap::location& l)
{
	set(t,val,abil,l);
}
void individual_effect::set(value_modifier t,int val,config* abil,const gamemap::location& l)
{
	type=t;
	value=val;
	ability=abil;
	loc=l;
}



effect::effect(const unit_ability_list& list, int def, bool backstab)
{

	int value_set = def; bool value_is_set = false;
	std::map<std::string,individual_effect> values_add;
	std::map<std::string,individual_effect> values_mul;

	individual_effect set_effect;

	for (std::vector< std::pair<config*, gamemap::location> >::const_iterator
	     i = list.cfgs.begin(), i_end = list.cfgs.end(); i != i_end; ++i) {
		const config& cfg = (*i->first);
		std::string const &effect_id = cfg[cfg["id"].empty() ? "name" : "id"];

		if (!backstab && utils::string_bool(cfg["backstab"]))
			continue;

		if (config const *apply_filter = cfg.child("filter_base_value")) {
			std::string const &cond_eq = (*apply_filter)["equals"];
			if (!cond_eq.empty() && lexical_cast_default<int>(cond_eq) != def)
				continue;
			std::string const &cond_ne = (*apply_filter)["not_equals"];
			if (!cond_ne.empty() && lexical_cast_default<int>(cond_ne) == def)
				continue;
			std::string const &cond_lt = (*apply_filter)["less_than"];
			if (!cond_lt.empty() && lexical_cast_default<int>(cond_lt) <= def)
				continue;
			std::string const &cond_gt = (*apply_filter)["greater_than"];
			if (!cond_gt.empty() && lexical_cast_default<int>(cond_gt) >= def)
				continue;
			std::string const &cond_ge = (*apply_filter)["greater_than_equal_to"];
			if (!cond_ge.empty() && lexical_cast_default<int>(cond_ge) > def)
				continue;
			std::string const &cond_le = (*apply_filter)["less_than_equal_to"];
			if (!cond_le.empty() && lexical_cast_default<int>(cond_le) < def)
				continue;
		}
		std::string const &cfg_value = cfg["value"];
		if (!cfg_value.empty()) {
			int value = lexical_cast_default<int>(cfg_value);
			bool cumulative = utils::string_bool(cfg["cumulative"]);
			if (!value_is_set && !cumulative) {
				value_set = value;
				set_effect.set(SET, value, i->first, i->second);
			} else {
				if (cumulative) value_set = maximum<int>(value_set, def);
				if (value > value_set) {
					value_set = value;
					set_effect.set(SET, value, i->first, i->second);
				}
			}
			value_is_set = true;
		}

		std::string const &cfg_add = cfg["add"];
		if (!cfg_add.empty()) {
			int add = lexical_cast_default<int>(cfg_add);
			std::map<std::string,individual_effect>::iterator add_effect = values_add.find(effect_id);
			if(add_effect == values_add.end() || add > add_effect->second.value) {
				values_add[effect_id].set(ADD,add,i->first,i->second);
			}
		}
		std::string const &cfg_mul = cfg["multiply"];
		if (!cfg_mul.empty()) {
			int multiply = int(lexical_cast_default<float>(cfg_mul) * 100);
			std::map<std::string,individual_effect>::iterator mul_effect = values_mul.find(effect_id);
			if(mul_effect == values_mul.end() || multiply > mul_effect->second.value) {
				values_mul[effect_id].set(MUL,multiply,i->first,i->second);
			}
		}
	}

	if(value_is_set && set_effect.type != NOT_USED) {
		effect_list_.push_back(set_effect);
	}

	int multiplier = 1;
	int divisor = 1;
	std::map<std::string,individual_effect>::const_iterator e, e_end;
	for (e = values_mul.begin(), e_end = values_mul.end(); e != e_end; ++e) {
		multiplier *= e->second.value;
		divisor *= 100;
		effect_list_.push_back(e->second);
	}
	int addition = 0;
	for (e = values_add.begin(), e_end = values_add.end(); e != e_end; ++e) {
		addition += e->second.value;
		effect_list_.push_back(e->second);
	}

	composite_value_ = (value_set + addition) * multiplier / divisor;
}

} // end namespace unit_abilities

