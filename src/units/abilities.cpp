/*
	Copyright (C) 2006 - 2023
	by Dominic Bolin <dominic.bolin@exong.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "display.hpp"
#include "display_context.hpp"
#include "font/text_formatting.hpp"
#include "game_board.hpp"
#include "game_version.hpp" // for version_info
#include "gettext.hpp"
#include "global.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/active_ability_list.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"
#include "filter_context.hpp"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/function_gamestate.hpp"
#include "deprecation.hpp"

#include <boost/dynamic_bitset.hpp>

#include <string_view>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)

namespace {
	class temporary_facing
	{
		map_location::DIRECTION save_dir_;
		unit_const_ptr u_;
	public:
		temporary_facing(unit_const_ptr u, map_location::DIRECTION new_dir)
			: save_dir_(u ? u->facing() : map_location::NDIRECTIONS)
			, u_(u)
		{
			if (u_) {
				u_->set_facing(new_dir);
			}
		}
		~temporary_facing()
		{
			if (u_) {
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
 * In this file use the following terminology: during an evaulation of an ability there are at most 3 unit involved:
 * 1)   The Teacher (the unit that own the ability)
 * 2)   The Student (the unit that is affected by the ability,
 *      usually the same as the teacher), but might also be a different
 *      unit via [affect_adjacent]
 * 2.5) The Opponent of a unit in a fight.
 * 3)   The Recipient, the unit whose stats are modified by the ability,
 *      this can either be the student or the opponenet, depending on apply_to=self/opponent
 */


namespace {

const unit_map& get_unit_map()
{
	// Used if we're in the game, including during the construction of the display_context
	if(resources::gameboard) {
		return resources::gameboard->units();
	}

	// If we get here, we're in the scenario editor
	assert(display::get_singleton());
	return display::get_singleton()->get_units();
}

const team& get_team(std::size_t side)
{
	// Used if we're in the game, including during the construction of the display_context
	if(resources::gameboard) {
		return resources::gameboard->get_team(side);
	}

	// If we get here, we're in the scenario editor
	assert(display::get_singleton());
	return display::get_singleton()->get_disp_context().get_team(side);
}

/**
 * Common code for the question "some other unit has an ability, can that ability affect this
 * unit" - it's not the full answer to that question, just a part of it.
 *
 * Although this is called while checking which units' "hides" abilities are active, that's only
 * for the question "is this unit next to an ally that has a 'camoflages adjacent allies' ability";
 * not the question "is this unit next to an enemy, therefore visible".
 */
bool affects_side(const config& cfg, std::size_t side, std::size_t other_side)
{
	const team& side_team = get_team(side);

	if(side == other_side)
		return cfg["affect_allies"].to_bool(true);
	if(side_team.is_enemy(other_side))
		return cfg["affect_enemies"].to_bool();
	else
		return cfg["affect_allies"].to_bool();
}

}

bool unit::get_ability_bool(const std::string& tag_name, const map_location& loc) const
{
	for (const ability_ptr& ab : abilities(tag_name)) {
		if (ability_active(tag_name, ab->cfg(), loc) &&
			ability_affects_self(tag_name, ab->cfg(), loc))
		{
			return true;
		}
	}

	const unit_map& units = get_unit_map();

	const auto adjacent = get_adjacent_tiles(loc);
	for(unsigned i = 0; i < adjacent.size(); ++i) {
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
		for ( ability_ptr& ab : it->abilities(tag_name)) {
			if (affects_side(ab->cfg(), side(), it->side()) &&
			    it->ability_active(tag_name, ab->cfg(), adjacent[i]) &&
			    ability_affects_adjacent(tag_name,  ab->cfg(), i, loc, *it))
			{
				return true;
			}
		}
	}


	return false;
}

active_ability_list unit::get_abilities(const std::string& tag_name, const map_location& loc) const
{
	active_ability_list res(loc_);


	for (ability_ptr& ab : abilities(tag_name)) {
		if(ability_active(tag_name, ab->cfg(), loc)
			&& ability_affects_self(tag_name, ab->cfg(), loc))
			{
			res.emplace_back(ab, loc, loc);
		}
	}

	const unit_map& units = get_unit_map();

	const auto adjacent = get_adjacent_tiles(loc);
	for(unsigned i = 0; i < adjacent.size(); ++i) {
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
		for (const ability_ptr& ab : it->abilities(tag_name)) {
			if(affects_side(ab->cfg(), side(), it->side())
				&& it->ability_active(tag_name, ab->cfg(), adjacent[i])
				&& ability_affects_adjacent(tag_name, ab->cfg(), i, loc, *it))
				{
				res.emplace_back(ab, loc, adjacent[i]);
			}
		}
	}


	return res;
}

active_ability_list unit::get_abilities_weapons(const std::string& tag_name, const map_location& loc, const_attack_ptr weapon, const_attack_ptr opp_weapon) const
{
	active_ability_list res = get_abilities(tag_name, loc);
	utils::erase_if(res, [&](const active_ability& i) {
		return !ability_affects_weapon(i.ability_cfg(), weapon, false) || !ability_affects_weapon(i.ability_cfg(), opp_weapon, true);
	});
	return res;
}

std::vector<std::string> unit::get_ability_list() const
{
	std::vector<std::string> res;

	for (const ability_ptr ab : this->abilities()) {
		std::string id = ab->cfg()["id"];
		if (!id.empty())
			res.push_back(std::move(id));
	}
	return res;
}


