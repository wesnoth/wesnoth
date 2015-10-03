/* $Id$ */
/*
   Copyright (C) 2006 - 2011 by Dominic Bolin <dominic.bolin@exong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file unit_abilities.cpp
 *  Manage unit-abilities, like heal, cure, and weapon_specials.
 */

#include "foreach.hpp"
#include "gamestatus.hpp"
#include "resources.hpp"
#include "terrain_filter.hpp"
#include "unit.hpp"
#include "unit_abilities.hpp"



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

static bool affects_side(const config& cfg, const std::vector<team>& teams, size_t side, size_t other_side)
{
	if (side == other_side)
		return utils::string_bool(cfg["affect_allies"], true);
	if (teams[side - 1].is_enemy(other_side))
		return utils::string_bool(cfg["affect_enemies"]);
	else
		return utils::string_bool(cfg["affect_allies"]);
}

}


bool unit::get_ability_bool(const std::string& ability, const map_location& loc) const
{
	if (const config &abilities = cfg_.child("abilities"))
	{
		BOOST_FOREACH (const config &i, abilities.child_range(ability)) {
			if (ability_active(ability, i, loc) &&
			    ability_affects_self(ability, i, loc))
				return true;
		}
	}

	assert(units_);
	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if (it == units_->end() || it->second.incapacitated())
			continue;
		const config &adj_abilities = it->second.cfg_.child("abilities");
		if (!adj_abilities)
			continue;
		BOOST_FOREACH (const config &j, adj_abilities.child_range(ability)) {
			if (unit_abilities::affects_side(j, teams_manager::get_teams(), side(), it->second.side()) &&
			    it->second.ability_active(ability, j, adjacent[i]) &&
			    ability_affects_adjacent(ability,  j, i, loc))
				return true;
		}
	}


	return false;
}
unit_ability_list unit::get_abilities(const std::string& ability, const map_location& loc) const
{
	unit_ability_list res;

	if (const config &abilities = cfg_.child("abilities"))
	{
		BOOST_FOREACH (const config &i, abilities.child_range(ability)) {
			if (ability_active(ability, i, loc) &&
			    ability_affects_self(ability, i, loc))
				res.cfgs.push_back(std::pair<const config *, map_location>(&i, loc));
		}
	}

	assert(units_ != NULL);
	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if (it == units_->end() || it->second.incapacitated())
			continue;
		const config &adj_abilities = it->second.cfg_.child("abilities");
		if (!adj_abilities)
			continue;
		BOOST_FOREACH (const config &j, adj_abilities.child_range(ability)) {
			if (unit_abilities::affects_side(j, teams_manager::get_teams(), side(), it->second.side()) &&
			    it->second.ability_active(ability, j, adjacent[i]) &&
			    ability_affects_adjacent(ability, j, i, loc))
				res.cfgs.push_back(std::pair<const config *, map_location>(&j, adjacent[i]));
		}
	}


	return res;
}

std::vector<std::string> unit::get_ability_list() const
{
	std::vector<std::string> res;

	const config &abilities = cfg_.child("abilities");
	if (!abilities) return res;
	BOOST_FOREACH (const config::any_child &ab, abilities.all_children_range()) {
		std::string const &id = ab.cfg["id"];
		if (!id.empty())
			res.push_back(id);
	}
	return res;
}

