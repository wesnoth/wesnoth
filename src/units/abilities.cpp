/*
	Copyright (C) 2006 - 2022
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
#include "gettext.hpp"
#include "global.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/abilities.hpp"
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
 */


namespace {

bool affects_side(const config& cfg, std::size_t side, std::size_t other_side)
{
	// display::get_singleton() has already been confirmed valid by both callers.
	const team& side_team = display::get_singleton()->get_disp_context().get_team(side);

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
	for (const config &i : this->abilities_.child_range(tag_name)) {
		if (ability_active(tag_name, i, loc) &&
			ability_affects_self(tag_name, i, loc))
		{
			return true;
		}
	}

	assert(display::get_singleton());
	const unit_map& units = display::get_singleton()->get_units();

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
		for (const config &j : it->abilities_.child_range(tag_name)) {
			if (affects_side(j, side(), it->side()) &&
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
	unit_ability_list res(loc_);

	for(const config& i : this->abilities_.child_range(tag_name)) {
		if(ability_active(tag_name, i, loc)
			&& ability_affects_self(tag_name, i, loc))
			{
			res.emplace_back(&i, loc, loc);
		}
	}

	assert(display::get_singleton());
	const unit_map& units = display::get_singleton()->get_units();

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
		for(const config& j : it->abilities_.child_range(tag_name)) {
			if(affects_side(j, side(), it->side())
				&& it->ability_active(tag_name, j, adjacent[i])
				&& ability_affects_adjacent(tag_name, j, i, loc, *it))
				{
				res.emplace_back(&j, loc, adjacent[i]);
			}
		}
	}


	return res;
}

unit_ability_list unit::get_abilities_weapons(const std::string& tag_name, const map_location& loc, const_attack_ptr weapon, const_attack_ptr opp_weapon) const
{
	unit_ability_list res = get_abilities(tag_name, loc);
	for(unit_ability_list::iterator i = res.begin(); i != res.end();) {
		if((!ability_affects_weapon(*i->ability_cfg, weapon, false) || !ability_affects_weapon(*i->ability_cfg, opp_weapon, true))) {
			i = res.erase(i);
		} else {
			++i;
		}
	}
	return res;
}

std::vector<std::string> unit::get_ability_list() const
{
	std::vector<std::string> res;

	for (const config::any_child ab : this->abilities_.all_children_range()) {
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

	/**
	 * Adds a quadruple consisting of (in order) id, base name,
	 * male or female name as appropriate for the unit, and description.
	 *
	 * @returns Whether name was resolved and quadruple added.
	 */
	bool add_ability_tooltip(const config::any_child &ab, unit_race::GENDER gender, std::vector<std::tuple<std::string, t_string,t_string,t_string>>& res, bool active)
	{
		if (active) {
			const t_string& name = gender_value(ab.cfg, gender, "name", "female_name", "name").t_str();

			if (!name.empty()) {
				res.emplace_back(
						ab.cfg["id"],
						ab.cfg["name"].t_str(),
						name,
						ab.cfg["description"].t_str() );
				return true;
			}
		}
		else
		{
			// See if an inactive name was specified.
			const config::attribute_value& inactive_value =
				gender_value(ab.cfg, gender, "name_inactive",
						"female_name_inactive", "name_inactive");
			const t_string& name = !inactive_value.blank() ? inactive_value.t_str() :
				gender_value(ab.cfg, gender, "name", "female_name", "name").t_str();

			if (!name.empty()) {
				res.emplace_back(
						ab.cfg["id"],
						default_value(ab.cfg, "name_inactive", "name").t_str(),
						name,
						default_value(ab.cfg, "description_inactive", "description").t_str() );
				return true;
			}
		}

		return false;
	}
}

std::vector<std::tuple<std::string, t_string, t_string, t_string>> unit::ability_tooltips() const
{
	std::vector<std::tuple<std::string, t_string,t_string,t_string>> res;

	for (const config::any_child ab : this->abilities_.all_children_range())
	{
		add_ability_tooltip(ab, gender_, res, true);
	}

	return res;
}

std::vector<std::tuple<std::string, t_string, t_string, t_string>> unit::ability_tooltips(boost::dynamic_bitset<>& active_list, const map_location& loc) const
{
	std::vector<std::tuple<std::string, t_string,t_string,t_string>> res;
	active_list.clear();

	for (const config::any_child ab : this->abilities_.all_children_range())
	{
		bool active = ability_active(ab.key, ab.cfg, loc);
		if (add_ability_tooltip(ab, gender_, res, active))
		{
			active_list.push_back(active);
		}
	}
	return res;
}

bool unit::ability_active(const std::string& ability,const config& cfg,const map_location& loc) const
{
	bool illuminates = ability == "illuminates";

	if (const config &afilter = cfg.child("filter"))
		if ( !unit_filter(vconfig(afilter)).set_use_flat_tod(illuminates).matches(*this, loc) )
			return false;

	const auto adjacent = get_adjacent_tiles(loc);

	assert(display::get_singleton());
	const unit_map& units = display::get_singleton()->get_units();

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
		const config &filter = i.child("filter");
		if (!filter || //filter tag given
			unit_filter(vconfig(filter)).set_use_flat_tod(illuminates).matches(*this, loc, from) ) {
			return true;
		}
	}
	return false;
}

bool unit::ability_affects_self(const std::string& ability,const config& cfg,const map_location& loc) const
{
	const config &filter = cfg.child("filter_self");
	bool affect_self = cfg["affect_self"].to_bool(true);
	if (!filter || !affect_self) return affect_self;
	return unit_filter(vconfig(filter)).set_use_flat_tod(ability == "illuminates").matches(*this, loc);
}

bool unit::ability_affects_weapon(const config& cfg, const_attack_ptr weapon, bool is_opp) const
{
	const std::string filter_tag_name = is_opp ? "filter_second_weapon" : "filter_weapon";
	if(!cfg.has_child(filter_tag_name)) {
		return true;
	}
	const config& filter = cfg.child(filter_tag_name);
	if(!weapon) {
		return false;
	}
	return weapon->matches_filter(filter);
}

bool unit::has_ability_type(const std::string& ability) const
{
	return !abilities_.child_range(ability).empty();
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

namespace {


template<typename T, typename TFuncFormula>
class get_ability_value_visitor
#ifdef USING_BOOST_VARIANT
	: public boost::static_visitor<T>
#endif
{
public:
	// Constructor stores the default value.
	get_ability_value_visitor(T def, const TFuncFormula& formula_handler) : def_(def), formula_handler_(formula_handler) {}

	T operator()(const utils::monostate&) const { return def_; }
	T operator()(bool)                 const { return def_; }
	T operator()(int i)                const { return static_cast<T>(i); }
	T operator()(unsigned long long u) const { return static_cast<T>(u); }
	T operator()(double d)             const { return static_cast<T>(d); }
	T operator()(const t_string&)     const { return def_; }
	T operator()(const std::string& s) const
	{
		if(s.size() >= 2 && s[0] == '(') {
			return formula_handler_(s);
		}
		return lexical_cast_default<T>(s, def_);
	}

private:
	const T def_;
	const TFuncFormula& formula_handler_;
};

template<typename T, typename TFuncFormula>
T get_single_ability_value(const config::attribute_value& v, T def, const unit_ability& ability_info, const map_location& receiver_loc, const_attack_ptr att, const TFuncFormula& formula_handler)
{
	return v.apply_visitor(get_ability_value_visitor(def, [&](const std::string& s) {

			try {
				assert(display::get_singleton());
				const unit_map& units = display::get_singleton()->get_units();

				auto u_itor = units.find(ability_info.teacher_loc);

				if(u_itor == units.end()) {
					return def;
				}
				wfl::map_formula_callable callable(std::make_shared<wfl::unit_callable>(*u_itor));
				if(att) {
					att->add_formula_context(callable);
				}
				if (auto uptr = units.find_unit_ptr(ability_info.student_loc)) {
					callable.add("student", wfl::variant(std::make_shared<wfl::unit_callable>(*uptr)));
				}
				if (auto uptr = units.find_unit_ptr(receiver_loc)) {
					callable.add("other", wfl::variant(std::make_shared<wfl::unit_callable>(*uptr)));
				}
				return formula_handler(wfl::formula(s, new wfl::gamestate_function_symbol_table), callable);
			} catch(const wfl::formula_error& e) {
				lg::log_to_chat() << "Formula error in ability or weapon special: " << e.type << " at " << e.filename << ':' << e.line << ")\n";
				ERR_WML << "Formula error in ability or weapon special: " << e.type << " at " << e.filename << ':' << e.line << ")";
				return def;
			}
	}));
}
}

template<typename TComp>
std::pair<int,map_location> unit_ability_list::get_extremum(const std::string& key, int def, const TComp& comp) const
{
	if ( cfgs_.empty() ) {
		return std::pair(def, map_location());
	}
	// The returned location is the best non-cumulative one, if any,
	// the best absolute cumulative one otherwise.
	map_location best_loc;
	bool only_cumulative = true;
	int abs_max = 0;
	int flat = 0;
	int stack = 0;
	for (const unit_ability& p : cfgs_)
	{
		int value = get_single_ability_value((*p.ability_cfg)[key], def, p, loc(), const_attack_ptr(), [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
			return formula.evaluate(callable).as_int();
		});

		if ((*p.ability_cfg)["cumulative"].to_bool()) {
			stack += value;
			if (value < 0) value = -value;
			if (only_cumulative && !comp(value, abs_max)) {
				abs_max = value;
				best_loc = p.teacher_loc;
			}
		} else if (only_cumulative || comp(flat, value)) {
			only_cumulative = false;
			flat = value;
			best_loc = p.teacher_loc;
		}
	}
	return std::pair(flat + stack, best_loc);
}

template std::pair<int, map_location> unit_ability_list::get_extremum<std::less<int>>(const std::string& key, int def, const std::less<int>& comp) const;
template std::pair<int, map_location> unit_ability_list::get_extremum<std::greater<int>>(const std::string& key, int def, const std::greater<int>& comp) const;

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

	struct special_match
	{
		std::string tag_name;
		const config* cfg;
	};

	/**
	 * Gets the children of @parent (which should be the specials for an
	 * attack_type) and places the ones whose tag or id= matches @a id into
	 * @a tag_result and @a id_result.
	 *
	 * If @a just_peeking is set to true, then @a tag_result and @a id_result
	 * are not touched; instead the return value is used to indicate if any
	 * matching children were found.
	 *
	 * @returns  true if @a just_peeking is true and a match was found;
	 *           false otherwise.
	 */
	bool get_special_children(std::vector<special_match>& tag_result,
	                           std::vector<special_match>& id_result,
	                           const config& parent, const std::string& id,
	                           bool just_peeking=false) {
		for (const config::any_child sp : parent.all_children_range())
		{
			if (just_peeking && (sp.key == id || sp.cfg["id"] == id)) {
				return true; // peek succeeded; done
			}

			if(sp.key == id) {
				special_match special = { sp.key, &sp.cfg };
				tag_result.push_back(special);
			}
			if(sp.cfg["id"] == id) {
				special_match special = { sp.key, &sp.cfg };
				id_result.push_back(special);
			}
		}
		return false;
	}

	bool get_special_children_id(std::vector<special_match>& id_result,
	                           const config& parent, const std::string& id,
	                           bool just_peeking=false) {
		for (const config::any_child sp : parent.all_children_range())
		{
			if (just_peeking && (sp.cfg["id"] == id)) {
				return true; // peek succeeded; done
			}

			if(sp.cfg["id"] == id) {
				special_match special = { sp.key, &sp.cfg };
				id_result.push_back(special);
			}
		}
		return false;
	}

	bool get_special_children_tags(std::vector<special_match>& tag_result,
	                           const config& parent, const std::string& id,
	                           bool just_peeking=false) {
		for (const config::any_child sp : parent.all_children_range())
		{
			if (just_peeking && (sp.key == id)) {
				return true; // peek succeeded; done
			}

			if(sp.key == id) {
				special_match special = { sp.key, &sp.cfg };
				tag_result.push_back(special);
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
bool attack_type::has_special(const std::string& special, bool simple_check, bool special_id, bool special_tags) const
{
	{
		std::vector<special_match> special_tag_matches;
		std::vector<special_match> special_id_matches;
		if(special_id && special_tags){
			if ( get_special_children(special_tag_matches, special_id_matches, specials_, special, simple_check) ) {
				return true;
			}
		} else if(special_id && !special_tags){
			if ( get_special_children_id(special_id_matches, specials_, special, simple_check) ) {
				return true;
			}
		} else if(!special_id && special_tags){
			if ( get_special_children_tags(special_tag_matches, specials_, special, simple_check) ) {
				return true;
			}
		}
		// If we make it to here, then either list.empty() or !simple_check.
		// So if the list is not empty, then this is not a simple check and
		// we need to check each special in the list to see if any are active.
		if(special_tags){
			for(const special_match& entry : special_tag_matches) {
				if ( special_active(*entry.cfg, AFFECT_SELF, entry.tag_name) ) {
					return true;
				}
			}
		}
		if(special_id){
			for(const special_match& entry : special_id_matches) {
				if ( special_active(*entry.cfg, AFFECT_SELF, entry.tag_name) ) {
					return true;
				}
			}
		}
	}

	// Skip checking the opponent's attack?
	if ( simple_check || !other_attack_ ) {
		return false;
	}

	std::vector<special_match> special_tag_matches;
	std::vector<special_match> special_id_matches;
	if(special_id && special_tags){
		get_special_children(special_tag_matches, special_id_matches, other_attack_->specials_, special);
	} else if(special_id && !special_tags){
		get_special_children_id(special_id_matches, other_attack_->specials_, special);
	} else if(!special_id && special_tags){
		get_special_children_tags(special_tag_matches, other_attack_->specials_, special);
	}
	if(special_tags){
		for(const special_match& entry : special_tag_matches) {
			if ( other_attack_->special_active(*entry.cfg, AFFECT_OTHER, entry.tag_name) ) {
				return true;
			}
		}
	}
	if(special_id){
		for(const special_match& entry : special_id_matches) {
			if ( other_attack_->special_active(*entry.cfg, AFFECT_OTHER, entry.tag_name) ) {
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
unit_ability_list attack_type::get_specials(const std::string& special) const
{
	//log_scope("get_specials");
	const map_location loc = self_ ? self_->get_location() : self_loc_;
	unit_ability_list res(loc);

	for(const config& i : specials_.child_range(special)) {
		if(special_active(i, AFFECT_SELF, special)) {
			res.emplace_back(&i, loc, loc);
		}
	}

	if(!other_attack_) {
		return res;
	}

	for(const config& i : other_attack_->specials_.child_range(special)) {
		if(other_attack_->special_active(i, AFFECT_OTHER, special)) {
			res.emplace_back(&i, other_loc_, other_loc_);
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

	for (const config::any_child sp : specials_.all_children_range())
	{
		if ( !active_list || special_active(sp.cfg, AFFECT_EITHER, sp.key) ) {
			const t_string &name = sp.cfg["name"];
			if (!name.empty()) {
				res.emplace_back(name, sp.cfg["description"].t_str() );
				if ( active_list )
					active_list->push_back(true);
			}
		} else {
			const t_string& name = default_value(sp.cfg, "name_inactive", "name").t_str();
			if (!name.empty()) {
				res.emplace_back(name, default_value(sp.cfg, "description_inactive", "description").t_str() );
				active_list->push_back(false);
			}
		}
	}
	return res;
}

/**
 * static used in weapon_specials (bool only_active, bool is_backstab) and
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
 * and the @a is_backstab parameter.
 */
std::string attack_type::weapon_specials(bool is_backstab) const
{
	//log_scope("weapon_specials");
	std::string res;
	for (const config::any_child sp : specials_.all_children_range())
	{
		const bool active = special_active(sp.cfg, AFFECT_EITHER, sp.key, is_backstab);

		const std::string& name =
			active
			? sp.cfg["name"].str()
			: default_value(sp.cfg, "name_inactive", "name").str();
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
	for (const config::any_child sp : specials_.all_children_range()) {
		if((checking_tags.count(sp.key) != 0)){
			const bool active = special_active(sp.cfg, AFFECT_SELF, sp.key);
			add_name(temp_string, active, sp.cfg["name"].str(), checking_name);
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
		for (const config::any_child sp : other_attack_->specials_.all_children_range()) {
			if((checking_tags.count(sp.key) != 0)){
				const bool active = other_attack_->special_active(sp.cfg, AFFECT_OTHER, sp.key);
				add_name(temp_string, active, sp.cfg["name"].str(), checking_name);
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
		for (const config::any_child sp : self->abilities().all_children_range()){
			bool tag_checked = (!checking_tags.empty()) ? (checking_tags.count(sp.key) != 0) : true;
			const bool active = tag_checked && check_self_abilities_impl(self_attack, other_attack, sp.cfg, self, self_loc, whom, sp.key, leader_bool);
			add_name(temp_string, active, sp.cfg["name"].str(), checking_name);
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
	assert(display::get_singleton());
	const unit_map& units = display::get_singleton()->get_units();
	if(self){
		const auto adjacent = get_adjacent_tiles(self_loc);
		for(unsigned i = 0; i < adjacent.size(); ++i) {
			const unit_map::const_iterator it = units.find(adjacent[i]);
			if (it == units.end() || it->incapacitated())
				continue;
			if(&*it == self.get())
				continue;
			for (const config::any_child sp : it->abilities().all_children_range()){
				bool tag_checked = (!checking_tags.empty()) ? (checking_tags.count(sp.key) != 0) : true;
				bool default_bool = (affect_adjacents == "affect_allies") ? true : false;
				bool affect_allies = (!affect_adjacents.empty()) ? sp.cfg[affect_adjacents].to_bool(default_bool) : true;
				const bool active = tag_checked && check_adj_abilities_impl(self_attack, other_attack, sp.cfg, self, *it, i, self_loc, whom, sp.key, leader_bool) && affect_allies;
				add_name(temp_string, active, sp.cfg["name"].str(), checking_name);
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
void attack_type::modified_attacks(bool is_backstab, unsigned & min_attacks,
                                   unsigned & max_attacks) const
{
	// Apply [attacks].
	unit_abilities::effect attacks_effect(get_specials_and_abilities("attacks"),
	                                      num_attacks(), is_backstab, shared_from_this());
	int attacks_value = attacks_effect.get_composite_value();

	if ( attacks_value < 0 ) {
		attacks_value = num_attacks();
		ERR_NG << "negative number of strikes after applying weapon specials";
	}

	// Apply [swarm].
	unit_ability_list swarm_specials = get_specials_and_abilities("swarm");
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
	unit_abilities::effect dmg_effect(get_specials_and_abilities("damage"), damage(), is_backstab, shared_from_this());
	int damage_value = dmg_effect.get_composite_value();
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

		const config & filter_child = filter.child(child_tag);
		if ( !filter_child )
			// The special does not filter on this unit, so we pass.
			return true;

		// If the primary unit doesn't exist, there's nothing to match
		if (!u) {
			return false;
		}

		unit_filter ufilt{vconfig(filter_child)};

		// If the other unit doesn't exist, try matching without it


		// Check for a weapon match.
		if ( const config & filter_weapon = filter_child.child("filter_weapon") ) {
			if ( !weapon || !weapon->matches_filter(filter_weapon) )
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

unit_ability_list attack_type::get_weapon_ability(const std::string& ability) const
{
	const map_location loc = self_ ? self_->get_location() : self_loc_;
	unit_ability_list abil_list(loc);
	unit_ability_list abil_other_list(loc);
	if(self_) {
		abil_list.append((*self_).get_abilities(ability, self_loc_));
		for(unit_ability_list::iterator i = abil_list.begin(); i != abil_list.end();) {
			if(!special_active(*i->ability_cfg, AFFECT_SELF, ability, true, "filter_student")) {
				i = abil_list.erase(i);
			} else {
				++i;
			}
		}
	}

	if(other_) {
		abil_other_list.append((*other_).get_abilities(ability, other_loc_));
		for(unit_ability_list::iterator i = abil_other_list.begin(); i != abil_other_list.end();) {
			if(!special_active_impl(other_attack_, shared_from_this(), *i->ability_cfg, AFFECT_OTHER, ability, true, "filter_student")) {
				i = abil_other_list.erase(i);
			} else {
				++i;
			}
		}
	}

	abil_list.append(abil_other_list);
	if(!abil_list.empty()){
		abil_list = overwrite_special_checking(ability, abil_list, abil_list, "filter_student");
	}
	return abil_list;
}

unit_ability_list attack_type::get_specials_and_abilities(const std::string& special) const
{
	unit_ability_list abil_list = get_weapon_ability(special);
	unit_ability_list spe_list = get_specials(special);
	if(!spe_list.empty()){
		spe_list = overwrite_special_checking(special, spe_list, abil_list);
		if(special == "plague" && !spe_list.empty()){
			return spe_list;
		}
		abil_list.append(spe_list);
	}
	return abil_list;
}

static bool overwrite_special_affects(const config& special)
{
	const std::string& apply_to = special["overwrite_specials"];
	return (apply_to == "one_side" || apply_to == "both_sides");
}

unit_ability_list attack_type::overwrite_special_checking(const std::string& ability, unit_ability_list temp_list, const unit_ability_list& abil_list, const std::string& filter_self) const
{
	bool overwrite_self = false;
	bool overwrite_either = false;

	for(const auto& i : abil_list) {
		if((*i.ability_cfg)["overwrite_specials"] == "both_sides") {
			overwrite_either = true;
			break;
		}
		if(overwrite_special_affects(*i.ability_cfg) && (special_active(*i.ability_cfg, AFFECT_SELF, ability, true, "filter_student") || special_active_impl(other_attack_, shared_from_this(), *i.ability_cfg, AFFECT_OTHER, ability, true, "filter_student"))) {
			overwrite_self = true;
		}
	}
	if(!overwrite_either && !overwrite_self){
		return temp_list;
	} else if(overwrite_either){
		for(unit_ability_list::iterator i = temp_list.begin(); i != temp_list.end();) {
			bool overwrite = false;
			if(filter_self == "filter_student"){
				overwrite = overwrite_special_affects(*i->ability_cfg);
			}
			if(!overwrite && (special_active(*i->ability_cfg, AFFECT_SELF, ability, true, filter_self) || special_active_impl(other_attack_, shared_from_this(), *i->ability_cfg, AFFECT_OTHER, ability, true, filter_self))) {
				i = temp_list.erase(i);
			} else {
				++i;
			}
		}
	} else if(overwrite_self){
		for(unit_ability_list::iterator i = temp_list.begin(); i != temp_list.end();) {
			bool overwrite = false;
			if(filter_self == "filter_student"){
				overwrite = overwrite_special_affects(*i->ability_cfg);
			}
			if(!overwrite && special_active(*i->ability_cfg, AFFECT_SELF, ability, true, filter_self)) {
				i = temp_list.erase(i);
			} else {
				++i;
			}
		}
	}
	return temp_list;
}

	/**
	 * Gets the children of parent (which should be the abilities for an
	 * attack_type) and places the ones whose tag or id= matches @a id into
	 * @a tag_result and @a id_result.
	 * @param tag_result receive the children whose tag matches @a id
	 * @param id_result receive the children whose id matches @a id
	 * @param parent the tags whose contain children (abilities here)
	 * @param id tag or id of child tested
	 * @param special_id if true, children check by id
	 * @param special_tags if true, children check by tags
	 */
static void get_ability_children(std::vector<special_match>& tag_result,
	                           std::vector<special_match>& id_result,
	                           const config& parent, const std::string& id,
	                           bool special_id=true, bool special_tags=true) {
	if(special_id && special_tags){
		get_special_children(tag_result, id_result, parent, id);
	} else if(special_id && !special_tags){
		get_special_children_id(id_result, parent, id);
	} else if(!special_id && special_tags){
		get_special_children_tags(tag_result, parent, id);
	}
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
		if((*u).get_self_ability_bool(special, tag_name, loc) && special_active_impl(self_attack, other_attack, special, whom, tag_name, true, "filter_student")) {
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
		if((*u).get_adj_ability_bool(special, tag_name, dir, loc, from) && special_active_impl(self_attack, other_attack, special, whom, tag_name, true, "filter_student")) {
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
	assert(display::get_singleton());
	const unit_map& units = display::get_singleton()->get_units();
	if(self_){
		std::vector<special_match> special_tag_matches_self;
		std::vector<special_match> special_id_matches_self;
		get_ability_children(special_tag_matches_self, special_id_matches_self, (*self_).abilities(), special, special_id , special_tags);
		if(special_tags){
			for(const special_match& entry : special_tag_matches_self) {
				if(check_self_abilities(*entry.cfg, entry.tag_name)){
					return true;
				}
			}
		}
		if(special_id){
			for(const special_match& entry : special_id_matches_self) {
				if(check_self_abilities(*entry.cfg, entry.tag_name)){
					return true;
				}
			}
		}

		const auto adjacent = get_adjacent_tiles(self_loc_);
		for(unsigned i = 0; i < adjacent.size(); ++i) {
			const unit_map::const_iterator it = units.find(adjacent[i]);
			if (it == units.end() || it->incapacitated())
				continue;
			if ( &*it == self_.get() )
				continue;

			std::vector<special_match> special_tag_matches_adj;
			std::vector<special_match> special_id_matches_adj;
			get_ability_children(special_tag_matches_adj, special_id_matches_adj, it->abilities(), special, special_id , special_tags);
			if(special_tags){
				for(const special_match& entry : special_tag_matches_adj) {
					if(check_adj_abilities(*entry.cfg, entry.tag_name, i , *it)){
						return true;
					}
				}
			}
			if(special_id){
				for(const special_match& entry : special_id_matches_adj) {
					if(check_adj_abilities(*entry.cfg, entry.tag_name, i , *it)){
						return true;
					}
				}
			}
		}
	}

	if(other_){
		std::vector<special_match> special_tag_matches_other;
		std::vector<special_match> special_id_matches_other;
		get_ability_children(special_tag_matches_other, special_id_matches_other, (*other_).abilities(), special, special_id , special_tags);
		if(special_tags){
			for(const special_match& entry : special_tag_matches_other) {
				if(check_self_abilities_impl(other_attack_, shared_from_this(), *entry.cfg, other_, other_loc_, AFFECT_OTHER, entry.tag_name)){
					return true;
				}
			}
		}

		if(special_id){
			for(const special_match& entry : special_id_matches_other) {
				if(check_self_abilities_impl(other_attack_, shared_from_this(), *entry.cfg, other_, other_loc_, AFFECT_OTHER, entry.tag_name)){
					return true;
				}
			}
		}

		const auto adjacent = get_adjacent_tiles(other_loc_);
		for(unsigned i = 0; i < adjacent.size(); ++i) {
			const unit_map::const_iterator it = units.find(adjacent[i]);
			if (it == units.end() || it->incapacitated())
				continue;
			if ( &*it == other_.get() )
				continue;

			std::vector<special_match> special_tag_matches_oadj;
			std::vector<special_match> special_id_matches_oadj;
			get_ability_children(special_tag_matches_oadj, special_id_matches_oadj, it->abilities(), special, special_id , special_tags);
			if(special_tags){
				for(const special_match& entry : special_tag_matches_oadj) {
					if(check_adj_abilities_impl(other_attack_, shared_from_this(), *entry.cfg, other_, *it, i, other_loc_, AFFECT_OTHER, entry.tag_name)){
						return true;
					}
				}
			}

			if(special_id){
				for(const special_match& entry : special_id_matches_oadj) {
					if(check_adj_abilities_impl(other_attack_, shared_from_this(), *entry.cfg, other_, *it, i, other_loc_, AFFECT_OTHER, entry.tag_name)){
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool attack_type::has_special_or_ability(const std::string& special, bool special_id, bool special_tags) const
{
	return (has_special(special, false, special_id, special_tags) || has_weapon_ability(special, special_id, special_tags));
}
//end of emulate weapon special functions.

bool attack_type::special_active(const config& special, AFFECTS whom, const std::string& tag_name,
                                 bool include_backstab, const std::string& filter_self) const
{
	return special_active_impl(shared_from_this(), other_attack_, special, whom, tag_name, include_backstab, filter_self);
}

/**
 * Returns whether or not the given special is active for the specified unit,
 * based on the current context (see set_specials_context).
 * @param self_attack       this unit's attack
 * @param other_attack      the other unit's attack
 * @param special           a weapon special WML structure
 * @param whom              specifies which combatant we care about
 * @param tag_name          tag name of the special config
 * @param include_backstab  false if backstab specials should not be active
 *                              (usually true since backstab is usually accounted
 *                              for elsewhere)
 *  @param filter_self      the filter to use
 */
bool attack_type::special_active_impl(
	const_attack_ptr self_attack,
	const_attack_ptr other_attack,
	const config& special,
	AFFECTS whom,
	const std::string& tag_name,
	bool include_backstab,
	const std::string& filter_self)
{
	assert(self_attack || other_attack);
	bool is_attacker = self_attack ? self_attack->is_attacker_ : !other_attack->is_attacker_;
	bool is_for_listing = self_attack ? self_attack->is_for_listing_ : other_attack->is_for_listing_;
	//log_scope("special_active");

	// Backstab check
	if ( !include_backstab )
		if ( special["backstab"].to_bool() )
			return false;

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
	assert(display::get_singleton());
	const unit_map& units = display::get_singleton()->get_units();

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

effect::effect(const unit_ability_list& list, int def, bool backstab, const_attack_ptr att, bool is_cumulable) :
	effect_list_(),
	composite_value_(0)
{

	int value_set = is_cumulable ? std::max(list.highest("value").first, 0) + std::min(list.lowest("value").first, 0) : def;
	std::map<std::string,individual_effect> values_add;
	std::map<std::string,individual_effect> values_mul;
	std::map<std::string,individual_effect> values_div;

	individual_effect set_effect_max;
	individual_effect set_effect_min;

	for (const unit_ability & ability : list) {
		const config& cfg = *ability.ability_cfg;
		const std::string& effect_id = cfg[cfg["id"].empty() ? "name" : "id"];

		if (!cfg["backstab"].blank()) {
			deprecated_message("backstab= in weapon specials", DEP_LEVEL::PREEMPTIVE, {1, 15, 0}, "Use [filter_adjacent] instead.");
		}

		if (!backstab && cfg["backstab"].to_bool())
			continue;
		if (!filter_base_matches(cfg, def))
			continue;

		if(!is_cumulable){
			if (const config::attribute_value *v = cfg.get("value")) {
				int value = get_single_ability_value(*v, def, ability, list.loc(), att, [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
					callable.add("base_value", wfl::variant(def));
					return formula.evaluate(callable).as_int();
				});

				int value_cum = cfg["cumulative"].to_bool() ? std::max(def, value) : value;
				assert((set_effect_min.type != NOT_USED) == (set_effect_max.type != NOT_USED));
				if(set_effect_min.type == NOT_USED) {
					set_effect_min.set(SET, value_cum, ability.ability_cfg, ability.teacher_loc);
					set_effect_max.set(SET, value_cum, ability.ability_cfg, ability.teacher_loc);
				}
				else {
					if(value_cum > set_effect_max.value) {
						set_effect_max.set(SET, value_cum, ability.ability_cfg, ability.teacher_loc);
					}
					if(value_cum < set_effect_min.value) {
						set_effect_min.set(SET, value_cum, ability.ability_cfg, ability.teacher_loc);
					}
				}
			}
		}

		if (const config::attribute_value *v = cfg.get("add")) {
			int add = get_single_ability_value(*v, def, ability, list.loc(), att, [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
				callable.add("base_value", wfl::variant(def));
				return formula.evaluate(callable).as_int();
			});
			std::map<std::string,individual_effect>::iterator add_effect = values_add.find(effect_id);
			if(add_effect == values_add.end() || add > add_effect->second.value) {
				values_add[effect_id].set(ADD, add, ability.ability_cfg, ability.teacher_loc);
			}
		}
		if (const config::attribute_value *v = cfg.get("sub")) {
			int sub = - get_single_ability_value(*v, def, ability, list.loc(), att, [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
				callable.add("base_value", wfl::variant(def));
				return formula.evaluate(callable).as_int();
			});
			std::map<std::string,individual_effect>::iterator sub_effect = values_add.find(effect_id);
			if(sub_effect == values_add.end() || sub < sub_effect->second.value) {
				values_add[effect_id].set(ADD, sub, ability.ability_cfg, ability.teacher_loc);
			}
		}
		if (const config::attribute_value *v = cfg.get("multiply")) {
			int multiply = static_cast<int>(get_single_ability_value(*v, static_cast<double>(def), ability, list.loc(), att, [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
				callable.add("base_value", wfl::variant(def));
				return formula.evaluate(callable).as_decimal() / 1000.0 ;
			}) * 100);
			std::map<std::string,individual_effect>::iterator mul_effect = values_mul.find(effect_id);
			if(mul_effect == values_mul.end() || multiply > mul_effect->second.value) {
				values_mul[effect_id].set(MUL, multiply, ability.ability_cfg, ability.teacher_loc);
			}
		}
		if (const config::attribute_value *v = cfg.get("divide")) {
			int divide = static_cast<int>(get_single_ability_value(*v, static_cast<double>(def), ability, list.loc(), att, [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
				callable.add("base_value", wfl::variant(def));
				return formula.evaluate(callable).as_decimal() / 1000.0 ;
			}) * 100);

			if (divide == 0) {
				ERR_NG << "division by zero with divide= in ability/weapon special " << effect_id;
			}
			else {
				std::map<std::string,individual_effect>::iterator div_effect = values_div.find(effect_id);
				if(div_effect == values_div.end() || divide > div_effect->second.value) {
					values_div[effect_id].set(DIV, divide, ability.ability_cfg, ability.teacher_loc);
				}
			}
		}
	}

	if(!is_cumulable && set_effect_max.type != NOT_USED) {
		value_set = std::max(set_effect_max.value, 0) + std::min(set_effect_min.value, 0);
		if(set_effect_max.value > def) {
			effect_list_.push_back(set_effect_max);
		}
		if(set_effect_min.value < def) {
			effect_list_.push_back(set_effect_min);
		}
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

	for(const auto& val : values_mul) {
		multiplier *= val.second.value/100.0;
		effect_list_.push_back(val.second);
	}

	for(const auto& val : values_div) {
		divisor *= val.second.value/100.0;
		effect_list_.push_back(val.second);
	}

	int addition = 0;
	for(const auto& val : values_add) {
		addition += val.second.value;
		effect_list_.push_back(val.second);
	}

	composite_value_ = static_cast<int>((value_set + addition) * multiplier / divisor);
}

} // end namespace unit_abilities