namespace {
	/**
	 * Adds a quadruple consisting of (in order) id, base name,
	 * male or female name as appropriate for the unit, and description.
	 *
	 * @returns Whether name was resolved and quadruple added.
	 */
	bool add_ability_tooltip(const unit_ability_t &ab, unit_race::GENDER gender, std::vector<std::tuple<std::string, t_string,t_string,t_string>>& res, bool active)
	{
		if (active) {
			const t_string& name = gender_value(ab.cfg(), gender, "name", "female_name", "name").t_str();

			if (!name.empty()) {
				res.emplace_back(
						ab.cfg()["id"],
						ab.cfg()["name"].t_str(),
						name,
						ab.cfg()["description"].t_str() );
				return true;
			}
		}
		else
		{
			// See if an inactive name was specified.
			const config::attribute_value& inactive_value =
				gender_value(ab.cfg(), gender, "name_inactive",
						"female_name_inactive", "name_inactive");
			const t_string& name = !inactive_value.blank() ? inactive_value.t_str() :
				gender_value(ab.cfg(), gender, "name", "female_name", "name").t_str();

			if (!name.empty()) {
				res.emplace_back(
						ab.cfg()["id"],
						ab.cfg().get_or("name_inactive", "name").t_str(),
						name,
						ab.cfg().get_or("description_inactive", "description").t_str() );
				return true;
			}
		}

		return false;
	}
}

std::vector<std::tuple<std::string, t_string, t_string, t_string>> unit::ability_tooltips() const
{
	std::vector<std::tuple<std::string, t_string,t_string,t_string>> res;

	for (const ability_ptr ab : this->abilities())
	{
		add_ability_tooltip(*ab, gender_, res, true);
	}

	return res;
}

std::vector<std::tuple<std::string, t_string, t_string, t_string>> unit::ability_tooltips(boost::dynamic_bitset<>& active_list, const map_location& loc) const
{
	std::vector<std::tuple<std::string, t_string,t_string,t_string>> res;
	active_list.clear();

	for (const ability_ptr ab : this->abilities())
	{
		bool active = ability_active(ab->tag(), ab->cfg(), loc);
		if (add_ability_tooltip(*ab, gender_, res, active))
		{
			active_list.push_back(active);
		}
	}
	return res;
}

bool unit::ability_active(const std::string& ability,const config& cfg,const map_location& loc) const
{
	bool illuminates = ability == "illuminates";

	if (auto afilter = cfg.optional_child("filter"))
		if ( !unit_filter(vconfig(*afilter)).set_use_flat_tod(illuminates).matches(*this, loc) )
			return false;

	const auto adjacent = get_adjacent_tiles(loc);

	const unit_map& units = get_unit_map();

	for (const config &i : cfg.child_range("filter_adjacent"))
	{
		std::size_t count = 0;
		unit_filter ufilt{ vconfig(i) };
		ufilt.set_use_flat_tod(illuminates);
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
		std::size_t count = 0;
		terrain_filter adj_filter(vconfig(i), resources::filter_con, false);
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
		auto filter = i.optional_child("filter");
		if (!filter || //filter tag given
			unit_filter(vconfig(*filter)).set_use_flat_tod(illuminates).matches(*this, loc, from) ) {
			return true;
		}
	}
	return false;
}

bool unit::ability_affects_self(const std::string& ability,const config& cfg,const map_location& loc) const
{
	auto filter = cfg.optional_child("filter_self");
	bool affect_self = cfg["affect_self"].to_bool(true);
	if (!filter || !affect_self) return affect_self;
	return unit_filter(vconfig(*filter)).set_use_flat_tod(ability == "illuminates").matches(*this, loc);
}

bool unit::ability_affects_weapon(const config& cfg, const_attack_ptr weapon, bool is_opp) const
{
	const std::string filter_tag_name = is_opp ? "filter_second_weapon" : "filter_weapon";
	if(!cfg.has_child(filter_tag_name)) {
		return true;
	}
	const config& filter = cfg.mandatory_child(filter_tag_name);
	if(!weapon) {
		return false;
	}
	return weapon->matches_filter(filter);
}

bool unit::has_ability_type(const std::string& ability) const
{
	return std::find_if(abilities().begin(), abilities().end(), [ability](const auto& ab) { return ab->tag() == ability; }) != abilities().end();
}

