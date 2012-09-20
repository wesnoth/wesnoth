/* $Id$ */
/*
   Copyright (C) 2006 - 2012 by Dominic Bolin <dominic.bolin@exong.net>
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

#include "gamestatus.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "terrain_filter.hpp"
#include "unit.hpp"
#include "team.hpp"
#include "unit_abilities.hpp"

#include <boost/foreach.hpp>

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

static bool affects_side(const config& cfg, const std::vector<team>& teams, size_t side, size_t other_side)
{
	if (side == other_side)
		return cfg["affect_allies"].to_bool(true);
	if (teams[side - 1].is_enemy(other_side))
		return cfg["affect_enemies"].to_bool();
	else
		return cfg["affect_allies"].to_bool();
}

}


bool unit::get_ability_bool(const std::string& tag_name, const map_location& loc) const
{
	if (const config &abilities = cfg_.child("abilities"))
	{
		BOOST_FOREACH(const config &i, abilities.child_range(tag_name)) {
			if (ability_active(tag_name, i, loc) &&
			    ability_affects_self(tag_name, i, loc))
				return true;
		}
	}

	const unit_map& units = *resources::units;
	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units.find(adjacent[i]);
		if (it == units.end() || it->incapacitated())
			continue;
		const config &adj_abilities = it->cfg_.child("abilities");
		if (!adj_abilities)
			continue;
		BOOST_FOREACH(const config &j, adj_abilities.child_range(tag_name)) {
			if (unit_abilities::affects_side(j, teams_manager::get_teams(), side(), it->side()) &&
			    it->ability_active(tag_name, j, adjacent[i]) &&
			    ability_affects_adjacent(tag_name,  j, i, loc))
				return true;
		}
	}


	return false;
}
unit_ability_list unit::get_abilities(const std::string& tag_name, const map_location& loc) const
{
	unit_ability_list res;

	if (const config &abilities = cfg_.child("abilities"))
	{
		BOOST_FOREACH(const config &i, abilities.child_range(tag_name)) {
			if (ability_active(tag_name, i, loc) &&
			    ability_affects_self(tag_name, i, loc))
				res.push_back(unit_ability(&i, loc));
		}
	}

	const unit_map& units = *resources::units;
	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units.find(adjacent[i]);
		if (it == units.end() || it->incapacitated())
			continue;
		const config &adj_abilities = it->cfg_.child("abilities");
		if (!adj_abilities)
			continue;
		BOOST_FOREACH(const config &j, adj_abilities.child_range(tag_name)) {
			if (unit_abilities::affects_side(j, teams_manager::get_teams(), side(), it->side()) &&
			    it->ability_active(tag_name, j, adjacent[i]) &&
			    ability_affects_adjacent(tag_name, j, i, loc))
				res.push_back(unit_ability(&j, adjacent[i]));
		}
	}


	return res;
}

std::vector<std::string> unit::get_ability_list() const
{
	std::vector<std::string> res;

	const config &abilities = cfg_.child("abilities");
	if (!abilities) return res;
	BOOST_FOREACH(const config::any_child &ab, abilities.all_children_range()) {
		std::string const &id = ab.cfg["id"];
		if (!id.empty())
			res.push_back(id);
	}
	return res;
}

std::vector<boost::tuple<t_string,t_string,t_string> > unit::ability_tooltips(bool force_active) const
{
	std::vector<boost::tuple<t_string,t_string,t_string> > res;

	const config &abilities = cfg_.child("abilities");
	if (!abilities) return res;

	BOOST_FOREACH(const config::any_child &ab, abilities.all_children_range())
	{
		if (force_active || ability_active(ab.key, ab.cfg, loc_))
		{
			t_string const &name =
				gender_ == unit_race::MALE || ab.cfg["female_name"].empty() ?
				ab.cfg["name"].t_str() : ab.cfg["female_name"].t_str();

			if (!name.empty()) {
				res.push_back(boost::make_tuple(
						ab.cfg["name"].t_str(),
						name,
						ab.cfg["description"].t_str()));
			}
		}
		else
		{
			t_string const &name =
				gender_ == unit_race::MALE || ab.cfg["female_name_inactive"].empty() ?
				ab.cfg["name_inactive"].t_str() : ab.cfg["female_name_inactive"].t_str();

			if (!name.empty()) {
				res.push_back(boost::make_tuple(
						ab.cfg["name_inactive"].t_str(),
						name,
						ab.cfg["description_inactive"].t_str()));
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
	assert(resources::units && resources::game_map && resources::teams && resources::tod_manager);

	if (const config &afilter = cfg.child("filter"))
		if (!matches_filter(vconfig(afilter), loc, cache_illuminates(illuminates, ability)))
			return false;

	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	const unit_map& units = *resources::units;

	BOOST_FOREACH(const config &i, cfg.child_range("filter_adjacent"))
	{
		BOOST_FOREACH(const std::string &j, utils::split(i["adjacent"]))
		{
			map_location::DIRECTION index =
				map_location::parse_direction(j);
			if (index == map_location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = units.find(adjacent[index]);
			if (unit == units.end())
				return false;
			if (!unit->matches_filter(vconfig(i), unit->get_location(),
				cache_illuminates(illuminates, ability)))
				return false;
		}
	}

	BOOST_FOREACH(const config &i, cfg.child_range("filter_adjacent_location"))
	{
		BOOST_FOREACH(const std::string &j, utils::split(i["adjacent"]))
		{
			map_location::DIRECTION index = map_location::parse_direction(j);
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
bool unit::ability_affects_adjacent(const std::string& ability, const config& cfg,int dir,const map_location& loc) const
{
	int illuminates = -1;

	assert(dir >=0 && dir <= 5);
	static const std::string adjacent_names[6] = {"n","ne","se","s","sw","nw"};
	BOOST_FOREACH(const config &i, cfg.child_range("affect_adjacent"))
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
	bool affect_self = cfg["affect_self"].to_bool(true);
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


std::pair<int,map_location> unit_ability_list::highest(const std::string& key, int def) const
{
	if ( cfgs_.empty() ) {
		return std::make_pair(def, map_location());
	}
	// The returned location is the best non-cumulative one, if any,
	// the best absolute cumulative one otherwise.
	map_location best_loc;
	bool only_cumulative = true;
	int abs_max = 0;
	int flat = 0;
	int stack = 0;
	BOOST_FOREACH(unit_ability const &p, cfgs_)
	{
		int value = (*p.first)[key].to_int(def);
		if ((*p.first)["cumulative"].to_bool()) {
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

std::pair<int,map_location> unit_ability_list::lowest(const std::string& key, int def) const
{
	if ( cfgs_.empty() ) {
		return std::make_pair(def, map_location());
	}
	// The returned location is the best non-cumulative one, if any,
	// the best absolute cumulative one otherwise.
	map_location best_loc;
	bool only_cumulative = true;
	int abs_max = 0;
	int flat = 0;
	int stack = 0;
	BOOST_FOREACH(unit_ability const &p, cfgs_)
	{
		int value = (*p.first)[key].to_int(def);
		if ((*p.first)["cumulative"].to_bool()) {
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

	/**
	 * Gets the children of @parent (which should be the specials for an
	 * attack_type) and places the ones whose tag or id= matches @a id into
	 * @a result.
	 * If @a just_peeking is set to true, then @a result is not touched;
	 * instead the return value is used to indicate if any matching children
	 * were found.
	 *
	 * @returns  true if @a just_peeking is true and a match was found;
	 *           false otherwise.
	 */
	bool get_special_children(std::vector<const config*>& result, const config& parent,
	                           const std::string& id, bool just_peeking=false) {
		BOOST_FOREACH(const config::any_child &sp, parent.all_children_range())
		{
			if (sp.key == id || sp.cfg["id"] == id) {
				if(just_peeking) {
					return true; // peek succeeded; done
				} else {
					result.push_back(&sp.cfg);
				}
			}
		}
		return false;
	}
}