std::vector<std::string> unit::ability_tooltips(bool force_active) const
{
	std::vector<std::string> res;

	const config &abilities = cfg_.child("abilities");
	if (!abilities) return res;

	BOOST_FOREACH (const config::any_child &ab, abilities.all_children_range())
	{
		if (force_active || ability_active(ab.key, ab.cfg, loc_))
		{
			std::string const &name =
				gender_ == unit_race::MALE || ab.cfg["female_name"].empty() ?
				ab.cfg["name"] : ab.cfg["female_name"];

			if (!name.empty()) {
				res.push_back(name);
				res.push_back(ab.cfg["description"]);
			}
		}
		else
		{
			std::string const &name =
				gender_ == unit_race::MALE || ab.cfg["female_name_inactive"].empty() ?
				ab.cfg["name_inactive"] : ab.cfg["female_name_inactive"];

			if (!name.empty()) {
				res.push_back(name);
				res.push_back(ab.cfg["description_inactive"]);
			}
		}
	}
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

bool unit::ability_active(const std::string& ability,const config& cfg,const map_location& loc) const
{
	int illuminates = -1;
	assert(units_ && resources::game_map && resources::teams && resources::tod_manager);

	if (const config &afilter = cfg.child("filter"))
		if (!matches_filter(vconfig(afilter), loc, cache_illuminates(illuminates, ability)))
			return false;

	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);

	BOOST_FOREACH (const config &i, cfg.child_range("filter_adjacent"))
	{
		BOOST_FOREACH (const std::string &j, utils::split(i["adjacent"]))
		{
			map_location::DIRECTION index =
				map_location::parse_direction(j);
			if (index == map_location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = units_->find(adjacent[index]);
			if (unit == units_->end())
				return false;
			if (!unit->second.matches_filter(vconfig(i), unit->first,
				cache_illuminates(illuminates, ability)))
				return false;
		}
	}

	BOOST_FOREACH (const config &i, cfg.child_range("filter_adjacent_location"))
	{
		BOOST_FOREACH (const std::string &j, utils::split(i["adjacent"]))
		{
			map_location::DIRECTION index = map_location::parse_direction(j);
			if (index == map_location::NDIRECTIONS) {
				continue;
			}
			terrain_filter adj_filter(vconfig(i), *units_);
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
bool unit::ability_affects_adjacent(const std::string& ability, const config& cfg,int dir,const map_location& loc) const
{
	int illuminates = -1;

	assert(dir >=0 && dir <= 5);
	static const std::string adjacent_names[6] = {"n","ne","se","s","sw","nw"};
	BOOST_FOREACH (const config &i, cfg.child_range("affect_adjacent"))
	{
		std::vector<std::string> dirs = utils::split(i["adjacent"]);
		if(std::find(dirs.begin(),dirs.end(),adjacent_names[dir]) != dirs.end()) {
			if (const config &filter = i.child("filter")) {
				if (matches_filter(vconfig(filter), loc,
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
bool unit::ability_affects_self(const std::string& ability,const config& cfg,const map_location& loc) const
{
	int illuminates = -1;
	const config &filter = cfg.child("filter_self");
	bool affect_self = utils::string_bool(cfg["affect_self"], true);
	if (!filter || !affect_self) return affect_self;
	return matches_filter(vconfig(filter), loc,cache_illuminates(illuminates, ability));
}

bool unit::has_ability_type(const std::string& ability) const
{
	if (const config &list = cfg_.child("abilities")) {
		config::const_child_itors itors = list.child_range(ability);
		return itors.first != itors.second;
	}
	return false;
}


bool unit_ability_list::empty() const
{
	return cfgs.empty();
}

std::pair<int,map_location> unit_ability_list::highest(const std::string& key, int def) const
{
	if (cfgs.empty()) {
		return std::make_pair(def, map_location());
	}
	// The returned location is the best non-cumulative one, if any,
	// the best absolute cumulative one otherwise.
	map_location best_loc;
	bool only_cumulative = true;
	int abs_max = 0;
	int flat = 0;
	int stack = 0;
	typedef std::pair<const config *, map_location> pt;
	BOOST_FOREACH (pt const &p, cfgs)
	{
		int value = lexical_cast_default<int>((*p.first)[key], def);
		if (utils::string_bool((*p.first)["cumulative"])) {
			stack += value;
			if (value < 0) value = -value;
			if (only_cumulative && value >= abs_max) {
				abs_max = value;
				best_loc = p.second;
			}
		} else if (only_cumulative || value > flat) {
			only_cumulative = false;
			flat = value;
			best_loc = p.second;
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

namespace {
	bool get_special_children(std::vector<const config*>& result, const config& parent,
	                           const std::string& id, bool just_peeking=false) {
		BOOST_FOREACH (const config::any_child &sp, parent.all_children_range())
		{
			if (sp.key == id || sp.cfg["id"] == id) {
				if(just_peeking) {
					return true; // peek succeeded, abort
				} else {
					result.push_back(&sp.cfg);
				}
			}
		}
		return false;
	}
}

bool attack_type::get_special_bool(const std::string& special,bool force) const
{
//	log_scope("get_special_bool");
	if (const config &specials = cfg_.child("specials"))
	{
		std::vector<const config*> list;
		if (get_special_children(list, specials, special, force)) return true;
		for (std::vector<const config*>::iterator i = list.begin(),
		     i_end = list.end(); i != i_end; ++i) {
			if (special_active(**i, true))
				return true;
		}
	}
	if (force || !other_attack_) return false;
	if (const config &specials = other_attack_->cfg_.child("specials"))
	{
		std::vector<const config*> list;
		get_special_children(list, specials, special);
		for (std::vector<const config*>::iterator i = list.begin(),
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
	if (const config &specials = cfg_.child("specials"))
	{
		BOOST_FOREACH (const config &i, specials.child_range(special)) {
			if (special_active(i, true))
				res.cfgs.push_back(std::pair<const config *, map_location>
					(&i, attacker_ ? aloc_ : dloc_));
		}
	}
	if (!other_attack_) return res;
	if (const config &specials = other_attack_->cfg_.child("specials"))
	{
		BOOST_FOREACH (const config &i, specials.child_range(special)) {
			if (other_attack_->special_active(i, false))
				res.cfgs.push_back(std::pair<const config *, map_location>
					(&i, attacker_ ? dloc_ : aloc_));
		}
	}
	return res;
}
std::vector<t_string> attack_type::special_tooltips(bool force) const
{
//	log_scope("special_tooltips");
	std::vector<t_string> res;
	const config &specials = cfg_.child("specials");
	if (!specials) return res;

	BOOST_FOREACH (const config::any_child &sp, specials.all_children_range())
	{
		if (force || special_active(sp.cfg, true)) {
			const t_string &name = sp.cfg["name"];
			if (!name.empty()) {
				res.push_back(name);
				res.push_back(sp.cfg["description"]);
			}
		} else {
			t_string const &name = sp.cfg["name_inactive"];
			if (!name.empty()) {
				res.push_back(name);
				res.push_back(sp.cfg["description_inactive"]);
			}
		}
	}
	return res;
}
std::string attack_type::weapon_specials(bool force) const
{
//	log_scope("weapon_specials");
	std::string res;
	const config &specials = cfg_.child("specials");
	if (!specials) return res;

	BOOST_FOREACH (const config::any_child &sp, specials.all_children_range())
	{
		char const *s = force || special_active(sp.cfg, true) ?
			"name" : "name_inactive";
		std::string const &name = sp.cfg[s];

		if (!name.empty()) {
			if (!res.empty()) res += ',';
			res += name;
		}
	}

	return res;
}



/*
 *
 * cfg: a weapon special WML structure
 *
 */
bool attack_type::special_active(const config& cfg, bool self) const
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
		{
			std::string const &active = cfg["active_on"];
			if (!active.empty() && active != "offense")
				return false;
		}
		if (const config &filter_self = cfg.child("filter_self"))
		{
			if (att == unitmap_->end() ||
			    !att->second.matches_filter(vconfig(filter_self), aloc_))
				return false;
			if (const config &filter_weapon = filter_self.child("filter_weapon")) {
				if (!matches_filter(filter_weapon, true))
					return false;
			}
		}
		if (const config &filter_opponent = cfg.child("filter_opponent"))
		{
			if (def == unitmap_->end() ||
			    !def->second.matches_filter(vconfig(filter_opponent), dloc_))
				return false;
			if (const config &filter_weapon = filter_opponent.child("filter_weapon")) {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(filter_weapon, true))
					return false;
			}
		}
	} else {
		{
			std::string const &active = cfg["active_on"];
			if (!active.empty() && active != "defense")
				return false;
		}
		if (const config &filter_self = cfg.child("filter_self"))
		{
			if (def == unitmap_->end() ||
			    !def->second.matches_filter(vconfig(filter_self), dloc_))
				return false;
			if (const config &filter_weapon = filter_self.child("filter_weapon")) {
				if (!matches_filter(filter_weapon, true))
					return false;
			}
		}
		if (const config &filter_opponent = cfg.child("filter_opponent"))
		{
			if (att == unitmap_->end() ||
			    !att->second.matches_filter(vconfig(filter_opponent), aloc_))
				return false;
			if (const config &filter_weapon = filter_opponent.child("filter_weapon")) {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(filter_weapon, true))
					return false;
			}
		}
	}
	if (const config &filter_attacker = cfg.child("filter_attacker"))
	{
		if (att == unitmap_->end() ||
		    !att->second.matches_filter(vconfig(filter_attacker), aloc_))
			return false;
		if (const config &filter_weapon = filter_attacker.child("filter_weapon"))
		{
			if (attacker_) {
				if (!matches_filter(filter_weapon, true))
					return false;
			} else {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(filter_weapon, true))
					return false;
			}
		}
	}
	if (const config &filter_defender = cfg.child("filter_defender"))
	{
		if (def == unitmap_->end() ||
		    !def->second.matches_filter(vconfig(filter_defender), dloc_))
			return false;
		if (const config &filter_weapon = filter_defender.child("filter_weapon"))
		{
			if (!attacker_) {
				if(!matches_filter(filter_weapon, true))
					return false;
			} else {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(filter_weapon, true))
					return false;
			}
		}
	}
	map_location adjacent[6];
	if(attacker_) {
		get_adjacent_tiles(aloc_,adjacent);
	} else {
		get_adjacent_tiles(dloc_,adjacent);
	}

	BOOST_FOREACH (const config &i, cfg.child_range("filter_adjacent"))
	{
		BOOST_FOREACH (const std::string &j, utils::split(i["adjacent"]))
		{
			map_location::DIRECTION index =
				map_location::parse_direction(j);
			if (index == map_location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = unitmap_->find(adjacent[index]);
			if (unit == unitmap_->end() ||
			    !unit->second.matches_filter(vconfig(i), unit->first))
				return false;
		}
	}

	BOOST_FOREACH (const config &i, cfg.child_range("filter_adjacent_location"))
	{
		BOOST_FOREACH (const std::string &j, utils::split(i["adjacent"]))
		{
			map_location::DIRECTION index =
				map_location::parse_direction(j);
			if (index == map_location::NDIRECTIONS)
				continue;
			terrain_filter adj_filter(vconfig(i), *unitmap_);
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
void attack_type::set_specials_context(const map_location& aloc,const map_location& dloc,
	const unit_map &unitmap, bool attacker, const attack_type *other_attack) const
{
	aloc_ = aloc;
	dloc_ = dloc;
	unitmap_ = &unitmap;
	attacker_ = attacker;
	other_attack_ = other_attack;
}

void attack_type::set_specials_context(const map_location& loc, const map_location& dloc, const unit& un, bool attacker) const
{
	aloc_ = loc;
	dloc_ = dloc;
	unitmap_ = un.units_;
	attacker_ = attacker;
	other_attack_ = NULL;
}




namespace unit_abilities
{

void individual_effect::set(value_modifier t, int val, const config *abil, const map_location &l)
{
	type=t;
	value=val;
	ability=abil;
	loc=l;
}

bool filter_base_matches(const config& cfg, int def)
{
	if (const config &apply_filter = cfg.child("filter_base_value")) {
		std::string const &cond_eq = apply_filter["equals"];
		std::string const &cond_ne = apply_filter["not_equals"];
		std::string const &cond_lt = apply_filter["less_than"];
		std::string const &cond_gt = apply_filter["greater_than"];
		std::string const &cond_ge = apply_filter["greater_than_equal_to"];
		std::string const &cond_le = apply_filter["less_than_equal_to"];
		if ((cond_eq.empty() || def == lexical_cast_default<int>(cond_eq))
		&& (cond_ne.empty() || def != lexical_cast_default<int>(cond_ne))
		&& (cond_lt.empty() || def <  lexical_cast_default<int>(cond_lt))
		&& (cond_gt.empty() || def >  lexical_cast_default<int>(cond_gt))
		&& (cond_ge.empty() || def >= lexical_cast_default<int>(cond_ge))
		&& (cond_le.empty() || def <= lexical_cast_default<int>(cond_le)))
            return true;
        else
            return false;
	}
    return true;
}

effect::effect(const unit_ability_list& list, int def, bool backstab) :
	effect_list_(),
	composite_value_(0)
{

	int value_set = def;
	bool value_is_set = false;
	std::map<std::string,individual_effect> values_add;
	std::map<std::string,individual_effect> values_mul;

	individual_effect set_effect;

	for (std::vector< std::pair<const config *, map_location> >::const_iterator
	     i = list.cfgs.begin(), i_end = list.cfgs.end(); i != i_end; ++i) {
		const config& cfg = (*i->first);
		std::string const &effect_id = cfg[cfg["id"].empty() ? "name" : "id"];

		if (!backstab && utils::string_bool(cfg["backstab"]))
			continue;
		if (!filter_base_matches(cfg, def))
			continue;

		std::string const &cfg_value = cfg["value"];
		if (!cfg_value.empty()) {
			int value = lexical_cast_default<int>(cfg_value);
			bool cumulative = utils::string_bool(cfg["cumulative"]);
			if (!value_is_set && !cumulative) {
				value_set = value;
				set_effect.set(SET, value, i->first, i->second);
			} else {
				if (cumulative) value_set = std::max<int>(value_set, def);
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