void attack_type::add_formula_context(wfl::map_formula_callable& callable) const
{
	if(unit_const_ptr & att = is_attacker_ ? self_ : other_) {
		callable.add("attacker", wfl::variant(std::make_shared<wfl::unit_callable>(*att)));
	}
	if(unit_const_ptr & def = is_attacker_ ? other_ : self_) {
		callable.add("defender", wfl::variant(std::make_shared<wfl::unit_callable>(*def)));
	}
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
	 * call @a fres, for each element that matches the id (tag matches or id matched dpending on @a special_id @a special_tags)
	 * stops the loop when fres returns true. (used for early exit when we just want to know ehther such an element exists)
	 */
	template<typename F>
	void forerach_ability_children(F fres,
	                           const ability_vector& parent, const std::string& id,
	                           bool special_id=true, bool special_tags=true)
	{
		for (const ability_ptr& sp : parent) {
			bool matches = (special_id  && sp->cfg()["id"] == id) || (special_tags && sp->tag() == id);
			if(matches && fres(sp)) {
				return;
			}
		}
	}

	/**
	 * Gets the children of parent (which should be the abilities for an
	 * attack_type) and places the ones whose tag or id= matches @a id into
	 * @a tag_result and @a id_result.
	 * @param tag_result receive the children whose tag or id matches @a id
	 * @param parent the tags whose contain children (abilities here)
	 * @param id tag or id of child tested
	 * @param special_id if true, children check by id
	 * @param special_tags if true, children check by tags
	 */
	void get_ability_children(ability_vector& res,
	                           const ability_vector& parent, const std::string& id,
	                           bool special_id=true, bool special_tags=true)
	{
		forerach_ability_children([&res](const ability_ptr& sp) {res.push_back(sp); return false; },
	                           parent, id, special_id, special_tags);
	}
	/**
	 * Implementation of attack_type::has_special_simple 
	 * @param parent the tags whose contain children (abilities here)
	 * @param id tag or id of child tested
	 * @param special_id if true, children check by id
	 * @param special_tags if true, children check by tags
	 */
	bool has_ability_children(const ability_vector& parent, const std::string& id,
	                           bool special_id=true, bool special_tags=true)
	{
		bool res;
		forerach_ability_children([&res](const ability_ptr& sp) {res = true; return true; },
	                           parent, id, special_id, special_tags);
		return res;

	}

}

/**
 * Returns whether or not @a *this has a special with a tag or id equal to
 * @a special. The check is merely for being present.
 */
bool attack_type::has_special_simple(const std::string& special, bool check_id, bool check_tags) const
{
	return has_ability_children(specials_, special, check_id, check_tags);
}

/**
 * Returns whether or not @a *this has a special with a tag or id equal to
 * @a special. The check is for a special
 * active in the current context (see set_specials_context), including
 * specials obtained from the opponent's attack.
 */
bool attack_type::has_special(const std::string& special, bool special_id, bool special_tags) const
{
	ability_vector special_matches;
	get_ability_children(special_matches, specials_, special, special_id, special_tags);
	for(const ability_ptr& entry : special_matches) {
		if ( special_active(entry->cfg(), AFFECT_SELF, entry->tag()) ) {
			return true;
		}
	}

	if (other_attack_ ) {
		ability_vector special_matches_opponent;
		get_ability_children(special_matches_opponent, other_attack_->specials_, special, special_id, special_tags);
		for(const ability_ptr& entry : special_matches_opponent) {
			if ( special_active(entry->cfg(), AFFECT_OTHER, entry->tag()) ) {
				return true;
			}
		}
	}

	return false;
}

/**
 * Returns the currently active specials as an ability list, given the current
 * context (see set_specials_context).
 */