/**
 * Returns whether or not @a *this has a special with a tag or id equal to
 * @a special. If @a simple_check is set to true, then the check is merely
 * for being present. Otherwise (the default), the check is for a special
 * active in the current context (see set_specials_context), including
 * specials obtained from the opponent's attack.
 */
bool attack_type::get_special_bool(const std::string& special, bool simple_check) const
{
	//log_scope("get_special_bool");
	if (const config &specials = cfg_.child("specials"))
	{
		std::vector<const config*> list;
		if ( get_special_children(list, specials, special, simple_check) )
			return true;

		// If we make it to here, then either list.empty() or !simple_check.
		// So if the list is not empty, then this is not a simple check and
		// we need to check each special in the list to see if any are active.
		for (std::vector<const config*>::iterator i = list.begin(),
		     i_end = list.end(); i != i_end; ++i) {
			if (special_active(**i, true))
				return true;
		}
	}

	// Skip checking the opponent's attack?
	if ( simple_check || !other_attack_ )
		return false;

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

/**
 * Returns the currently active specials as an ability list, given the current
 * context (see set_specials_context).
 */
unit_ability_list attack_type::get_specials(const std::string& special) const
{
	//log_scope("get_specials");
	unit_ability_list res;
	if (const config &specials = cfg_.child("specials"))
	{
		BOOST_FOREACH(const config &i, specials.child_range(special)) {
			if (special_active(i, true))
				res.push_back(unit_ability(&i, attacker_ ? aloc_ : dloc_));
		}
	}
	if (!other_attack_) return res;
	if (const config &specials = other_attack_->cfg_.child("specials"))
	{
		BOOST_FOREACH(const config &i, specials.child_range(special)) {
			if (other_attack_->special_active(i, false))
				res.push_back(unit_ability(&i, attacker_ ? dloc_ : aloc_));
		}
	}
	return res;
}

/**
 * Returns a vector of names and decriptions for the specials of *this.
 * The vector has the format: name, description, name, description, etc.
 * (So the length is always even.)
 *
 * This chooses between active and inactive names and descriptions, based
 * on the current context (see set_specials_context). Setting @a force_active
 * to true causes all specials to be assumed active. If the appropriate
 * name is empty, the special is skipped.
 */
std::vector<t_string> attack_type::special_tooltips(bool force_active) const
{
	//log_scope("special_tooltips");
	std::vector<t_string> res;
	const config &specials = cfg_.child("specials");
	if (!specials) return res;

	BOOST_FOREACH(const config::any_child &sp, specials.all_children_range())
	{
		if ( force_active || special_active(sp.cfg, true) ) {
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

/**
 * Returns a comma-separated string of names for the specials of *this.
 *
 * This chooses between active and inactive names, based on the current
 * context (see set_specials_context). Setting @a force_active to true
 * causes all specials to be assumed active. If the appropriate name is
 * empty, the special is skipped.
 */
std::string attack_type::weapon_specials(bool force_active) const
{
	//log_scope("weapon_specials");
	std::string res;
	const config &specials = cfg_.child("specials");
	if (!specials) return res;

	BOOST_FOREACH(const config::any_child &sp, specials.all_children_range())
	{
		char const *s = force_active || special_active(sp.cfg, true) ?
			"name" : "name_inactive";
		std::string const &name = sp.cfg[s];

		if (!name.empty()) {
			if (!res.empty()) res += ',';
			res += name;
		}
	}

	return res;
}


namespace { // Helpers for attack_type::special_active()

	/**
	 * Returns whether or not the given special affects the opponent of the unit
	 * with the special.
	 * @param[in]  special      a weapon special WML structure
	 * @param[in]  is_attacker  whether or not the unit with the special is the attacker
	 */
	bool special_affects_opponent(const config& special, bool is_attacker)
	{
		//log_scope("special_affects_opponent");
		std::string const &apply_to = special["apply_to"];
		if ( apply_to.empty() )
			return false;
		if ( apply_to == "both" )
			return true;
		if ( apply_to == "opponent" )
			return true;
		if ( is_attacker  &&  apply_to == "defender" )
			return true;
		if ( !is_attacker &&  apply_to == "attacker" )
			return true;
		return false;
	}

	/**
	 * Returns whether or not the given special affects the unit with the special.
	 * @param[in]  special      a weapon special WML structure
	 * @param[in]  is_attacker  whether or not the unit with the special is the attacker
	 */
	bool special_affects_self(const config& special, bool is_attacker)
	{
		//log_scope("special_affects_self");
		std::string const &apply_to = special["apply_to"];
		if ( apply_to.empty() )
			return true;
		if ( apply_to == "both" )
			return true;
		if ( apply_to == "self" )
			return true;
		if ( is_attacker  &&  apply_to == "attacker" )
			return true;
		if ( !is_attacker &&  apply_to == "defender")
			return true;
		return false;
	}

}//anonymous namespace

/**
 * Returns whether or not the given special is active for the current unit,
 * based on the current context (see set_specials_context).
 * @param[in]  special  a weapon special WML structure
 * @param[in]  self     true if the special is from the current unit;
 *                      false if it is from the opponent.
 */
bool attack_type::special_active(const config& special, bool self) const
{
	//log_scope("special_active");
	assert(unitmap_ != NULL);
	unit_map::const_iterator att = unitmap_->find(aloc_);
	unit_map::const_iterator def = unitmap_->find(dloc_);

	if(self) {
		if ( !special_affects_self(special, attacker_) )
			return false;
	} else {
		if ( !special_affects_opponent(special, !attacker_) )
			return false;
	}

	if(attacker_) {
		{
			std::string const &active = special["active_on"];
			if (!active.empty() && active != "offense")
				return false;
		}
		if (const config &filter_self = special.child("filter_self"))
		{
			if (att == unitmap_->end() ||
			    !att->matches_filter(vconfig(filter_self), aloc_))
				return false;
			if (const config &filter_weapon = filter_self.child("filter_weapon")) {
				if (!matches_filter(filter_weapon, true))
					return false;
			}
		}
		if (const config &filter_opponent = special.child("filter_opponent"))
		{
			if (def == unitmap_->end() ||
			    !def->matches_filter(vconfig(filter_opponent), dloc_))
				return false;
			if (const config &filter_weapon = filter_opponent.child("filter_weapon")) {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(filter_weapon, true))
					return false;
			}
		}
	} else {
		{
			std::string const &active = special["active_on"];
			if (!active.empty() && active != "defense")
				return false;
		}
		if (const config &filter_self = special.child("filter_self"))
		{
			if (def == unitmap_->end() ||
			    !def->matches_filter(vconfig(filter_self), dloc_))
				return false;
			if (const config &filter_weapon = filter_self.child("filter_weapon")) {
				if (!matches_filter(filter_weapon, true))
					return false;
			}
		}
		if (const config &filter_opponent = special.child("filter_opponent"))
		{
			if (att == unitmap_->end() ||
			    !att->matches_filter(vconfig(filter_opponent), aloc_))
				return false;
			if (const config &filter_weapon = filter_opponent.child("filter_weapon")) {
				if (!other_attack_ ||
				    !other_attack_->matches_filter(filter_weapon, true))
					return false;
			}
		}
	}
	if (const config &filter_attacker = special.child("filter_attacker"))
	{
		if (att == unitmap_->end() ||
		    !att->matches_filter(vconfig(filter_attacker), aloc_))
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
	if (const config &filter_defender = special.child("filter_defender"))
	{
		if (def == unitmap_->end() ||
		    !def->matches_filter(vconfig(filter_defender), dloc_))
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

	BOOST_FOREACH(const config &i, special.child_range("filter_adjacent"))
	{
		BOOST_FOREACH(const std::string &j, utils::split(i["adjacent"]))
		{
			map_location::DIRECTION index =
				map_location::parse_direction(j);
			if (index == map_location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = unitmap_->find(adjacent[index]);
			if (unit == unitmap_->end() ||
			    !unit->matches_filter(vconfig(i), unit->get_location()))
				return false;
		}
	}

	BOOST_FOREACH(const config &i, special.child_range("filter_adjacent_location"))
	{
		BOOST_FOREACH(const std::string &j, utils::split(i["adjacent"]))
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

/**
 * Sets the context under which specials will be checked for being active.
 * @param[in]  aloc          The location of the attacker.
 * @param[in]  dloc          The location of the defender.
 * @param[in]  unitmap       The unit_map used to find units based on location.
 * @param[in]  attacker      Whether or not the current unit is the attacker.
 * @param[in]  other_attack  The attack used by the other unit.
 */
void attack_type::set_specials_context(const map_location& aloc,const map_location& dloc,
	const unit_map &unitmap, bool attacker, const attack_type *other_attack) const
{
	aloc_ = aloc;
	dloc_ = dloc;
	unitmap_ = &unitmap;
	attacker_ = attacker;
	other_attack_ = other_attack;
}

/**
 * Sets the context under which specials will be checked for being active.
 * @param[in]  loc           The location of the attacker.
 * @param[in]  dloc          The location of the defender.
 * @param[in]  unit          Unused
 * @param[in]  attacker      Whether or not the current unit is the attacker.
 */
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
	if (const config &apply_filter = cfg.child("filter_base_value")) {
		config::attribute_value cond_eq = apply_filter["equals"];
		config::attribute_value cond_ne = apply_filter["not_equals"];
		config::attribute_value cond_lt = apply_filter["less_than"];
		config::attribute_value cond_gt = apply_filter["greater_than"];
		config::attribute_value cond_ge = apply_filter["greater_than_equal_to"];
		config::attribute_value cond_le = apply_filter["less_than_equal_to"];
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

	int value_set = def;
	bool value_is_set = false;
	std::map<std::string,individual_effect> values_add;
	std::map<std::string,individual_effect> values_mul;
	std::map<std::string,individual_effect> values_div;

	individual_effect set_effect;

	BOOST_FOREACH (const unit_ability & ability, list) {
		const config& cfg = *ability.first;
		std::string const &effect_id = cfg[cfg["id"].empty() ? "name" : "id"];

		if (!backstab && cfg["backstab"].to_bool())
			continue;
		if (!filter_base_matches(cfg, def))
			continue;

		if (const config::attribute_value *v = cfg.get("value")) {
			int value = *v;
			bool cumulative = cfg["cumulative"].to_bool();
			if (!value_is_set && !cumulative) {
				value_set = value;
				set_effect.set(SET, value, ability.first, ability.second);
			} else {
				if (cumulative) value_set = std::max<int>(value_set, def);
				if (value > value_set) {
					value_set = value;
					set_effect.set(SET, value, ability.first, ability.second);
				}
			}
			value_is_set = true;
		}

		if (const config::attribute_value *v = cfg.get("add")) {
			int add = *v;
			std::map<std::string,individual_effect>::iterator add_effect = values_add.find(effect_id);
			if(add_effect == values_add.end() || add > add_effect->second.value) {
				values_add[effect_id].set(ADD, add, ability.first, ability.second);
			}
		}
		if (const config::attribute_value *v = cfg.get("sub")) {
			int sub = - *v;
			std::map<std::string,individual_effect>::iterator sub_effect = values_add.find(effect_id);
			if(sub_effect == values_add.end() || sub > sub_effect->second.value) {
				values_add[effect_id].set(ADD, sub, ability.first, ability.second);
			}
		}
		if (const config::attribute_value *v = cfg.get("multiply")) {
			int multiply = int(v->to_double() * 100);
			std::map<std::string,individual_effect>::iterator mul_effect = values_mul.find(effect_id);
			if(mul_effect == values_mul.end() || multiply > mul_effect->second.value) {
				values_mul[effect_id].set(MUL, multiply, ability.first, ability.second);
			}
		}
		if (const config::attribute_value *v = cfg.get("divide")) {
			if (*v == 0) {
				ERR_NG << "division by zero with divide= in ability/weapon special " << effect_id << "\n";
			}
			else {
				int divide = int(v->to_double() * 100);
				std::map<std::string,individual_effect>::iterator div_effect = values_div.find(effect_id);
				if(div_effect == values_div.end() || divide > div_effect->second.value) {
					values_div[effect_id].set(DIV, divide, ability.first, ability.second);
				}
			}
		}
	}

	if(value_is_set && set_effect.type != NOT_USED) {
		effect_list_.push_back(set_effect);
	}

	/* Do multiplication with floating point values rather than integers
	 * We want two places of precision for each multiplier
	 * Using integers multiplied by 100 to keep precision causes overflow
	 *   after 3-4 abilities for 32-bit values and ~8 for 64-bit
	 * Avoiding the overflow by dividing after each step introduces rounding errors
	 *   that may vary depending on the order effects are applied
	 * As the final values are likely <1000 (always true for mainline), loss of less significant digits is not an issue
	 */
	double multiplier = 1.0;
	double divisor = 1.0;
	std::map<std::string,individual_effect>::const_iterator e, e_end;
	for (e = values_mul.begin(), e_end = values_mul.end(); e != e_end; ++e) {
		multiplier *= e->second.value/100.0;
		effect_list_.push_back(e->second);
	}
	for (e = values_div.begin(), e_end = values_div.end(); e != e_end; ++e) {
		divisor *= e->second.value/100.0;
		effect_list_.push_back(e->second);
	}
	int addition = 0;
	for (e = values_add.begin(), e_end = values_add.end(); e != e_end; ++e) {
		addition += e->second.value;
		effect_list_.push_back(e->second);
	}

	composite_value_ = int((value_set + addition) * multiplier / divisor);
}

} // end namespace unit_abilities

