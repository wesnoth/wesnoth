/* $Id$ */
/*
   Copyright (C) 2006 - 2011 by Dominic Bolin <dominic.bolin@exong.net>
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
 *  @file
 *  Manage unit-abilities, like heal, cure, and weapon_specials.
 */

#include "foreach.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "terrain_filter.hpp"
#include "unit.hpp"
#include "team.hpp"
#include "unit_abilities.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)



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

static bool affects_side(const config& cfg, const t_teams& teams, size_t side, size_t other_side)
{
	static const config::t_token z_affect_allies("affect_allies", false);
	static const config::t_token z_affect_enemies("affect_enemies", false);

	if (side == other_side)
		return cfg[z_affect_allies].to_bool(true);
	if (teams[side - 1].is_enemy(other_side))
		return cfg[z_affect_enemies].to_bool();
	else
		return cfg[z_affect_allies].to_bool();
}

}


bool unit::get_ability_bool(const config::t_token& ability, const map_location& loc, gamemap const & game_map, unit_map const & units, t_teams const & teams, LuaKernel & lua_kernel, tod_manager const & tod_manager) const
{
	static const config::t_token z_abilities("abilities", false);

	if (const config &abilities = cfg_.child(z_abilities))
	{
		foreach (const config &i, abilities.child_range(ability)) {
			if (ability_active(ability, i, loc, game_map, units, teams, lua_kernel, tod_manager)
				&& ability_affects_self(ability, i, loc))
				return true;
		}
	}

	//const unit_map& units = *resources::units;
	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units.find(adjacent[i]);
		if (it == units.end() || it->incapacitated())
			continue;
		const config &adj_abilities = it->cfg_.child(z_abilities);
		if (!adj_abilities)
			continue;
		foreach (const config &j, adj_abilities.child_range(ability)) {
			if (unit_abilities::affects_side(j, teams_manager::get_teams(), side(), it->side()) &&
			    it->ability_active(ability, j, adjacent[i]) &&
			    ability_affects_adjacent(ability,  j, i, loc))
				return true;
		}
	}


	return false;
}
unit_ability_list unit::get_abilities(const config::t_token& ability, const map_location& loc) const
{
	static const config::t_token z_abilities("abilities", false);

	unit_ability_list res;

	if (const config &abilities = cfg_.child(z_abilities))
	{
		foreach (const config &i, abilities.child_range(ability)) {
			if (ability_active(ability, i, loc) &&
			    ability_affects_self(ability, i, loc))
				res.cfgs.push_back(std::pair<const config *, map_location>(&i, loc));
		}
	}

	const unit_map& units = *resources::units;
	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units.find(adjacent[i]);
		if (it == units.end() || it->incapacitated())
			continue;
		const config &adj_abilities = it->cfg_.child(z_abilities);
		if (!adj_abilities)
			continue;
		foreach (const config &j, adj_abilities.child_range(ability)) {
			if (unit_abilities::affects_side(j, teams_manager::get_teams(), side(), it->side()) &&
			    it->ability_active(ability, j, adjacent[i]) &&
			    ability_affects_adjacent(ability, j, i, loc))
				res.cfgs.push_back(std::pair<const config *, map_location>(&j, adjacent[i]));
		}
	}


	return res;
}

std::vector<std::string> unit::get_ability_list() const
{
	static const config::t_token z_abilities("abilities", false);
	static const config::t_token z_id("id", false);

	std::vector<std::string> res;

	const config &abilities = cfg_.child(z_abilities);
	if (!abilities) return res;
	foreach (const config::any_child &ab, abilities.all_children_range()) {
		std::string const &id = ab.cfg[z_id];
		if (!id.empty())
			res.push_back(id);
	}
	return res;
}