active_ability_list attack_type::get_specials(const std::string& special) const
{
	//log_scope("get_specials");
	const map_location loc = self_ ? self_->get_location() : self_loc_;
	active_ability_list res(loc);

	for (ability_ptr& sp : specials(special)) {
		if(special_active(sp->cfg(), AFFECT_SELF, special)) {
			res.emplace_back(std::move(sp), loc, loc);
		}
	}

	if(!other_attack_) {
		return res;
	}

	for(ability_ptr& sp : specials(special)) {
		if(other_attack_->special_active(sp->cfg(), AFFECT_OTHER, special)) {
			res.emplace_back(std::move(sp), other_loc_, other_loc_);
		}
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
std::vector<std::pair<t_string, t_string>> attack_type::special_tooltips(
	boost::dynamic_bitset<>* active_list) const
{
	//log_scope("special_tooltips");
	std::vector<std::pair<t_string, t_string>> res;
	if ( active_list )
		active_list->clear();

	for (ability_ptr sp : specials_)
	{
		if ( !active_list || special_active(sp->cfg(), AFFECT_EITHER, sp->tag()) ) {
			const t_string &name = sp->cfg()["name"];
			if (!name.empty()) {
				res.emplace_back(name, sp->cfg()["description"].t_str() );
				if ( active_list )
					active_list->push_back(true);
			}
		} else {
			const t_string& name = sp->cfg().get_or("name_inactive", "name").t_str();
			if (!name.empty()) {
				res.emplace_back(name, sp->cfg().get_or("description_inactive", "description").t_str() );
				active_list->push_back(false);
			}
		}
	}
	return res;
}

/**
 * static used in weapon_specials (bool only_active) and
 * @return a string and a set_string for the weapon_specials function below.
 * @param[in,out] temp_string the string modified and returned
 * @param[in] active the boolean for determine if @name can be added or not
 * @param[in] name string who must be or not added
 * @param[in,out] checking_name the reference for checking if @name already added
 */
static void add_name(std::string& temp_string, bool active, const std::string name, std::set<std::string>& checking_name)
{
	if (active) {
		if (!name.empty() && checking_name.count(name) == 0) {
			checking_name.insert(name);
			if (!temp_string.empty()) temp_string += ", ";
			temp_string += font::span_color(font::BUTTON_COLOR, name);
		}
	}
}

/**
 * Returns a comma-separated string of active names for the specials of *this.
 * Empty names are skipped.
 *
 * Whether or not a special is active depends
 * on the current context (see set_specials_context)
 */
std::string attack_type::weapon_specials() const
{
	//log_scope("weapon_specials");
	std::string res;
	for (const ability_ptr& sp : specials_)
	{
		const bool active = special_active(sp->cfg(), AFFECT_EITHER, sp->tag());

		const std::string& name =
			active
			? sp->cfg()["name"].str()
			: sp->cfg().get_or("name_inactive", "name").str();
		if (!name.empty()) {
			if (!res.empty()) res += ", ";
			if (!active) res += font::span_color(font::inactive_details_color);
			res += name;
			if (!active) res += "</span>";
		}
	}
	std::string temp_string;
	std::set<std::string> checking_name;
	weapon_specials_impl_self(temp_string, self_, shared_from_this(), other_attack_, self_loc_, AFFECT_SELF, checking_name);
	weapon_specials_impl_adj(temp_string, self_, shared_from_this(), other_attack_, self_loc_, AFFECT_SELF, checking_name, {}, "affect_allies");
	if(!temp_string.empty() && !res.empty()) {
		temp_string = ", \n" + temp_string;
		res += temp_string;
	} else if (!temp_string.empty()){
		res = temp_string;
	}
	return res;
}

static void add_name_list(std::string& temp_string, std::string& weapon_abilities, std::set<std::string>& checking_name, const std::string from_str)
{
	if(!temp_string.empty()){
		temp_string = translation::dsgettext("wesnoth", from_str.c_str()) + temp_string;
		weapon_abilities += (!weapon_abilities.empty() && !temp_string.empty()) ? "\n" : "";
		weapon_abilities += temp_string;
		temp_string.clear();
		checking_name.clear();
	}
}

std::string attack_type::weapon_specials_value(const std::set<std::string> checking_tags) const
{
	//log_scope("weapon_specials_value");
	std::string temp_string, weapon_abilities;
	std::set<std::string> checking_name;
	for (const ability_ptr& sp : specials_) {
		if((checking_tags.count(sp->tag()) != 0)){
			const bool active = special_active(sp->cfg(), AFFECT_SELF, sp->tag());
			add_name(temp_string, active, sp->cfg()["name"].str(), checking_name);
		}
	}
	add_name_list(temp_string, weapon_abilities, checking_name, "");

	weapon_specials_impl_self(temp_string, self_, shared_from_this(), other_attack_, self_loc_, AFFECT_SELF, checking_name, checking_tags, true);
	add_name_list(temp_string, weapon_abilities, checking_name, "Owned: ");

	weapon_specials_impl_adj(temp_string, self_, shared_from_this(), other_attack_, self_loc_, AFFECT_SELF, checking_name, checking_tags, "affect_allies", true);
	add_name_list(temp_string, weapon_abilities, checking_name, "Taught: ");

	weapon_specials_impl_adj(temp_string, self_, shared_from_this(), other_attack_, self_loc_, AFFECT_SELF, checking_name, checking_tags, "affect_enemies", true);
	add_name_list(temp_string, weapon_abilities, checking_name, "Taught: (by an enemy): ");


	if(other_attack_) {
		for (const ability_ptr& sp : specials_) {
			if((checking_tags.count(sp->tag()) != 0)){
				const bool active = other_attack_->special_active(sp->cfg(), AFFECT_OTHER, sp->tag());
				add_name(temp_string, active, sp->cfg()["name"].str(), checking_name);
			}
		}
	}
	weapon_specials_impl_self(temp_string, other_, other_attack_, shared_from_this(), other_loc_, AFFECT_OTHER, checking_name, checking_tags);
	weapon_specials_impl_adj(temp_string, other_, other_attack_, shared_from_this(), other_loc_, AFFECT_OTHER, checking_name, checking_tags);
	add_name_list(temp_string, weapon_abilities, checking_name, "Used by opponent: ");

	return weapon_abilities;
}

void attack_type::weapon_specials_impl_self(
	std::string& temp_string,
	unit_const_ptr self,
	const_attack_ptr self_attack,
	const_attack_ptr other_attack,
	const map_location& self_loc,
	AFFECTS whom,
	std::set<std::string>& checking_name,
	const std::set<std::string>& checking_tags,
	bool leader_bool)
{
	if(self){
		for (const ability_ptr sp : self->abilities()){
			bool tag_checked = (!checking_tags.empty()) ? (checking_tags.count(sp->tag()) != 0) : true;
			const bool active = tag_checked && check_self_abilities_impl(self_attack, other_attack, sp->cfg(), self, self_loc, whom, sp->tag(), leader_bool);
			add_name(temp_string, active, sp->cfg()["name"].str(), checking_name);
		}
	}
}

void attack_type::weapon_specials_impl_adj(
	std::string& temp_string,
	unit_const_ptr self,
	const_attack_ptr self_attack,
	const_attack_ptr other_attack,
	const map_location& self_loc,
	AFFECTS whom,
	std::set<std::string>& checking_name,
	const std::set<std::string>& checking_tags,
	const std::string& affect_adjacents,
	bool leader_bool)
{
	const unit_map& units = get_unit_map();
	if(self){
		const auto adjacent = get_adjacent_tiles(self_loc);
		for(unsigned i = 0; i < adjacent.size(); ++i) {
			const unit_map::const_iterator it = units.find(adjacent[i]);
			if (it == units.end() || it->incapacitated())
				continue;
			if(&*it == self.get())
				continue;
			// sp is no a reference here, to be more robust against that case that the filter
			// changes the unit. (not that we support that but we also don't want to crash)
			// TODO: this is nto enough, we need to make a copy of the abilities vector, to
			// be safe against manipulation.
			for (ability_ptr& sp : it->abilities()){
				bool tag_checked = (!checking_tags.empty()) ? (checking_tags.count(sp->tag()) != 0) : true;
				bool default_bool = (affect_adjacents == "affect_allies") ? true : false;
				bool affect_allies = (!affect_adjacents.empty()) ? sp->cfg()[affect_adjacents].to_bool(default_bool) : true;
				const bool active = tag_checked && check_adj_abilities_impl(self_attack, other_attack, sp->cfg(), self, *it, i, self_loc, whom, sp->tag(), leader_bool) && affect_allies;
				add_name(temp_string, active, sp->cfg()["name"].str(), checking_name);
			}
		}
	}
}


/**
 * Sets the context under which specials will be checked for being active.
 * This version is appropriate if both units in a combat are known.
 * @param[in]  weapon        The weapon being considered.
 * @param[in]  self          A reference to the unit with this weapon.
 * @param[in]  other         A reference to the other unit in the combat.
 * @param[in]  unit_loc      The location of the unit with this weapon.
 * @param[in]  other_loc     The location of the other unit in the combat.
 * @param[in]  attacking     Whether or not the unit with this weapon is the attacker.
 * @param[in]  other_attack  The attack used by the other unit.
 */
attack_type::specials_context_t::specials_context_t(
	const attack_type& weapon,
	const_attack_ptr other_attack,
	unit_const_ptr self,
	unit_const_ptr other,
	const map_location& unit_loc,
	const map_location& other_loc,
	bool attacking)
	: parent(weapon.shared_from_this())
{
	weapon.self_ = self;
	weapon.other_ = other;
	weapon.self_loc_ = unit_loc;
	weapon.other_loc_ = other_loc;
	weapon.is_attacker_ = attacking;
	weapon.other_attack_ = other_attack;
	weapon.is_for_listing_ = false;
}

/**
 * Sets the context under which specials will be checked for being active.
 * This version is appropriate if there is no specific combat being considered.
 * @param[in]  weapon        The weapon being considered.
 * @param[in]  self          A reference to the unit with this weapon.
 * @param[in]  loc           The location of the unit with this weapon.
 * @param[in]  attacking     Whether or not the unit with this weapon is the attacker.
 */
attack_type::specials_context_t::specials_context_t(const attack_type& weapon, unit_const_ptr self, const map_location& loc, bool attacking)
	: parent(weapon.shared_from_this())
{
	weapon.self_ = self;
	weapon.other_ = unit_ptr();
	weapon.self_loc_ = loc;
	weapon.other_loc_ = map_location::null_location();
	weapon.is_attacker_ = attacking;
	weapon.other_attack_ = nullptr;
	weapon.is_for_listing_ = false;
}

/**
 * Sets the context under which specials will be checked for being active.
 * This version is appropriate for theoretical units of a particular type.
 * @param[in]  weapon        The weapon being considered.
 * @param[in]  self_type     A reference to the type of the unit with this weapon.
 * @param[in]  loc           The location of the unit with this weapon.
 * @param[in]  attacking     Whether or not the unit with this weapon is the attacker.
 */
attack_type::specials_context_t::specials_context_t(const attack_type& weapon, const unit_type& self_type, const map_location& loc, bool attacking)
	: parent(weapon.shared_from_this())
{
	UNUSED(self_type);
	weapon.self_ = unit_ptr();
	weapon.other_ = unit_ptr();
	weapon.self_loc_ = loc;
	weapon.other_loc_ = map_location::null_location();
	weapon.is_attacker_ = attacking;
	weapon.other_attack_ = nullptr;
	weapon.is_for_listing_ = false;
}

attack_type::specials_context_t::specials_context_t(const attack_type& weapon, bool attacking)
	: parent(weapon.shared_from_this())
{
	weapon.is_for_listing_ = true;
	weapon.is_attacker_ = attacking;
}

attack_type::specials_context_t::~specials_context_t()
{
	if(was_moved) return;
	parent->self_ = unit_ptr();
	parent->other_ = unit_ptr();
	parent->self_loc_ = map_location::null_location();
	parent->other_loc_ = map_location::null_location();
	parent->is_attacker_ = false;
	parent->other_attack_ = nullptr;
	parent->is_for_listing_ = false;
}

attack_type::specials_context_t::specials_context_t(attack_type::specials_context_t&& other)
	: parent(other.parent)
{
	other.was_moved = true;
}

/**
 * Calculates the number of attacks this weapon has, considering specials.
 * This returns two numbers because of the swarm special. The actual number of
 * attacks depends on the unit's health and should be:
 *   min_attacks + (max_attacks - min_attacks) * (current hp) / (max hp)
 * c.f. swarm_blows()
 */
void attack_type::modified_attacks(unsigned & min_attacks,
                                   unsigned & max_attacks) const
{
	// Apply [attacks].
	int attacks_value = composite_value(get_specials_and_abilities("attacks"), num_attacks());

	if ( attacks_value < 0 ) {
		attacks_value = num_attacks();
		ERR_NG << "negative number of strikes after applying weapon specials";
	}

	// Apply [swarm].
	active_ability_list swarm_specials = get_specials_and_abilities("swarm");
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
int attack_type::modified_damage() const
{
	int damage_value = composite_value(get_specials_and_abilities("damage"), damage());
	return damage_value;
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
	 * @param[in]  u           A unit to filter.
	 * @param[in]  u2          Another unit to filter.
	 * @param[in]  loc         The presumed location of @a un_it.
	 * @param[in]  weapon      The attack_type to filter.
	 * @param[in]  filter      The filter containing the child filter to use.
	 * @param[in]  for_listing
	 * @param[in]  child_tag   The tag of the child filter to use.
	 */
	static bool special_unit_matches(unit_const_ptr & u,
		                             unit_const_ptr & u2,
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

		auto filter_child = filter.optional_child(child_tag);
		if ( !filter_child )
			// The special does not filter on this unit, so we pass.
			return true;

		// If the primary unit doesn't exist, there's nothing to match
		if (!u) {
			return false;
		}

		unit_filter ufilt{vconfig(*filter_child)};

		// If the other unit doesn't exist, try matching without it


		// Check for a weapon match.
		if (auto filter_weapon = filter_child->optional_child("filter_weapon") ) {
			if ( !weapon || !weapon->matches_filter(*filter_weapon) )
				return false;
		}

		// Passed.
		// If the other unit doesn't exist, try matching without it
		if (!u2) {
			return ufilt.matches(*u, loc);
		}
		return ufilt.matches(*u, loc, *u2);
	}

}//anonymous namespace


//The following functions are intended to allow the use in combat of capacities
//identical to special weapons and therefore to be able to use them on adjacent
//units (abilities of type 'aura') or else on all types of weapons even if the
//beneficiary unit does not have a corresponding weapon
//(defense against ranged weapons abilities for a unit that only has melee attacks)

active_ability_list attack_type::get_weapon_ability(const std::string& ability) const
{
	const map_location loc = self_ ? self_->get_location() : self_loc_;
	active_ability_list abil_list(loc);
	if(self_) {
		abil_list.append_if((*self_).get_abilities(ability, self_loc_), [&](const active_ability& i) {
			return special_active(i.ability_cfg(), AFFECT_SELF, ability, "filter_student");
		});
	}

	if(other_) {
		abil_list.append_if((*other_).get_abilities(ability, other_loc_), [&](const active_ability& i) {
			return special_active_impl(other_attack_, shared_from_this(), i.ability_cfg(), AFFECT_OTHER, ability, "filter_student");
		});
	}

	return abil_list;
}

active_ability_list attack_type::get_specials_and_abilities(const std::string& special) const
{
	active_ability_list abil_list = get_weapon_ability(special);
	if(!abil_list.empty()){
		abil_list = overwrite_special_checking(special, abil_list, abil_list, "filter_student", false);
	}
	active_ability_list spe_list = get_specials(special);
	if(!spe_list.empty()){
		spe_list = overwrite_special_checking(special, spe_list, abil_list, "filter_self", true);
		if(special == "plague" && !spe_list.empty()){
			return spe_list;
		}
		abil_list.append(spe_list);
	}
	return abil_list;
}

int attack_type::composite_value(const active_ability_list& abil_list, int base_value) const
{
	return unit_abilities::effect(abil_list, base_value, shared_from_this()).get_composite_value();
}

static bool overwrite_special_affects(const config& special)
{
	const std::string& apply_to = special["overwrite_specials"];
	return (apply_to == "one_side" || apply_to == "both_sides");
}

active_ability_list attack_type::overwrite_special_checking(const std::string& ability, active_ability_list input, active_ability_list overwriters, const std::string& filter_self, bool is_special) const
{
	for(active_ability_list::iterator i = overwriters.begin(); i != overwriters.end();) {
		if(!overwrite_special_affects(i->ability_cfg())) {
			i = overwriters.erase(i);
		} else {
			++i;
		}
	}
	if(overwriters.empty()){
		return input;
	}

	for(const auto& i : overwriters) {
		bool affect_side = (i.ability_cfg()["overwrite_specials"] == "one_side");
		utils::erase_if(input, [&](const active_ability& j) {
			bool is_overwritable = (is_special || !overwrite_special_affects(j.ability_cfg()));
			bool one_side_overwritable = true;
			if(affect_side && is_overwritable){
				if(special_active_impl(shared_from_this(), other_attack_, i.ability_cfg(), AFFECT_SELF, ability, filter_self)){
					one_side_overwritable = special_active_impl(shared_from_this(), other_attack_, j.ability_cfg(), AFFECT_SELF, ability, filter_self);
				}
				else if(special_active_impl(other_attack_, shared_from_this(), i.ability_cfg(), AFFECT_OTHER, ability, filter_self)){
					one_side_overwritable = special_active_impl(other_attack_, shared_from_this(), j.ability_cfg(), AFFECT_OTHER, ability, filter_self);
				}
			}
			return (is_overwritable && one_side_overwritable);
		});
	}
	return input;
}



bool unit::get_self_ability_bool(const config& special, const std::string& tag_name, const map_location& loc) const
{
	return (ability_active(tag_name, special, loc) && ability_affects_self(tag_name, special, loc));
}

bool unit::get_adj_ability_bool(const config& special, const std::string& tag_name, int dir, const map_location& loc, const unit& from) const
{
	const auto adjacent = get_adjacent_tiles(loc);
	return (affects_side(special, side(), from.side()) && from.ability_active(tag_name, special, adjacent[dir]) && ability_affects_adjacent(tag_name,  special, dir, loc, from));
}

bool unit::get_self_ability_bool_weapon(const config& special, const std::string& tag_name, const map_location& loc, const_attack_ptr weapon, const_attack_ptr opp_weapon) const
{
	return (get_self_ability_bool(special, tag_name, loc) && ability_affects_weapon(special, weapon, false) && ability_affects_weapon(special, opp_weapon, true));
}

bool unit::get_adj_ability_bool_weapon(const config& special, const std::string& tag_name, int dir, const map_location& loc, const unit& from, const_attack_ptr weapon, const_attack_ptr opp_weapon) const
{
	return (get_adj_ability_bool(special, tag_name, dir, loc, from) && ability_affects_weapon(special, weapon, false) && ability_affects_weapon(special, opp_weapon, true));
}

bool attack_type::check_self_abilities(const config& cfg, const std::string& special) const
{
	return check_self_abilities_impl(shared_from_this(), other_attack_, cfg, self_, self_loc_, AFFECT_SELF, special, true);
}

bool attack_type::check_self_abilities_impl(const_attack_ptr self_attack, const_attack_ptr other_attack, const config& special, unit_const_ptr u, const map_location& loc, AFFECTS whom, const std::string& tag_name, bool leader_bool)
{
	if(tag_name == "leadership" && leader_bool){
		if((*u).get_self_ability_bool_weapon(special, tag_name, loc, self_attack, other_attack)) {
			return true;
		}
	}
	if((*u).checking_tags().count(tag_name) != 0){
		if((*u).get_self_ability_bool(special, tag_name, loc) && special_active_impl(self_attack, other_attack, special, whom, tag_name, "filter_student")) {
			return true;
		}
	}
	return false;
}

bool attack_type::check_adj_abilities(const config& cfg, const std::string& special, int dir, const unit& from) const
{
	return check_adj_abilities_impl(shared_from_this(), other_attack_, cfg, self_, from, dir, self_loc_, AFFECT_SELF, special, true);
}

bool attack_type::check_adj_abilities_impl(const_attack_ptr self_attack, const_attack_ptr other_attack, const config& special, unit_const_ptr u, const unit& from, int dir, const map_location& loc, AFFECTS whom, const std::string& tag_name, bool leader_bool)
{
	if(tag_name == "leadership" && leader_bool){
		if((*u).get_adj_ability_bool_weapon(special, tag_name, dir, loc, from, self_attack, other_attack)) {
			return true;
		}
	}
	if((*u).checking_tags().count(tag_name) != 0){
		if((*u).get_adj_ability_bool(special, tag_name, dir, loc, from) && special_active_impl(self_attack, other_attack, special, whom, tag_name, "filter_student")) {
			return true;
		}
	}
	return false;
}
/**
 * Returns whether or not @a *this has a special ability with a tag or id equal to
 * @a special. the Check is for a special ability
 * active in the current context (see set_specials_context), including
 * specials obtained from the opponent's attack.
 */
bool attack_type::has_weapon_ability(const std::string& special, bool special_id, bool special_tags) const
{
	const unit_map& units = get_unit_map();
	if(self_){
		ability_vector special_matches_self;
		get_ability_children(special_matches_self, self_->abilities(), special, special_id , special_tags);
		for(const ability_ptr& entry : special_matches_self) {
			if(check_self_abilities(entry->cfg(), entry->tag())){
				return true;
			}
		}

		const auto adjacent = get_adjacent_tiles(self_loc_);
		for(unsigned i = 0; i < adjacent.size(); ++i) {
			const unit_map::const_iterator it = units.find(adjacent[i]);
			if (it == units.end() || it->incapacitated())
				continue;
			if ( &*it == self_.get() )
				continue;

			ability_vector special_matches_adj;
			get_ability_children(special_matches_adj, it->abilities(), special, special_id , special_tags);
			for(const ability_ptr& entry : special_matches_adj) {
				if(check_adj_abilities(entry->cfg(), entry->tag(), i , *it)){
					return true;
				}
			}
		}
	}

	if(other_){
		ability_vector special_matches_other;
		get_ability_children(special_matches_other, other_->abilities(), special, special_id , special_tags);
		for(const ability_ptr& entry : special_matches_other) {
			if(check_self_abilities_impl(other_attack_, shared_from_this(), entry->cfg(), other_, other_loc_, AFFECT_OTHER, entry->tag())){
				return true;
			}
		}

		const auto adjacent = get_adjacent_tiles(other_loc_);
		for(unsigned i = 0; i < adjacent.size(); ++i) {
			const unit_map::const_iterator it = units.find(adjacent[i]);
			if (it == units.end() || it->incapacitated())
				continue;
			if ( &*it == other_.get() )
				continue;

			ability_vector special_matches_oadj;
			get_ability_children(special_matches_oadj, it->abilities(), special, special_id , special_tags);
			for(const ability_ptr& entry : special_matches_oadj) {
				if(check_adj_abilities_impl(other_attack_, shared_from_this(), entry->cfg(), other_, *it, i, other_loc_, AFFECT_OTHER, entry->tag())){
					return true;
				}
			}
		}
	}
	return false;
}

bool attack_type::has_special_or_ability(const std::string& special, bool special_id, bool special_tags) const
{
	//Now that filter_(second)attack in event supports special_id/type_active, including abilities used as weapons,
	//these can be detected even in placeholder attacks generated to compensate for the lack of attack in defense against an attacker using a range attack not possessed by the defender.
	//It is therefore necessary to check if the range is not empty (proof that the weapon is not a placeholder) to decide if has_weapon_ability can be returned or not.
	if(range().empty()){
		return false;
	}
	return (has_special(special, special_id, special_tags) || has_weapon_ability(special, special_id, special_tags));
}
//end of emulate weapon special functions.

bool attack_type::special_active(const config& special, AFFECTS whom, const std::string& tag_name,
                                 const std::string& filter_self) const
{
	return special_active_impl(shared_from_this(), other_attack_, special, whom, tag_name, filter_self);
}

/**
 * Returns whether or not the given special is active for the specified unit,
 * based on the current context (see set_specials_context).
 * @param self_attack       this unit's attack
 * @param other_attack      the other unit's attack
 * @param special           a weapon special WML structure
 * @param whom              specifies which combatant we care about
 * @param tag_name          tag name of the special config
 *  @param filter_self      the filter to use
 */
bool attack_type::special_active_impl(
	const_attack_ptr self_attack,
	const_attack_ptr other_attack,
	const config& special,
	AFFECTS whom,
	const std::string& tag_name,
	const std::string& filter_self)
{
	assert(self_attack || other_attack);
	bool is_attacker = self_attack ? self_attack->is_attacker_ : !other_attack->is_attacker_;
	bool is_for_listing = self_attack ? self_attack->is_for_listing_ : other_attack->is_for_listing_;
	//log_scope("special_active");


	// Does this affect the specified unit?
	if ( whom == AFFECT_SELF ) {
		if ( !special_affects_self(special, is_attacker) )
			return false;
	}
	if ( whom == AFFECT_OTHER ) {
		if ( !special_affects_opponent(special, is_attacker) )
			return false;
	}

	// Is this active on attack/defense?
	const std::string & active_on = special["active_on"];
	if ( !active_on.empty() ) {
		if ( is_attacker  &&  active_on != "offense" )
			return false;
		if ( !is_attacker  &&  active_on != "defense" )
			return false;
	}

	// Get the units involved.
	const unit_map& units = get_unit_map();

	unit_const_ptr self = self_attack ? self_attack->self_ : other_attack->other_;
	unit_const_ptr other = self_attack ? self_attack->other_ : other_attack->self_;
	map_location self_loc = self_attack ? self_attack->self_loc_ : other_attack->other_loc_;
	map_location other_loc = self_attack ? self_attack->other_loc_ : other_attack->self_loc_;
	//TODO: why is this needed?
	if(self == nullptr) {
		unit_map::const_iterator it = units.find(self_loc);
		if(it.valid()) {
			self = it.get_shared_ptr();
		}
	}
	if(other == nullptr) {
		unit_map::const_iterator it = units.find(other_loc);
		if(it.valid()) {
			other = it.get_shared_ptr();
		}
	}

	// Make sure they're facing each other.
	temporary_facing self_facing(self, self_loc.get_relative_dir(other_loc));
	temporary_facing other_facing(other, other_loc.get_relative_dir(self_loc));

	// Filter poison, plague, drain, slow, petrifies
	// True if "whom" corresponds to "self", false if "whom" is "other"
	bool whom_is_self = ((whom == AFFECT_SELF) || ((whom == AFFECT_EITHER) && special_affects_self(special, is_attacker)));
	unit_const_ptr them = whom_is_self ? other : self;
	map_location their_loc = whom_is_self ? other_loc : self_loc;

	if (tag_name == "drains" && them && them->get_state("undrainable")) {
		return false;
	}
	if (tag_name == "plague" && them &&
		(them->get_state("unplagueable") ||
		 resources::gameboard->map().is_village(their_loc))) {
		return false;
	}
	if (tag_name == "poison" && them &&
		(them->get_state("unpoisonable") || them->get_state(unit::STATE_POISONED))) {
		return false;
	}
	if (tag_name == "slow" && them &&
		(them->get_state("unslowable") || them->get_state(unit::STATE_SLOWED))) {
		return false;
	}
	if (tag_name == "petrifies" && them &&
		them->get_state("unpetrifiable")) {
		return false;
	}


	// Translate our context into terms of "attacker" and "defender".
	unit_const_ptr & att = is_attacker ? self : other;
	unit_const_ptr & def = is_attacker ? other : self;
	const map_location & att_loc   = is_attacker ? self_loc : other_loc;
	const map_location & def_loc   = is_attacker ? other_loc : self_loc;
	const_attack_ptr att_weapon = is_attacker ? self_attack : other_attack;
	const_attack_ptr def_weapon = is_attacker ? other_attack : self_attack;

	// Filter firststrike here, if both units have first strike then the effects cancel out. Only check
	// the opponent if "whom" is the defender, otherwise this leads to infinite recursion.
	if (tag_name == "firststrike") {
		bool whom_is_defender = whom_is_self ? !is_attacker : is_attacker;
		if (whom_is_defender && att_weapon && att_weapon->has_special_or_ability("firststrike"))
			return false;
	}

	// Filter the units involved.
	if (!special_unit_matches(self, other, self_loc, self_attack, special, is_for_listing, filter_self))
		return false;
	if (!special_unit_matches(other, self, other_loc, other_attack, special, is_for_listing, "filter_opponent"))
		return false;
	if (!special_unit_matches(att, def, att_loc, att_weapon, special, is_for_listing, "filter_attacker"))
		return false;
	if (!special_unit_matches(def, att, def_loc, def_weapon, special, is_for_listing, "filter_defender"))
		return false;

	const auto adjacent = get_adjacent_tiles(self_loc);

	// Filter the adjacent units.
	for (const config &i : special.child_range("filter_adjacent"))
	{
		std::size_t count = 0;
		std::vector<map_location::DIRECTION> dirs = map_location::parse_directions(i["adjacent"]);
		unit_filter filter{ vconfig(i) };
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
		std::size_t count = 0;
		std::vector<map_location::DIRECTION> dirs = map_location::parse_directions(i["adjacent"]);
		terrain_filter adj_filter(vconfig(i), resources::filter_con, false);
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

