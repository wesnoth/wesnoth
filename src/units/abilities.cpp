/*
   Copyright (C) 2006 - 2017 by Dominic Bolin <dominic.bolin@exong.net>
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

#include "display_context.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/abilities.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"
#include "filter_context.hpp"
#include <boost/dynamic_bitset.hpp>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

namespace {
	class temporary_facing
	{
		map_location::DIRECTION save_dir_;
		unit_map::const_iterator u_;
	public:
		temporary_facing(unit_map::const_iterator u, map_location::DIRECTION new_dir)
			: save_dir_(u.valid() ? u->facing() : map_location::NDIRECTIONS)
			, u_(u)
		{
			if (u_.valid()) {
				u_->set_facing(new_dir);
			}
		}
		~temporary_facing()
		{
			if (u_.valid()) {
				u_->set_facing(save_dir_);
			}
		}
	};
}

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
 *	female_name= _ "female^heals"
 *	name_inactive=null
 *	female_name_inactive=null
 *	description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
 *	description_inactive=null
 *
 *	affect_self=yes
 *	[filter] // SUF
 *		...
 *	[/filter]
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


namespace {

bool affects_side(const config& cfg, const std::vector<team>& teams, size_t side, size_t other_side)
{
	if (side == other_side)
		return cfg["affect_allies"].to_bool(true);
	if (teams[side - 1].is_enemy(other_side))
		return cfg["affect_enemies"].to_bool();
	else
		return cfg["affect_allies"].to_bool();
}

}


bool unit::get_ability_bool(const std::string& tag_name, const map_location& loc, const display_context& dc) const
{
	for (const config &i : this->abilities_.child_range(tag_name)) {
		if (ability_active(tag_name, i, loc) &&
			ability_affects_self(tag_name, i, loc))
		{
			return true;
		}
	}

	assert(resources::units);
	const unit_map& units = *resources::units;

	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units.find(adjacent[i]);
		if (it == units.end() || it->incapacitated())
			continue;
		// Abilities may be tested at locations other than the unit's current
		// location. This is intentional to allow for less messing with the unit
		// map during calculations, particularly with regards to movement.
		// Thus, we need to make sure the adjacent unit (*it) is not actually
		// ourself.
		if ( &*it == this )
			continue;
		for (const config &j : it->abilities_.child_range(tag_name)) {
			if (affects_side(j, dc.teams(), side(), it->side()) &&
			    it->ability_active(tag_name, j, adjacent[i]) &&
			    ability_affects_adjacent(tag_name,  j, i, loc, *it))
			{
				return true;
			}
		}
	}


	return false;
}
unit_ability_list unit::get_abilities(const std::string& tag_name, const map_location& loc) const
{
	unit_ability_list res;

	for (const config &i : this->abilities_.child_range(tag_name)) {
		if (ability_active(tag_name, i, loc) &&
			ability_affects_self(tag_name, i, loc))
		{
			res.push_back(unit_ability(&i, loc));
		}
	}

	assert(resources::units);
	const unit_map& units = *resources::units;

	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units.find(adjacent[i]);
		if (it == units.end() || it->incapacitated())
			continue;
		// Abilities may be tested at locations other than the unit's current
		// location. This is intentional to allow for less messing with the unit
		// map during calculations, particularly with regards to movement.
		// Thus, we need to make sure the adjacent unit (*it) is not actually
		// ourself.
		if ( &*it == this )
			continue;
		for (const config &j : it->abilities_.child_range(tag_name)) {
			if (affects_side(j, resources::gameboard->teams(), side(), it->side()) &&
			    it->ability_active(tag_name, j, adjacent[i]) &&
			    ability_affects_adjacent(tag_name, j, i, loc, *it))
			{
				res.push_back(unit_ability(&j, adjacent[i]));
			}
		}
	}


	return res;
}

std::vector<std::string> unit::get_ability_list() const
{
	std::vector<std::string> res;

	for (const config::any_child &ab : this->abilities_.all_children_range()) {
		std::string id = ab.cfg["id"];
		if (!id.empty())
			res.push_back(std::move(id));
	}
	return res;
}


namespace {
	// These functions might have wider usefulness than this file, but for now
	// I'll make them local.

	/**
	 * Chooses a value from the given config. If the value specified by @a key is
	 * blank, then @a default_key is chosen instead.
	 */
	inline const config::attribute_value & default_value(
		const config & cfg, const std::string & key, const std::string & default_key)
	{
		const config::attribute_value & value = cfg[key];
		return !value.blank() ? value : cfg[default_key];
	}

	/**
	 * Chooses a value from the given config based on gender. If the value for
	 * the specified gender is blank, then @a default_key is chosen instead.
	 */
	inline const config::attribute_value & gender_value(
		const config & cfg, unit_race::GENDER gender, const std::string & male_key,
		const std::string & female_key, const std::string & default_key)
	{
		return default_value(cfg,
		                     gender == unit_race::MALE ? male_key : female_key,
		                     default_key);
	}
}