std::vector<std::string> unit::ability_tooltips(bool force_active) const
{
	static const config::t_token z_abilities("abilities", false);
	static const config::t_token z_female_name("female_name", false);
	static const config::t_token z_name("name", false);
	static const config::t_token z_description("description", false);
	static const config::t_token z_female_name_inactive("female_name_inactive", false);
	static const config::t_token z_name_inactive("name_inactive", false);
	static const config::t_token z_description_inactive("description_inactive", false);

	std::vector<std::string> res;

	const config &abilities = cfg_.child(z_abilities);
	if (!abilities) return res;

	foreach (const config::any_child &ab, abilities.all_children_range())
	{
		if (force_active || ability_active(ab.key, ab.cfg, loc_))
		{
			std::string const &name =
				gender_ == unit_race::MALE || ab.cfg[z_female_name].empty() ?
				ab.cfg[z_name] : ab.cfg[z_female_name];

			if (!name.empty()) {
				res.push_back(name);
				res.push_back(ab.cfg[z_description]);
			}
		}
		else
		{
			std::string const &name =
				gender_ == unit_race::MALE || ab.cfg[z_female_name_inactive].empty() ?
				ab.cfg[z_name_inactive] : ab.cfg[z_female_name_inactive];

			if (!name.empty()) {
				res.push_back(name);
				res.push_back(ab.cfg[z_description_inactive]);
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
	static const config::t_token z_illuminates("illuminates", false);

	if (cache < 0)
		cache = (ability == z_illuminates);
	return (cache != 0);
}

bool unit::ability_active(const std::string& ability,const config& cfg,const map_location& loc
						  , gamemap const & game_map, unit_map const & units
						  , t_teams const & teams, LuaKernel & lua_kernel
						  , tod_manager const & tod_manager) const
{
	static const config::t_token z_filter("filter", false);
	static const config::t_token z_filter_adjacent("filter_adjacent", false);
	static const config::t_token z_adjacent("adjacent", false);
	static const config::t_token z_filter_adjacent_location("filter_adjacent_location", false);

	int illuminates = -1;
	//assert(resources::units && resources::game_map && resources::teams && resources::tod_manager);

	if (const config &afilter = cfg.child(z_filter))
		if (!matches_filter(vconfig(afilter), loc, cache_illuminates(illuminates, ability),game_map, units, teams, lua_kernel, tod_manager))
			return false;

	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	//const unit_map& units = *resources::units;

	foreach (const config &i, cfg.child_range(z_filter_adjacent))
	{
		foreach (const config::t_token j, utils::split_attr(i[z_adjacent]))
		{
			map_location::DIRECTION index =
				map_location::parse_direction(*j);
			if (index == map_location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = units.find(adjacent[index]);
			if (unit == units.end())
				return false;
			if (!unit->matches_filter(vconfig(i), unit->get_location(),
									  cache_illuminates(illuminates, ability), game_map, units, teams, lua_kernel, tod_manager))
				return false;
		}
	}

	foreach (const config &i, cfg.child_range(z_filter_adjacent_location))
	{
		foreach (const config::t_token j, utils::split_attr(i[z_adjacent]))
		{
			map_location::DIRECTION index = map_location::parse_direction(*j);
			if (index == map_location::NDIRECTIONS) {
				continue;
			}
			terrain_filter adj_filter(vconfig(i), units);
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
bool unit::ability_affects_adjacent(const std::string& ability, const config& cfg,int dir,
									const map_location& loc,
									gamemap const & game_map, unit_map const & units,
									t_teams const & teams, LuaKernel & lua_kernel,
									tod_manager const & tod_manager
									) const
{
	static const config::t_token z_n("n", false);
	static const config::t_token z_ne("ne", false);
	static const config::t_token z_se("se", false);
	static const config::t_token z_s("s", false);
	static const config::t_token z_sw("sw", false);
	static const config::t_token z_nw("nw", false);
	static const config::t_token z_affect_adjacent("affect_adjacent", false);
	static const config::t_token z_adjacent("adjacent", false);
	static const config::t_token z_filter("filter", false);

	int illuminates = -1;

	assert(dir >=0 && dir <= 5);
	static const std::string adjacent_names[6] = {z_n,z_ne,z_se,z_s,z_sw,z_nw};
	foreach (const config &i, cfg.child_range(z_affect_adjacent))
	{
		std::vector<std::string> dirs = utils::split(i[z_adjacent]);
		if(std::find(dirs.begin(),dirs.end(),adjacent_names[dir]) != dirs.end()) {
			if (const config &filter = i.child(z_filter)) {
				if (matches_filter(vconfig(filter), loc,
								   cache_illuminates(illuminates, ability), game_map, units, teams, lua_kernel, tod_manager))
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
bool unit::ability_affects_self(const std::string& ability,const config& cfg,const map_location& loc
								, gamemap const & game_map, unit_map const & units
								, t_teams const & teams, LuaKernel & lua_kernel
								, tod_manager const & tod_manager
								) const {
	static const config::t_token z_filter_self("filter_self", false);
	static const config::t_token z_affect_self("affect_self", false);

	int illuminates = -1;
	const config &filter = cfg.child(z_filter_self);
	bool affect_self = cfg[z_affect_self].to_bool(true);
	if (!filter || !affect_self) return affect_self;
	return matches_filter(vconfig(filter),
						  loc, cache_illuminates(illuminates, ability),game_map, units, teams, lua_kernel, tod_manager);
}

bool unit::has_ability_type(const config::t_token& ability) const
{
	static const config::t_token z_abilities("abilities", false);

	if (const config &list = cfg_.child(z_abilities)) {
		config::const_child_itors itors = list.child_range(ability);
		return itors.first != itors.second;
	}
	return false;
}


bool unit_ability_list::empty() const
{
	return cfgs.empty();
}

std::pair<int,map_location> unit_ability_list::highest(const config::t_token& key, int def) const {
	static const config::t_token z_cumulative("cumulative", false);
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
	foreach (pt const &p, cfgs)
	{
		int value = (*p.first)[key].to_int(def);
		if ((*p.first)[z_cumulative].to_bool()) {
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

std::pair<int,map_location> unit_ability_list::lowest(const config::t_token& key, int def) const
{
	static const config::t_token z_cumulative("cumulative", false);

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
	foreach (pt const &p, cfgs)
	{
		int value = (*p.first)[key].to_int(def);
		if ((*p.first)[z_cumulative].to_bool()) {
			stack += value;
			if (value < 0) value = -value;
			if (only_cumulative && value <= abs_max) {
				abs_max = value;
				best_loc = p.second;
			}
		} else if (only_cumulative || value < flat) {
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
	                           const config::t_token& id, bool just_peeking=false) {
	static const config::t_token z_id("id", false);

		foreach (const config::any_child &sp, parent.all_children_range())
		{
			if (sp.key == id || sp.cfg[z_id] == id) {
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

bool attack_type::get_special_bool(const config::t_token& special,bool force) const
{
	static const config::t_token z_specials("specials", false);

//	log_scope("get_special_bool");
	if (const config &specials = cfg_.child(z_specials))
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
	if (const config &specials = other_attack_->cfg_.child(z_specials))
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

unit_ability_list attack_type::get_specials(const config::t_token& special) const {
	static const config::t_token z_specials("specials", false);

//	log_scope("get_specials");
	unit_ability_list res;
	if (const config &specials = cfg_.child(z_specials))
	{
		foreach (const config &i, specials.child_range(special)) {
			if (special_active(i, true))
				res.cfgs.push_back(std::pair<const config *, map_location>
					(&i, attacker_ ? aloc_ : dloc_));
		}
	}
	if (!other_attack_) return res;
	if (const config &specials = other_attack_->cfg_.child(z_specials))
	{
		foreach (const config &i, specials.child_range(special)) {
			if (other_attack_->special_active(i, false))
				res.cfgs.push_back(std::pair<const config *, map_location>
					(&i, attacker_ ? dloc_ : aloc_));
		}
	}
	return res;
}
std::vector<t_string> attack_type::special_tooltips(bool force) const
{
	static const config::t_token z_specials("specials", false);
	static const config::t_token z_name("name", false);
	static const config::t_token z_description("description", false);
	static const config::t_token z_name_inactive("name_inactive", false);
	static const config::t_token z_description_inactive("description_inactive", false);

//	log_scope("special_tooltips");
	std::vector<t_string> res;
	const config &specials = cfg_.child(z_specials);
	if (!specials) return res;

	foreach (const config::any_child &sp, specials.all_children_range())
	{
		if (force || special_active(sp.cfg, true)) {
			const config::attribute_value &name = sp.cfg[z_name];
			if (!name.empty()) {
				res.push_back(name.t_str());
				res.push_back(sp.cfg[z_description].t_str());
			}
		} else {
			config::attribute_value const &name = sp.cfg[z_name_inactive];
			if (!name.empty()) {
				res.push_back(name.t_str());
				res.push_back(sp.cfg[z_description_inactive].t_str());
			}
		}
	}
	return res;
}
config::t_token attack_type::weapon_specials(bool force) const
{
	static const config::t_token z_specials("specials", false);
	static const config::t_token z_empty("", false);
	static const config::t_token z_name("name", false);
	static const config::t_token z_name_inactive("name_inactive", false);

//	log_scope("weapon_specials");
	const config &specials = cfg_.child(z_specials);
	if (!specials) return z_empty;

	std::string res;
	foreach (const config::any_child &sp, specials.all_children_range())
	{
		config::t_token const *s = force || special_active(sp.cfg, true) ?
			&z_name : &z_name_inactive;
		config::attribute_value const &name = sp.cfg[*s];

		if (!name.empty()) {
			if (!res.empty()) { res +=  ',' ; }
		res += (* name.token() );
		}
	}

return config::t_token( res );
}



/*
 *
 * cfg: a weapon special WML structure
 *
 */
bool attack_type::special_active(gamemap const & game_map, unit_map const & units,
								 t_teams const & teams, LuaKernel & lua_kernel,
								 tod_manager const & tod_manager,
								 const config& cfg, bool self) const
{
	static const config::t_token z_active_on("active_on", false);
	static const config::t_token z_offense("offense", false);
	static const config::t_token z_filter_self("filter_self", false);
	static const config::t_token z_filter_weapon("filter_weapon", false);
	static const config::t_token z_filter_opponent("filter_opponent", false);
	static const config::t_token z_defense("defense", false);
	static const config::t_token z_filter_attacker("filter_attacker", false);
	static const config::t_token z_filter_defender("filter_defender", false);
	static const config::t_token z_filter_adjacent("filter_adjacent", false);
	static const config::t_token z_adjacent("adjacent", false);
	static const config::t_token z_filter_adjacent_location("filter_adjacent_location", false);

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
			config::attribute_value const &active = cfg[z_active_on];
			if (!active.empty() && active != z_offense)
				return false;
		}
		if (const config &filter_self = cfg.child(z_filter_self))
		{
			if (att == unitmap_->end() ||
			    !att->matches_filter(vconfig(filter_self), aloc_, false, game_map, units, teams, lua_kernel, tod_manager))
				return false;
			if (const config &filter_weapon = filter_self.child(z_filter_weapon)) {
				if (!matches_filter(filter_weapon, true))
					return false;
			}
		}
		if (const config &filter_opponent = cfg.child(z_filter_opponent))
		{
			if (def == unitmap_->end() ||
			    !def->matches_filter(vconfig(filter_opponent), dloc_,false, game_map, units, teams, lua_kernel, tod_manager))
				return false;
			if (const config &filter_weapon = filter_opponent.child(z_filter_weapon)) {
				if (!other_attack_ ||
				    !other_attack_->matches_filter( filter_weapon, true))
					return false;
			}
		}
	} else {
		{
			config::attribute_value const &active = cfg[z_active_on];
			if (!active.empty() && active != z_defense)
				return false;
		}
		if (const config &filter_self = cfg.child(z_filter_self))
		{
			if (def == unitmap_->end() ||
			    !def->matches_filter(vconfig(filter_self), dloc_, false, game_map, units, teams, lua_kernel, tod_manager))
				return false;
			if (const config &filter_weapon = filter_self.child(z_filter_weapon)) {
				if (!matches_filter( filter_weapon, true))
					return false;
			}
		}
		if (const config &filter_opponent = cfg.child(z_filter_opponent))
		{
			if (att == unitmap_->end() ||
			    !att->matches_filter(vconfig(filter_opponent), aloc_, false, game_map, units, teams, lua_kernel, tod_manager))
				return false;
			if (const config &filter_weapon = filter_opponent.child(z_filter_weapon)) {
				if (!other_attack_ ||
				    !other_attack_->matches_filter( filter_weapon, true))
					return false;
			}
		}
	}
	if (const config &filter_attacker = cfg.child(z_filter_attacker))
	{
		if (att == unitmap_->end() ||
		    !att->matches_filter(vconfig(filter_attacker), aloc_, false, game_map, units, teams, lua_kernel, tod_manager))
			return false;
		if (const config &filter_weapon = filter_attacker.child(z_filter_weapon))
		{
			if (attacker_) {
				if (!matches_filter(filter_weapon, true))
					return false;
			} else {
				if (!other_attack_ ||
				    !other_attack_->matches_filter( filter_weapon, true))
					return false;
			}
		}
	}
	if (const config &filter_defender = cfg.child(z_filter_defender))
	{
		if (def == unitmap_->end() ||
		    !def->matches_filter(vconfig(filter_defender), dloc_, false, game_map, units, teams, lua_kernel, tod_manager))
			return false;
		if (const config &filter_weapon = filter_defender.child(z_filter_weapon))
		{
			if (!attacker_) {
				if(!matches_filter( filter_weapon, true))
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

	foreach (const config &i, cfg.child_range(z_filter_adjacent))
	{
		foreach (const config::t_token &j, utils::split_attr(i[z_adjacent]))
		{
			map_location::DIRECTION index =
				map_location::parse_direction(j);
			if (index == map_location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = unitmap_->find(adjacent[index]);
			if (unit == unitmap_->end() ||
			    !unit->matches_filter(vconfig(i), unit->get_location(), false, game_map, units, teams, lua_kernel, tod_manager))
				return false;
		}
	}

	foreach (const config &i, cfg.child_range(z_filter_adjacent_location))
	{
		foreach (const config::t_token &j, utils::split_attr( (i[z_adjacent]) ))
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
	static const config::t_token z_apply_to("apply_to", false);
	static const config::t_token z_both("both", false);
	static const config::t_token z_opponent("opponent", false);
	static const config::t_token z_defender("defender", false);
	static const config::t_token z_attacker("attacker", false);

//	log_scope("special_affects_opponent");
	config::attribute_value const &apply_to = cfg[z_apply_to];
	if (apply_to.empty())
		return false;
	if (apply_to == z_both)
		return true;
	if (apply_to == z_opponent)
		return true;
	if (attacker_ && apply_to == z_defender)
		return true;
	if (!attacker_ && apply_to == z_attacker)
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
	static const config::t_token z_apply_to("apply_to", false);
	static const config::t_token z_both("both", false);
	static const config::t_token z_self("self", false);
	static const config::t_token z_attacker("attacker", false);
	static const config::t_token z_defender("defender", false);

//	log_scope("special_affects_self");
	config::attribute_value const &apply_to = cfg[z_apply_to];
	if (apply_to.empty())
		return true;
	if (apply_to == z_both)
		return true;
	if (apply_to == z_self)
		return true;
	if (attacker_ && apply_to == z_attacker)
		return true;
	if (!attacker_ && apply_to == z_defender)
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

void attack_type::set_specials_context(const map_location& loc, const map_location& dloc, const unit& /*un*/, bool attacker) const
{
	aloc_ = loc;
	dloc_ = dloc;
	unitmap_ = resources::units;
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
	static const config::t_token z_filter_base_value("filter_base_value", false);
	static const config::t_token z_equals("equals", false);
	static const config::t_token z_not_equals("not_equals", false);
	static const config::t_token z_less_than("less_than", false);
	static const config::t_token z_greater_than("greater_than", false);
	static const config::t_token z_greater_than_equal_to("greater_than_equal_to", false);
	static const config::t_token z_less_than_equal_to("less_than_equal_to", false);

	if (const config &apply_filter = cfg.child(z_filter_base_value)) {
		config::attribute_value cond_eq = apply_filter[z_equals];
		config::attribute_value cond_ne = apply_filter[z_not_equals];
		config::attribute_value cond_lt = apply_filter[z_less_than];
		config::attribute_value cond_gt = apply_filter[z_greater_than];
		config::attribute_value cond_ge = apply_filter[z_greater_than_equal_to];
		config::attribute_value cond_le = apply_filter[z_less_than_equal_to];
		return  (cond_eq.empty() || def == cond_eq.to_int()) &&
			(cond_ne.empty() || def != cond_ne.to_int()) &&
			(cond_lt.empty() || def <  cond_lt.to_int()) &&
			(cond_gt.empty() || def >  cond_gt.to_int()) &&
			(cond_ge.empty() || def >= cond_ge.to_int()) &&
			(cond_le.empty() || def <= cond_le.to_int());
	}
	return true;
}

effect::effect(const unit_ability_list& list, int def, bool backstab) :
	effect_list_(),
	composite_value_(0)
{
	static const config::t_token z_id("id", false);
	static const config::t_token z_name("name", false);
	static const config::t_token z_backstab("backstab", false);
	static const config::t_token z_value("value", false);
	static const config::t_token z_cumulative("cumulative", false);
	static const config::t_token z_add("add", false);
	static const config::t_token z_sub("sub", false);
	static const config::t_token z_multiply("multiply", false);
	static const config::t_token z_divide("divide", false);


	int value_set = def;
	bool value_is_set = false;
	std::map<std::string, individual_effect> values_add;
	std::map<std::string, individual_effect> values_mul;
	std::map<std::string, individual_effect> values_div;

	individual_effect set_effect;

	for (std::vector< std::pair<const config *, map_location> >::const_iterator
	     i = list.cfgs.begin(), i_end = list.cfgs.end(); i != i_end; ++i) {
		const config& cfg = (*i->first);
		std::string const &effect_id = cfg[cfg[z_id].empty() ? z_name : z_id];

		if (!backstab && cfg[z_backstab].to_bool())
			continue;
		if (!filter_base_matches(cfg, def))
			continue;

		if (const config::attribute_value *v = cfg.get(z_value)) {
			int value = *v;
			bool cumulative = cfg[z_cumulative].to_bool();
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

		if (const config::attribute_value *v = cfg.get(z_add)) {
			int add = *v;
			std::map<std::string,individual_effect>::iterator add_effect = values_add.find(effect_id);
			if(add_effect == values_add.end() || add > add_effect->second.value) {
				values_add[effect_id].set(ADD,add,i->first,i->second);
			}
		}
		if (const config::attribute_value *v = cfg.get(z_sub)) {
			int sub = - *v;
			std::map<std::string,individual_effect>::iterator sub_effect = values_add.find(effect_id);
			if(sub_effect == values_add.end() || sub > sub_effect->second.value) {
				values_add[effect_id].set(ADD,sub,i->first,i->second);
			}
		}
		if (const config::attribute_value *v = cfg.get(z_multiply)) {
			int multiply = int(v->to_double() * 100);
			std::map<std::string,individual_effect>::iterator mul_effect = values_mul.find(effect_id);
			if(mul_effect == values_mul.end() || multiply > mul_effect->second.value) {
				values_mul[effect_id].set(MUL,multiply,i->first,i->second);
			}
		}
		if (const config::attribute_value *v = cfg.get(z_divide)) {
			if (v->to_int() == 0) {
				ERR_NG << "division by zero with divide= in ability/weapon special " << effect_id << "\n";
			}
			else {
				int divide = int(v->to_double() * 100);
				std::map<std::string,individual_effect>::iterator div_effect = values_div.find(effect_id);
				if(div_effect == values_div.end() || divide > div_effect->second.value) {
					values_div[effect_id].set(DIV,divide,i->first,i->second);
				}
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
	for (e = values_div.begin(), e_end = values_div.end(); e != e_end; ++e) {
		multiplier *= 100;
		divisor *= e->second.value;
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