std::vector<std::tuple<t_string, t_string, t_string> > unit::ability_tooltips(boost::dynamic_bitset<>* active_list) const
{
	std::vector<std::tuple<t_string,t_string,t_string> > res;
	if ( active_list )
		active_list->clear();

	for (const config::any_child &ab : this->abilities_.all_children_range())
	{
		if ( !active_list || ability_active(ab.key, ab.cfg, loc_) )
		{
			t_string const &name =
				gender_value(ab.cfg, gender_, "name", "female_name", "name").t_str();

			if (!name.empty()) {
				res.push_back(std::make_tuple(
						ab.cfg["name"].t_str(),
						name,
						ab.cfg["description"].t_str() ));
				if ( active_list )
					active_list->push_back(true);
			}
		}
		else
		{
			// See if an inactive name was specified.
			config::attribute_value const &inactive_value =
				gender_value(ab.cfg, gender_, "name_inactive",
				             "female_name_inactive", "name_inactive");
			t_string const &name = !inactive_value.blank() ? inactive_value.t_str() :
				gender_value(ab.cfg, gender_, "name", "female_name", "name").t_str();

			if (!name.empty()) {
				res.push_back(std::make_tuple(
						default_value(ab.cfg, "name_inactive", "name").t_str(),
						name,
						default_value(ab.cfg, "description_inactive", "description").t_str() ));
				active_list->push_back(false);
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
bool unit::ability_active(const std::string& ability,const config& cfg,const map_location& loc) const
{
	bool illuminates = ability == "illuminates";

	if (const config &afilter = cfg.child("filter"))
		if ( !unit_filter(vconfig(afilter), resources::filter_con, illuminates).matches(*this, loc) )
			return false;

	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);

	assert(resources::units);
	const unit_map& units = *resources::units;

	for (const config &i : cfg.child_range("filter_adjacent"))
	{
		size_t count = 0;
		const unit_filter ufilt(vconfig(i), resources::filter_con, illuminates);
		std::vector<map_location::DIRECTION> dirs = map_location::parse_directions(i["adjacent"]);
		for (const map_location::DIRECTION index : dirs)
		{
			if (index == map_location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = units.find(adjacent[index]);
			if (unit == units.end())
				return false;
			if (!ufilt(*unit, *this))
				return false;
			if (i.has_attribute("is_enemy")) {
				const display_context& dc = resources::filter_con->get_disp_context();
				if (i["is_enemy"].to_bool() != dc.get_team(unit->side()).is_enemy(side_)) {
					continue;
				}
			}
			count++;
		}
		if (i["count"].empty() && count != dirs.size()) {
			return false;
		}
		if (!in_ranges<int>(count, utils::parse_ranges(i["count"].str()))) {
			return false;
		}
	}

	for (const config &i : cfg.child_range("filter_adjacent_location"))
	{
		size_t count = 0;
		terrain_filter adj_filter(vconfig(i), resources::filter_con);
		adj_filter.flatten(illuminates);

		std::vector<map_location::DIRECTION> dirs = map_location::parse_directions(i["adjacent"]);
		for (const map_location::DIRECTION index : dirs)
		{
			if (index == map_location::NDIRECTIONS) {
				continue;
			}
			if(!adj_filter.match(adjacent[index])) {
				return false;
			}
			count++;
		}
		if (i["count"].empty() && count != dirs.size()) {
			return false;
		}
		if (!in_ranges<int>(count, utils::parse_ranges(i["count"].str()))) {
			return false;
		}
	}
	return true;
}
/*
 *
 * cfg: an ability WML structure
 *
 */
bool unit::ability_affects_adjacent(const std::string& ability, const config& cfg,int dir,const map_location& loc,const unit& from) const
{
	bool illuminates = ability == "illuminates";

	assert(dir >=0 && dir <= 5);
	map_location::DIRECTION direction = static_cast<map_location::DIRECTION>(dir);

	for (const config &i : cfg.child_range("affect_adjacent"))
	{
		if (i.has_attribute("adjacent")) { //key adjacent defined
			std::vector<map_location::DIRECTION> dirs = map_location::parse_directions(i["adjacent"]);
			if (std::find(dirs.begin(), dirs.end(), direction) == dirs.end()) {
				continue;
			}
		}
		const config &filter = i.child("filter");
		if (!filter || //filter tag given
			unit_filter(vconfig(filter), resources::filter_con, illuminates).matches(*this, loc, from) ) {
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
	const config &filter = cfg.child("filter_self");
	bool affect_self = cfg["affect_self"].to_bool(true);
	if (!filter || !affect_self) return affect_self;
	return unit_filter(vconfig(filter), resources::filter_con, ability == "illuminates").matches(*this, loc);
}

bool unit::has_ability_type(const std::string& ability) const
{
	return !abilities_.child_range(ability).empty();
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
	for (unit_ability const &p : cfgs_)
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
	for (unit_ability const &p : cfgs_)
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
 *	apply_to=self  #self,opponent,defender,attacker,both
 *	#active_on=defense # or offense; omitting this means "both"
 *
 *	swarm_attacks_max=4
 *	swarm_attacks_min=2
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
		for (const config::any_child &sp : parent.all_children_range())
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
	{
		std::vector<const config*> list;
		if ( get_special_children(list, specials_, special, simple_check) ) {
			return true;
		}
		// If we make it to here, then either list.empty() or !simple_check.
		// So if the list is not empty, then this is not a simple check and
		// we need to check each special in the list to see if any are active.
		for (std::vector<const config*>::iterator i = list.begin(), i_end = list.end(); i != i_end; ++i) {
			if ( special_active(**i, AFFECT_SELF) ) {
				return true;
			}
		}
	}
	// Skip checking the opponent's attack?
	if ( simple_check || !other_attack_ ) {
		return false;
	}

	std::vector<const config*> list;
	get_special_children(list, other_attack_->specials_, special);
	for (std::vector<const config*>::iterator i = list.begin(), i_end = list.end(); i != i_end; ++i) {
		if ( other_attack_->special_active(**i, AFFECT_OTHER) ) {
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
	for (const config &i : specials_.child_range(special)) {
		if ( special_active(i, AFFECT_SELF) )
			res.push_back(unit_ability(&i, self_loc_));
	}
	if (!other_attack_) return res;
	for (const config &i : other_attack_->specials_.child_range(special)) {
		if ( other_attack_->special_active(i, AFFECT_OTHER) )
			res.push_back(unit_ability(&i, other_loc_));
	}
	return res;
}

/**
 * Returns a vector of names and descriptions for the specials of *this.
 * Each std::pair in the vector has first = name and second = description.
 *
 * This uses either the active or inactive name/description for each special,
 * based on the current context (see set_specials_context), provided
 * @a active_list is not nullptr. Otherwise specials are assumed active.
 * If the appropriate name is empty, the special is skipped.
 */
std::vector<std::pair<t_string, t_string> > attack_type::special_tooltips(
	boost::dynamic_bitset<>* active_list) const
{
	//log_scope("special_tooltips");
	std::vector<std::pair<t_string, t_string> > res;
	if ( active_list )
		active_list->clear();

	for (const config::any_child &sp : specials_.all_children_range())
	{
		if ( !active_list || special_active(sp.cfg, AFFECT_EITHER) ) {
			const t_string &name = sp.cfg["name"];
			if (!name.empty()) {
				res.push_back(std::make_pair(name, sp.cfg["description"].t_str() ));
				if ( active_list )
					active_list->push_back(true);
			}
		} else {
			t_string const &name = default_value(sp.cfg, "name_inactive", "name").t_str();
			if (!name.empty()) {
				res.push_back(std::make_pair(
					name, default_value(sp.cfg, "description_inactive", "description").t_str() ));
				active_list->push_back(false);
			}
		}
	}
	return res;
}

/**
 * Returns a comma-separated string of active names for the specials of *this.
 * Empty names are skipped.
 *
 * This excludes inactive specials if only_active is true. Whether or not a
 * special is active depends on the current context (see set_specials_context)
 * and the @a is_backstab parameter.
 */
std::string attack_type::weapon_specials(bool only_active, bool is_backstab) const
{
	//log_scope("weapon_specials");
	std::string res;
	for (const config::any_child &sp : specials_.all_children_range())
	{
		if ( only_active  &&  !special_active(sp.cfg, AFFECT_EITHER, is_backstab) )
			continue;

		const std::string& name = sp.cfg["name"].str();
		if (!name.empty()) {
			if (!res.empty()) res += ", ";
			res += name;
		}
	}

	return res;
}


/**
 * Sets the context under which specials will be checked for being active.
 * This version is appropriate if both units in a combat are known.
 * @param[in]  unit_loc      The location of the unit with this weapon.
 * @param[in]  other_loc     The location of the other unit in the combat.
 * @param[in]  attacking     Whether or not the unit with this weapon is the attacker.
 * @param[in]  other_attack  The attack used by the other unit.
 */
void attack_type::set_specials_context(const map_location& unit_loc,
                                       const map_location& other_loc,
                                       bool attacking,
                                       const_attack_ptr other_attack) const
{
	self_loc_ = unit_loc;
	other_loc_ = other_loc;
	is_attacker_ = attacking;
	other_attack_ = other_attack;
	is_for_listing_ = false;
}

/**
 * Sets the context under which specials will be checked for being active.
 * This version is appropriate if there is no specific combat being considered.
 * @param[in]  loc           The location of the unit with this weapon.
 * @param[in]  attacking     Whether or not the unit with this weapon is the attacker.
 */
void attack_type::set_specials_context(const map_location& loc, bool attacking) const
{
	self_loc_ = loc;
	other_loc_ = map_location::null_location();
	is_attacker_ = attacking;
	other_attack_ = nullptr;
	is_for_listing_ = false;
}

void attack_type::set_specials_context_for_listing() const
{
	is_for_listing_ = true;
}


/**
 * Calculates the number of attacks this weapon has, considering specials.
 * This returns two numbers because of the swarm special. The actual number of
 * attacks depends on the unit's health and should be:
 *   min_attacks + (max_attacks - min_attacks) * (current hp) / (max hp)
 * c.f. swarm_blows()
 */
void attack_type::modified_attacks(bool is_backstab, unsigned & min_attacks,
                                   unsigned & max_attacks) const
{
	// Apply [attacks].
	unit_abilities::effect attacks_effect(get_specials("attacks"),
	                                      num_attacks(), is_backstab);
	int attacks_value = attacks_effect.get_composite_value();
	if ( attacks_value < 0 ) {
		attacks_value = num_attacks();
		ERR_NG << "negative number of strikes after applying weapon specials" << std::endl;
	}

	// Apply [swarm].
	unit_ability_list swarm_specials = get_specials("swarm");
	if ( !swarm_specials.empty() ) {
		min_attacks = std::max<int>(0, swarm_specials.highest("swarm_attacks_min").first);
		max_attacks = std::max<int>(0, swarm_specials.highest("swarm_attacks_max", attacks_value).first);
	} else {
		min_attacks = max_attacks = attacks_value;
	}
}


/**
 * Returns the damage per attack of this weapon, considering specials.
 */
int attack_type::modified_damage(bool is_backstab) const
{
	unit_abilities::effect dmg_effect(get_specials("damage"), damage(), is_backstab);
	return dmg_effect.get_composite_value();
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
		const std::string& apply_to = special["apply_to"];
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
		const std::string& apply_to = special["apply_to"];
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

	/**
	 * Determines if a unit/weapon combination matches the specified child
	 * (normally a [filter_*] child) of the provided filter.
	 * @param[in]  un_it      The unit to filter.
	 * @param[in]  loc        The presumed location of @a un_it.
	 * @param[in]  weapon     The attack_type to filter.
	 * @param[in]  filter     The filter containing the child filter to use.
	 * @param[in]  child_tag  The tag of the child filter to use.
	 */
	static bool special_unit_matches(const unit_map::const_iterator & un_it,
									 const unit_map::const_iterator & u2,
		                             const map_location & loc,
		                             const_attack_ptr weapon,
		                             const config & filter,
									 const bool for_listing,
		                             const std::string & child_tag)
	{
		if (for_listing && !loc.valid())
			// The special's context was set to ignore this unit, so assume we pass.
			// (This is used by reports.cpp to show active specials when the
			// opponent is not known. From a player's perspective, the special
			// is active, in that it can be used, even though the player might
			// need to select an appropriate opponent.)
			return true;

		const config & filter_child = filter.child(child_tag);
		if ( !filter_child )
			// The special does not filter on this unit, so we pass.
			return true;

		// If the primary unit doesn't exist, there's nothing to match
		if (!un_it.valid()) {
			return false;
		}

		unit_filter ufilt(vconfig(filter_child), resources::filter_con);

		// If the other unit doesn't exist, try matching without it
		if (!u2.valid()) {
			return ufilt.matches(*un_it, loc);
		}

		// Check for a unit match.
		if (!ufilt.matches(*un_it, loc, *u2)) {
			return false;
		}

		// Check for a weapon match.
		if ( const config & filter_weapon = filter_child.child("filter_weapon") ) {
			if ( !weapon || !weapon->matches_filter(filter_weapon) )
				return false;
		}

		// Passed.
		return true;
	}

}//anonymous namespace

/**
 * Returns whether or not the given special is active for the specified unit,
 * based on the current context (see set_specials_context).
 * @param[in] special           a weapon special WML structure
 * @param[in] whom              specifies which combatant we care about
 * @param[in] include_backstab  false if backstab specials should not be active
 *                              (usually true since backstab is usually accounted
 *                              for elsewhere)
 */
bool attack_type::special_active(const config& special, AFFECTS whom,
                                 bool include_backstab) const
{
	//log_scope("special_active");

	// Backstab check
	if ( !include_backstab )
		if ( special["backstab"].to_bool() )
			return false;

	// Does this affect the specified unit?
	if ( whom == AFFECT_SELF ) {
		if ( !special_affects_self(special, is_attacker_) )
			return false;
	}
	if ( whom == AFFECT_OTHER ) {
		if ( !special_affects_opponent(special, is_attacker_) )
			return false;
	}

	// Is this active on attack/defense?
	const std::string & active_on = special["active_on"];
	if ( !active_on.empty() ) {
		if ( is_attacker_  &&  active_on != "offense" )
			return false;
		if ( !is_attacker_  &&  active_on != "defense" )
			return false;
	}

	// Get the units involved.
	assert(resources::units);
	const unit_map& units = *resources::units;

	unit_map::const_iterator self  = units.find(self_loc_);
	unit_map::const_iterator other = units.find(other_loc_);

	// Make sure they're facing each other.
	temporary_facing self_facing(self, self_loc_.get_relative_dir(other_loc_));
	temporary_facing other_facing(other, other_loc_.get_relative_dir(self_loc_));

	// Translate our context into terms of "attacker" and "defender".
	unit_map::const_iterator & att = is_attacker_ ? self : other;
	unit_map::const_iterator & def = is_attacker_ ? other : self;
	const map_location & att_loc   = is_attacker_ ? self_loc_ : other_loc_;
	const map_location & def_loc   = is_attacker_ ? other_loc_ : self_loc_;
	const_attack_ptr att_weapon = is_attacker_ ? shared_from_this() : other_attack_;
	const_attack_ptr def_weapon = is_attacker_ ? other_attack_ : shared_from_this();

	// Filter the units involved.
	if (!special_unit_matches(self, other, self_loc_, shared_from_this(), special, is_for_listing_, "filter_self"))
		return false;
	if (!special_unit_matches(other, self, other_loc_, other_attack_, special, is_for_listing_, "filter_opponent"))
		return false;
	if (!special_unit_matches(att, def, att_loc, att_weapon, special, is_for_listing_, "filter_attacker"))
		return false;
	if (!special_unit_matches(def, att, def_loc, def_weapon, special, is_for_listing_, "filter_defender"))
		return false;

	map_location adjacent[6];
	get_adjacent_tiles(self_loc_, adjacent);

	// Filter the adjacent units.
	for (const config &i : special.child_range("filter_adjacent"))
	{
		size_t count = 0;
		std::vector<map_location::DIRECTION> dirs = map_location::parse_directions(i["adjacent"]);
		unit_filter filter(vconfig(i), resources::filter_con);
		for (const map_location::DIRECTION index : dirs)
		{
			if (index == map_location::NDIRECTIONS)
				continue;
			unit_map::const_iterator unit = units.find(adjacent[index]);
			if (unit == units.end() || !filter.matches(*unit, adjacent[index], *self))
				return false;
			if (i.has_attribute("is_enemy")) {
				const display_context& dc = resources::filter_con->get_disp_context();
				if (i["is_enemy"].to_bool() != dc.get_team(unit->side()).is_enemy(self->side())) {
					continue;
				}
			}
			count++;
		}
		if (i["count"].empty() && count != dirs.size()) {
			return false;
		}
		if (!in_ranges<int>(count, utils::parse_ranges(i["count"].str()))) {
			return false;
		}
	}

	// Filter the adjacent locations.
	for (const config &i : special.child_range("filter_adjacent_location"))
	{
		size_t count = 0;
		std::vector<map_location::DIRECTION> dirs = map_location::parse_directions(i["adjacent"]);
		terrain_filter adj_filter(vconfig(i), resources::filter_con);
		for (const map_location::DIRECTION index : dirs)
		{
			if (index == map_location::NDIRECTIONS)
				continue;
			if(!adj_filter.match(adjacent[index])) {
				return false;
			}
			count++;
		}
		if (i["count"].empty() && count != dirs.size()) {
			return false;
		}
		if (!in_ranges<int>(count, utils::parse_ranges(i["count"].str()))) {
			return false;
		}
	}

	return true;
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

	for (const unit_ability & ability : list) {
		const config& cfg = *ability.first;
		const std::string& effect_id = cfg[cfg["id"].empty() ? "name" : "id"];

		if (!cfg["backstab"].blank()) {
			lg::wml_error() << "The backstab= key in weapon specials is deprecated; use [filter_adjacent] instead\n";
		}

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
				ERR_NG << "division by zero with divide= in ability/weapon special " << effect_id << std::endl;
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

