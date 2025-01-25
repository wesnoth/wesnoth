/*
	Copyright (C) 2006 - 2024
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
#include "serialization/markup.hpp"
#include "game_board.hpp"
#include "game_version.hpp" // for version_info
#include "gettext.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/abilities.hpp"
#include "units/ability_tags.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"
#include "utils/config_filters.hpp"
#include "filter_context.hpp"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/function_gamestate.hpp"
#include "units/filter.hpp"
#include "deprecation.hpp"
#include <utility>



static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)

namespace {
	class temporary_facing
	{
		map_location::direction save_dir_;
		unit_const_ptr u_;
	public:
		temporary_facing(const unit_const_ptr& u, map_location::direction new_dir)
			: save_dir_(u ? u->facing() : map_location::direction::indeterminate)
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

const unit_map& get_unit_map()
{
	// Used if we're in the game, including during the construction of the display_context
	if(resources::gameboard) {
		return resources::gameboard->units();
	}

	// If we get here, we're in the scenario editor
	assert(display::get_singleton());
	return display::get_singleton()->context().units();
}

const team& get_team(std::size_t side)
{
	// Used if we're in the game, including during the construction of the display_context
	if(resources::gameboard) {
		return resources::gameboard->get_team(side);
	}

	// If we get here, we're in the scenario editor
	assert(display::get_singleton());
	return display::get_singleton()->context().get_team(side);
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
	for (const config &i : this->abilities_.child_range(tag_name)) {
		if (get_self_ability_bool(i, tag_name, loc))
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
		for (const config &j : it->abilities_.child_range(tag_name)) {
			if (get_adj_ability_bool(j, tag_name, i, loc,*it))
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
		if (get_self_ability_bool(i, tag_name, loc))
		{
			res.emplace_back(&i, loc, loc);
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
		for(const config& j : it->abilities_.child_range(tag_name)) {
			if(get_adj_ability_bool(j, tag_name, i, loc,*it))
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
	utils::erase_if(res, [&](const unit_ability& i) {
		return !ability_affects_weapon(*i.ability_cfg, weapon, false) || !ability_affects_weapon(*i.ability_cfg, opp_weapon, true);
	});
	return res;
}

std::vector<std::string> unit::get_ability_list() const
{
	std::vector<std::string> res;

	for(const auto [key, cfg] : this->abilities_.all_children_view()) {
		std::string id = cfg["id"];
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
	bool add_ability_tooltip(const config& ab, unit_race::GENDER gender, std::vector<std::tuple<std::string, t_string,t_string,t_string>>& res, bool active)
	{
		if (active) {
			const t_string& name = gender_value(ab, gender, "name", "female_name", "name").t_str();

			if (!name.empty()) {
				res.emplace_back(
						ab["id"],
						ab["name"].t_str(),
						name,
						ab["description"].t_str() );
				return true;
			}
		}
		else
		{
			// See if an inactive name was specified.
			const config::attribute_value& inactive_value =
				gender_value(ab, gender, "name_inactive",
						"female_name_inactive", "name_inactive");
			const t_string& name = !inactive_value.blank() ? inactive_value.t_str() :
				gender_value(ab, gender, "name", "female_name", "name").t_str();

			if (!name.empty()) {
				res.emplace_back(
						ab["id"],
						ab.get_or("name_inactive", "name").t_str(),
						name,
						ab.get_or("description_inactive", "description").t_str() );
				return true;
			}
		}

		return false;
	}
}

std::vector<std::tuple<std::string, t_string, t_string, t_string>> unit::ability_tooltips() const
{
	std::vector<std::tuple<std::string, t_string,t_string,t_string>> res;

	for(const auto [_, cfg] : this->abilities_.all_children_view())
	{
		add_ability_tooltip(cfg, gender_, res, true);
	}

	return res;
}

std::vector<std::tuple<std::string, t_string, t_string, t_string>> unit::ability_tooltips(boost::dynamic_bitset<>& active_list, const map_location& loc) const
{
	std::vector<std::tuple<std::string, t_string,t_string,t_string>> res;
	active_list.clear();

	for(const auto [key, cfg] : this->abilities_.all_children_view())
	{
		bool active = ability_active(key, cfg, loc);
		if (add_ability_tooltip(cfg, gender_, res, active))
		{
			active_list.push_back(active);
		}
	}
	return res;
}

namespace {
	/**
	 * Print "Recursion limit reached" log messages, including deduplication if the same problem has
	 * already been logged.
	 */
	void show_recursion_warning(const unit& unit, const config& filter) {
		// This function is only called when an ability is checked for the second time
		// filter has already been parsed multiple times, so I'm not trying to optimize the performance
		// of this; it's merely to prevent the logs getting spammed. For example, each of
		// four_cycle_recursion_branching and event_test_filter_attack_student_weapon_condition only log
		// 3 unique messages, but without deduplication they'd log 1280 and 392 respectively.
		static std::vector<std::tuple<std::string, std::string>> already_shown;

		auto identifier = std::tuple<std::string, std::string>{unit.id(), filter.debug()};
		if(utils::contains(already_shown, identifier)) {
			return;
		}

		std::string_view filter_text_view = std::get<1>(identifier);
		utils::trim(filter_text_view);
		ERR_NG << "Looped recursion error for unit '" << unit.id()
		<< "' while checking ability '" << filter_text_view << "'";

		// Arbitrary limit, just ensuring that having a huge number of specials causing recursion
		// warnings can't lead to unbounded memory consumption here.
		if(already_shown.size() > 100) {
			already_shown.clear();
		}
		already_shown.push_back(std::move(identifier));
	}
}//anonymous namespace

unit::recursion_guard unit::update_variables_recursion(const config& ability) const
{
	if(utils::contains(open_queries_, &ability)) {
		return recursion_guard();
	}
	return recursion_guard(*this, ability);
}

unit::recursion_guard::recursion_guard() = default;

unit::recursion_guard::recursion_guard(const unit & u, const config& ability)
	: parent(u.shared_from_this())
{
	u.open_queries_.emplace_back(&ability);
}

unit::recursion_guard::recursion_guard(unit::recursion_guard&& other)
{
	std::swap(parent, other.parent);
}

unit::recursion_guard::operator bool() const {
	return bool(parent);
}

unit::recursion_guard& unit::recursion_guard::operator=(unit::recursion_guard&& other)
{
	assert(this != &other);
	assert(!parent);
	std::swap(parent, other.parent);
	return *this;
}

unit::recursion_guard::~recursion_guard()
{
	if(parent) {
		assert(!parent->open_queries_.empty());
		parent->open_queries_.pop_back();
	}
}

bool unit::ability_active(const std::string& ability,const config& cfg,const map_location& loc) const
{
	auto filter_lock = update_variables_recursion(cfg);
	if(!filter_lock) {
		show_recursion_warning(*this, cfg);
		return false;
	}
	return ability_active_impl(ability, cfg, loc);
}

bool unit::ability_active_impl(const std::string& ability,const config& cfg,const map_location& loc) const
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
		std::vector<map_location::direction> dirs = map_location::parse_directions(i["adjacent"]);
		for (const map_location::direction index : dirs)
		{
			if (index == map_location::direction::indeterminate)
				continue;
			unit_map::const_iterator unit = units.find(adjacent[static_cast<int>(index)]);
			if (unit == units.end())
				return false;
			if (!ufilt(*unit, *this))
				return false;
			if((*this).id() == (*unit).id())
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
		if (!in_ranges<int>(count, utils::parse_ranges_unsigned(i["count"].str()))) {
			return false;
		}
	}

	for (const config &i : cfg.child_range("filter_adjacent_location"))
	{
		std::size_t count = 0;
		terrain_filter adj_filter(vconfig(i), resources::filter_con, false);
		adj_filter.flatten(illuminates);

		std::vector<map_location::direction> dirs = map_location::parse_directions(i["adjacent"]);
		for (const map_location::direction index : dirs)
		{
			if (index == map_location::direction::indeterminate) {
				continue;
			}
			if(!adj_filter.match(adjacent[static_cast<int>(index)])) {
				return false;
			}
			count++;
		}
		if (i["count"].empty() && count != dirs.size()) {
			return false;
		}
		if (!in_ranges<int>(count, utils::parse_ranges_unsigned(i["count"].str()))) {
			return false;
		}
	}
	return true;
}

bool unit::ability_affects_adjacent(const std::string& ability, const config& cfg,int dir,const map_location& loc,const unit& from) const
{
	bool illuminates = ability == "illuminates";

	assert(dir >=0 && dir <= 5);
	map_location::direction direction{ dir };

	for (const config &i : cfg.child_range("affect_adjacent"))
	{
		if (i.has_attribute("adjacent")) { //key adjacent defined
			std::vector<map_location::direction> dirs = map_location::parse_directions(i["adjacent"]);
			if (std::find(dirs.begin(), dirs.end(), direction) == dirs.end()) {
				continue;
			}
		}
		if((*this).id() == from.id()){
			return false;
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

bool unit::ability_affects_weapon(const config& cfg, const const_attack_ptr& weapon, bool is_opp) const
{
	const std::string filter_tag_name = is_opp ? "filter_second_weapon" : "filter_weapon";
	if(!cfg.has_child(filter_tag_name)) {
		return true;
	}
	const config& filter = cfg.mandatory_child(filter_tag_name);
	if(!weapon) {
		return false;
	}
	attack_type::recursion_guard filter_lock;
	filter_lock  = weapon->update_variables_recursion(cfg);
	if(!filter_lock) {
		show_recursion_warning(*this, cfg);
		return false;
	}
	return weapon->matches_filter(filter);
}

bool unit::has_ability_type(const std::string& ability) const
{
	return !abilities_.child_range(ability).empty();
}

//these two functions below are used in order to add to the unit
//a second set of halo encoded in the abilities (like illuminates halo in [illuminates] ability for example)
static void add_string_to_vector(std::vector<std::string>& image_list, const config& cfg, const std::string& attribute_name)
{
	auto ret = std::find(image_list.begin(), image_list.end(), cfg[attribute_name].str());
	if(ret == image_list.end()){
		image_list.push_back(cfg[attribute_name].str());
	}
}

std::vector<std::string> unit::halo_or_icon_abilities(const std::string& image_type) const
{
	std::vector<std::string> image_list;
	for(const auto [key, cfg] : abilities_.all_children_view()){
		bool is_active = ability_active(key, cfg, loc_);
		//Add halo/overlay to owner of ability if active and affect_self is true.
		if( !cfg[image_type + "_image"].str().empty() && is_active && ability_affects_self(key, cfg, loc_)){
			add_string_to_vector(image_list, cfg,image_type + "_image");
		}
		//Add halo/overlay to owner of ability who affect adjacent only if active.
		if(!cfg[image_type + "_image_self"].str().empty() && is_active){
			add_string_to_vector(image_list, cfg, image_type + "_image_self");
		}
	}

	const unit_map& units = get_unit_map();

	//Add halo/overlay to unit under abilities owned by adjacent who has [affect_adjacent]
	//if condition matched
	const auto adjacent = get_adjacent_tiles(loc_);
	for(unsigned i = 0; i < adjacent.size(); ++i) {
		const unit_map::const_iterator it = units.find(adjacent[i]);
		if (it == units.end() || it->incapacitated())
			continue;
		if ( &*it == this )
			continue;
		for(const auto [key, cfg] : it->abilities_.all_children_view()) {
			if(!cfg[image_type + "_image"].str().empty() && affects_side(cfg, side(), it->side()) && it->ability_active(key, cfg, adjacent[i]) && ability_affects_adjacent(key, cfg, i, loc_, *it))
			{
				add_string_to_vector(image_list, cfg, image_type + "_image");
			}
		}
	}
	//rearranges vector alphabetically when its size equals or exceeds two.
	if(image_list.size() >= 2){
		std::sort(image_list.begin(), image_list.end());
	}
	return image_list;
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
T get_single_ability_value(const config::attribute_value& v, T def, const unit_ability& ability_info, const map_location& receiver_loc, const const_attack_ptr& att, const TFuncFormula& formula_handler)
{
	return v.apply_visitor(get_ability_value_visitor(def, [&](const std::string& s) {

			try {
				const unit_map& units = get_unit_map();

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
		for(const auto [key, cfg] : parent.all_children_view())
		{
			if (just_peeking && (key == id || cfg["id"] == id)) {
				return true; // peek succeeded; done
			}

			if(key == id) {
				special_match special = { key, &cfg };
				tag_result.push_back(special);
			}
			if(cfg["id"] == id) {
				special_match special = { key, &cfg };
				id_result.push_back(special);
			}
		}
		return false;
	}

	bool get_special_children_id(std::vector<special_match>& id_result,
	                           const config& parent, const std::string& id,
	                           bool just_peeking=false) {
		for(const auto [key, cfg] : parent.all_children_view())
		{
			if (just_peeking && (cfg["id"] == id)) {
				return true; // peek succeeded; done
			}

			if(cfg["id"] == id) {
				special_match special = { key, &cfg };
				id_result.push_back(special);
			}
		}
		return false;
	}

	bool get_special_children_tags(std::vector<special_match>& tag_result,
	                           const config& parent, const std::string& id,
	                           bool just_peeking=false) {
		for(const auto [key, cfg] : parent.all_children_view())
		{
			if (just_peeking && (key == id)) {
				return true; // peek succeeded; done
			}

			if(key == id) {
				special_match special = { key, &cfg };
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

	for(const auto [key, cfg] : specials_.all_children_view())
	{
		if ( !active_list || special_active(cfg, AFFECT_EITHER, key) ) {
			const t_string &name = cfg["name"];
			if (!name.empty()) {
				res.emplace_back(name, cfg["description"].t_str() );
				if ( active_list )
					active_list->push_back(true);
			}
		} else {
			const t_string& name = cfg.get_or("name_inactive", "name").t_str();
			if (!name.empty()) {
				res.emplace_back(name, cfg.get_or("description_inactive", "description").t_str() );
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
static void add_name(std::string& temp_string, bool active, const std::string& name, std::set<std::string>& checking_name)
{
	if (active) {
		if (!name.empty() && checking_name.count(name) == 0) {
			checking_name.insert(name);
			if (!temp_string.empty()) temp_string += ", ";
			temp_string += markup::span_color(font::BUTTON_COLOR, name);
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
	for(const auto [key, cfg] : specials_.all_children_view())
	{
		const bool active = special_active(cfg, AFFECT_EITHER, key);

		const std::string& name =
			active
			? cfg["name"].str()
			: cfg.get_or("name_inactive", "name").str();
		if (!name.empty()) {
			if (!res.empty()) {
				res += ", ";
			}

			if (!active) {
				res += markup::span_color(font::inactive_details_color, name);
			} else {
				res += name;
			}
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

static void add_name_list(std::string& temp_string, std::string& weapon_abilities, std::set<std::string>& checking_name, const std::string& from_str)
{
	if(!temp_string.empty()){
		temp_string = from_str.c_str() + temp_string;
		weapon_abilities += (!weapon_abilities.empty() && !temp_string.empty()) ? "\n" : "";
		weapon_abilities += temp_string;
		temp_string.clear();
		checking_name.clear();
	}
}

std::string attack_type::weapon_specials_value(const std::set<std::string>& checking_tags) const
{
	//log_scope("weapon_specials_value");
	std::string temp_string, weapon_abilities;
	std::set<std::string> checking_name;
	for(const auto [key, cfg] : specials_.all_children_view()) {
		if((checking_tags.count(key) != 0)){
			const bool active = special_active(cfg, AFFECT_SELF, key);
			add_name(temp_string, active, cfg["name"].str(), checking_name);
		}
	}
	add_name_list(temp_string, weapon_abilities, checking_name, "");

	weapon_specials_impl_self(temp_string, self_, shared_from_this(), other_attack_, self_loc_, AFFECT_SELF, checking_name, checking_tags, true);
	add_name_list(temp_string, weapon_abilities, checking_name, _("Owned: "));

	weapon_specials_impl_adj(temp_string, self_, shared_from_this(), other_attack_, self_loc_, AFFECT_SELF, checking_name, checking_tags, "affect_allies", true);
	// TRANSLATORS: Past-participle of "teach", used for an ability similar to leadership
	add_name_list(temp_string, weapon_abilities, checking_name, _("Taught: "));

	weapon_specials_impl_adj(temp_string, self_, shared_from_this(), other_attack_, self_loc_, AFFECT_SELF, checking_name, checking_tags, "affect_enemies", true);
	// TRANSLATORS: Past-participle of "teach", used for an ability similar to leadership
	add_name_list(temp_string, weapon_abilities, checking_name, _("Taught: (by an enemy): "));


	if(other_attack_) {
		for(const auto [key, cfg] : other_attack_->specials_.all_children_view()) {
			if((checking_tags.count(key) != 0)){
				const bool active = other_attack_->special_active(cfg, AFFECT_OTHER, key);
				add_name(temp_string, active, cfg["name"].str(), checking_name);
			}
		}
	}
	weapon_specials_impl_self(temp_string, other_, other_attack_, shared_from_this(), other_loc_, AFFECT_OTHER, checking_name, checking_tags);
	weapon_specials_impl_adj(temp_string, other_, other_attack_, shared_from_this(), other_loc_, AFFECT_OTHER, checking_name, checking_tags);
	add_name_list(temp_string, weapon_abilities, checking_name, _("Used by opponent: "));

	return weapon_abilities;
}

void attack_type::weapon_specials_impl_self(
	std::string& temp_string,
	const unit_const_ptr& self,
	const const_attack_ptr& self_attack,
	const const_attack_ptr& other_attack,
	const map_location& self_loc,
	AFFECTS whom,
	std::set<std::string>& checking_name,
	const std::set<std::string>& checking_tags,
	bool leader_bool)
{
	if(self){
		for(const auto [key, cfg] : self->abilities().all_children_view()){
			bool tag_checked = (!checking_tags.empty()) ? (checking_tags.count(key) != 0) : true;
			const bool active = tag_checked && check_self_abilities_impl(self_attack, other_attack, cfg, self, self_loc, whom, key, leader_bool);
			add_name(temp_string, active, cfg["name"].str(), checking_name);
		}
	}
}

void attack_type::weapon_specials_impl_adj(
	std::string& temp_string,
	const unit_const_ptr& self,
	const const_attack_ptr& self_attack,
	const const_attack_ptr& other_attack,
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
			for(const auto [key, cfg] : it->abilities().all_children_view()) {
				bool tag_checked = (!checking_tags.empty()) ? (checking_tags.count(key) != 0) : true;
				bool default_bool = (affect_adjacents == "affect_allies") ? true : false;
				bool affect_allies = (!affect_adjacents.empty()) ? cfg[affect_adjacents].to_bool(default_bool) : true;
				const bool active = tag_checked && check_adj_abilities_impl(self_attack, other_attack, cfg, self, *it, i, self_loc, whom, key, leader_bool) && affect_allies;
				add_name(temp_string, active, cfg["name"].str(), checking_name);
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
	weapon.self_ = std::move(self);
	weapon.other_ = std::move(other);
	weapon.self_loc_ = unit_loc;
	weapon.other_loc_ = other_loc;
	weapon.is_attacker_ = attacking;
	weapon.other_attack_ = std::move(other_attack);
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
	weapon.self_ = std::move(self);
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
attack_type::specials_context_t::specials_context_t(const attack_type& weapon, const unit_type& /*self_type*/, const map_location& loc, bool attacking)
	: parent(weapon.shared_from_this())
{
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
	: parent(std::move(other.parent))
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
		attacks_value = 0;
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

static std::string select_replacement_type(const unit_ability_list& damage_type_list)
{
	std::map<std::string, unsigned int> type_count;
	unsigned int max = 0;
	for(auto& i : damage_type_list) {
		const config& c = *i.ability_cfg;
		if(c.has_attribute("replacement_type")) {
			std::string type = c["replacement_type"].str();
			unsigned int count = ++type_count[type];
			if((count > max)) {
				max = count;
			}
		}
	}

	if (type_count.empty()) return "";

	std::vector<std::string> type_list;
	for(auto& i : type_count){
		if(i.second == max){
			type_list.push_back(i.first);
		}
	}

	if(type_list.empty()) return "";

	return type_list.front();
}

static std::string select_alternative_type(const unit_ability_list& damage_type_list, const unit_ability_list& resistance_list, const unit& u)
{
	std::map<std::string, int> type_res;
	int max_res = std::numeric_limits<int>::min();
	for(auto& i : damage_type_list) {
		const config& c = *i.ability_cfg;
		if(c.has_attribute("alternative_type")) {
			std::string type = c["alternative_type"].str();
			if(type_res.count(type) == 0){
				type_res[type] = u.resistance_value(resistance_list, type);
				max_res = std::max(max_res, type_res[type]);
			}
		}
	}

	if (type_res.empty()) return "";

	std::vector<std::string> type_list;
	for(auto& i : type_res){
		if(i.second == max_res){
			type_list.push_back(i.first);
		}
	}
	if(type_list.empty()) return "";

	return type_list.front();
}

std::string attack_type::select_damage_type(const unit_ability_list& damage_type_list, const std::string& key_name, const unit_ability_list& resistance_list) const
{
	bool is_alternative = (key_name == "alternative_type");
	if(is_alternative && other_){
		return select_alternative_type(damage_type_list, resistance_list, (*other_));
	} else if(!is_alternative){
		return select_replacement_type(damage_type_list);
	}
	return "";
}

/**
 * Returns the type of damage inflicted.
 */
std::pair<std::string, std::string> attack_type::damage_type() const
{
	if(attack_empty()){
		return {"", ""};
	}
	unit_ability_list damage_type_list = get_specials_and_abilities("damage_type");
	if(damage_type_list.empty()){
		return {type(), ""};
	}

	unit_ability_list resistance_list;
	if(other_){
		resistance_list = (*other_).get_abilities_weapons("resistance", other_loc_, other_attack_, shared_from_this());
	}
	std::string replacement_type = select_damage_type(damage_type_list, "replacement_type", resistance_list);
	std::string alternative_type = select_damage_type(damage_type_list, "alternative_type", resistance_list);
	std::string type_damage = replacement_type.empty() ? type() : replacement_type;
	if(!alternative_type.empty() && type_damage != alternative_type){
		return {type_damage, alternative_type};
	}
	return {type_damage, ""};
}

std::set<std::string> attack_type::alternative_damage_types() const
{
	unit_ability_list damage_type_list = get_specials_and_abilities("damage_type");
	if(damage_type_list.empty()){
		return {};
	}
	std::set<std::string> damage_types;
	for(auto& i : damage_type_list) {
		const config& c = *i.ability_cfg;
		if(c.has_attribute("alternative_type")){
			damage_types.insert(c["alternative_type"].str());
		}
	}

	return damage_types;
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
	 * Print "Recursion limit reached" log messages, including deduplication if the same problem has
	 * already been logged.
	 */
	void show_recursion_warning(const const_attack_ptr& attack, const config& filter) {
		// This function is only called when a special is checked for the second time
		// filter has already been parsed multiple times, so I'm not trying to optimize the performance
		// of this; it's merely to prevent the logs getting spammed. For example, each of
		// four_cycle_recursion_branching and event_test_filter_attack_student_weapon_condition only log
		// 3 unique messages, but without deduplication they'd log 1280 and 392 respectively.
		static std::vector<std::tuple<std::string, std::string>> already_shown;

		auto identifier = std::tuple<std::string, std::string>{attack->id(), filter.debug()};
		if(utils::contains(already_shown, identifier)) {
			return;
		}

		std::string_view filter_text_view = std::get<1>(identifier);
		utils::trim(filter_text_view);
		ERR_NG << "Looped recursion error for weapon '" << attack->id()
		<< "' while checking weapon special '" << filter_text_view << "'";

		// Arbitrary limit, just ensuring that having a huge number of specials causing recursion
		// warnings can't lead to unbounded memory consumption here.
		if(already_shown.size() > 100) {
			already_shown.clear();
		}
		already_shown.push_back(std::move(identifier));
	}

	/**
	 * Determines if a unit/weapon combination matches the specified child
	 * (normally a [filter_*] child) of the provided filter.
	 * @param[in]  u           A unit to filter.
	 * @param[in]  u2          Another unit to filter.
	 * @param[in]  loc         The presumed location of @a unit.
	 * @param[in]  weapon      The attack_type to filter.
	 * @param[in]  filter      The filter containing the child filter to use.
	 * @param[in]  for_listing
	 * @param[in]  child_tag   The tag of the child filter to use.
	 * @param[in]  check_if_recursion    Parameter used for don't have infinite recusion for some filter attribute.
	 */
	static bool special_unit_matches(unit_const_ptr & u,
		                             unit_const_ptr & u2,
		                             const map_location & loc,
		                             const const_attack_ptr& weapon,
		                             const config & filter,
									 const bool for_listing,
		                             const std::string & child_tag, const std::string& check_if_recursion)
	{
		if (for_listing && !loc.valid())
			// The special's context was set to ignore this unit, so assume we pass.
			// (This is used by reports.cpp to show active specials when the
			// opponent is not known. From a player's perspective, the special
			// is active, in that it can be used, even though the player might
			// need to select an appropriate opponent.)
			return true;

		//Add wml filter if "backstab" attribute used.
		if (!filter["backstab"].blank() && child_tag == "filter_opponent") {
			deprecated_message("backstab= in weapon specials", DEP_LEVEL::INDEFINITE, "", "Use [filter_opponent] with a formula instead; the code can be found in data/core/macros/ in the WEAPON_SPECIAL_BACKSTAB macro.");
		}
		config cfg = filter;
		if(filter["backstab"].to_bool() && child_tag == "filter_opponent"){
			const std::string& backstab_formula = "enemy_of(self, flanker) and not flanker.petrified where flanker = unit_at(direction_from(loc, other.facing))";
			config& filter_child = cfg.child_or_add("filter_opponent");
			if(!filter.has_child("filter_opponent")){
				filter_child["formula"] = backstab_formula;
			} else {
				config filter_opponent;
				filter_opponent["formula"] = backstab_formula;
				filter_child.add_child("and", filter_opponent);
			}
		}
		const config& filter_backstab = filter["backstab"].to_bool() ? cfg : filter;

		auto filter_child = filter_backstab.optional_child(child_tag);
		if ( !filter_child )
			// The special does not filter on this unit, so we pass.
			return true;

		// If the primary unit doesn't exist, there's nothing to match
		if (!u) {
			return false;
		}

		unit_filter ufilt{vconfig(*filter_child)};

		// If the other unit doesn't exist, try matching without it


		attack_type::recursion_guard filter_lock;
		if (weapon && (filter_child->optional_child("has_attack") || filter_child->optional_child("filter_weapon"))) {
			filter_lock  = weapon->update_variables_recursion(filter);
			if(!filter_lock) {
				show_recursion_warning(weapon, filter);
				return false;
			}
		}
		// Check for a weapon match.
		if (auto filter_weapon = filter_child->optional_child("filter_weapon") ) {
			if ( !weapon || !weapon->matches_filter(*filter_weapon, check_if_recursion) )
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
	if(self_) {
		abil_list.append_if((*self_).get_abilities(ability, self_loc_), [&](const unit_ability& i) {
			return special_active(*i.ability_cfg, AFFECT_SELF, ability, "filter_student");
		});
	}

	if(other_) {
		abil_list.append_if((*other_).get_abilities(ability, other_loc_), [&](const unit_ability& i) {
			return special_active_impl(other_attack_, shared_from_this(), *i.ability_cfg, AFFECT_OTHER, ability, "filter_student");
		});
	}

	return abil_list;
}

unit_ability_list attack_type::get_specials_and_abilities(const std::string& special) const
{
	// get all weapon specials of the provided type
	unit_ability_list abil_list = get_specials(special);
	// append all such weapon specials as abilities as well
	abil_list.append(get_weapon_ability(special));
	// get a list of specials/"specials as abilities" that may potentially overwrite others
	unit_ability_list overwriters = overwrite_special_overwriter(abil_list, special);
	if(!abil_list.empty() && !overwriters.empty()){
		// remove all abilities that would be overwritten
		utils::erase_if(abil_list, [&](const unit_ability& j) {
			return (overwrite_special_checking(overwriters, *j.ability_cfg, special));
		});
	}
	return abil_list;
}

int attack_type::composite_value(const unit_ability_list& abil_list, int base_value) const
{
	return unit_abilities::effect(abil_list, base_value, shared_from_this()).get_composite_value();
}

static bool overwrite_special_affects(const config& special)
{
	const std::string& apply_to = special["overwrite_specials"];
	return (apply_to == "one_side" || apply_to == "both_sides");
}

unit_ability_list attack_type::overwrite_special_overwriter(unit_ability_list overwriters, const std::string& tag_name) const
{
	//remove element without overwrite_specials key, if list empty after check return empty list.
	utils::erase_if(overwriters, [&](const unit_ability& i) {
		return (!overwrite_special_affects(*i.ability_cfg));
	});

	// if empty, nothing is doing any overwriting
	if(overwriters.empty()){
		return overwriters;
	}

	// if there are specials/"specials as abilities" that could potentially overwrite each other
	if(overwriters.size() >= 2){
		// sort them by overwrite priority from highest to lowest (default priority is 0)
		utils::sort_if(overwriters,[](const unit_ability& i, const unit_ability& j){
			auto oi = (*i.ability_cfg).optional_child("overwrite");
			double l = 0;
			if(oi && !oi["priority"].empty()){
				l = oi["priority"].to_double(0);
			}
			auto oj = (*j.ability_cfg).optional_child("overwrite");
			double r = 0;
			if(oj && !oj["priority"].empty()){
				r = oj["priority"].to_double(0);
			}
			return l > r;
		});
		// remove any that need to be overwritten
		utils::erase_if(overwriters, [&](const unit_ability& i) {
			return (overwrite_special_checking(overwriters, *i.ability_cfg, tag_name));
		});
	}
	return overwriters;
}

bool attack_type::overwrite_special_checking(unit_ability_list& overwriters, const config& cfg, const std::string& tag_name) const
{
	if(overwriters.empty()){
		return false;
	}

	for(const auto& j : overwriters) {
		// whether the overwriter affects a single side
		bool affect_side = ((*j.ability_cfg)["overwrite_specials"] == "one_side");
		// the overwriter's priority, default of 0
		auto overwrite_specials = (*j.ability_cfg).optional_child("overwrite");
		double priority = overwrite_specials ? overwrite_specials["priority"].to_double(0) : 0.00;
		// the cfg being checked for whether it will be overwritten
		auto has_overwrite_specials = cfg.optional_child("overwrite");
		// if the overwriter's priority is greater than 0, then true if the cfg being checked has a higher priority
		// else true
		bool prior = (priority > 0) ? (has_overwrite_specials && has_overwrite_specials["priority"].to_double(0) >= priority) : true;
		// true if the cfg being checked affects one or both sides and doesn't have a higher priority, or if it doesn't affect one or both sides
		// aka whether the cfg being checked can potentially be overwritten by the current overwriter
		bool is_overwritable = (overwrite_special_affects(cfg) && !prior) || !overwrite_special_affects(cfg);
		bool one_side_overwritable = true;

		// if the current overwriter affects one side and the cfg being checked can be overwritten by this overwriter
		// then check that the current overwriter and the cfg being checked both affect either this unit or its opponent
		if(affect_side && is_overwritable){
			if(special_affects_self(*j.ability_cfg, is_attacker_)){
				one_side_overwritable = special_affects_self(cfg, is_attacker_);
			}
			else if(special_affects_opponent(*j.ability_cfg, !is_attacker_)){
				one_side_overwritable = special_affects_opponent(cfg, !is_attacker_);
			}
		}

		// check whether the current overwriter is disabled due to a filter
		bool special_matches = true;
		if(overwrite_specials){
			auto overwrite_filter = (*overwrite_specials).optional_child("filter_specials");
			if(!overwrite_filter){
				overwrite_filter = (*overwrite_specials).optional_child("experimental_filter_specials");
				if(overwrite_filter){
					deprecated_message("experimental_filter_specials", DEP_LEVEL::INDEFINITE, "", "Use filter_specials instead.");
				}
			}
			if(overwrite_filter && is_overwritable && one_side_overwritable){
				special_matches = special_matches_filter(cfg, tag_name, *overwrite_filter);
			}
		}

		// if the cfg being checked should be overwritten
		// and either this unit or its opponent are affected
		// and the current overwriter is not disabled due to a filter
		if(is_overwritable && one_side_overwritable && special_matches){
			return true;
		}
	}
	return false;
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

bool unit::get_self_ability_bool(const config& cfg, const std::string& ability, const map_location& loc) const
{
	auto filter_lock = update_variables_recursion(cfg);
	if(!filter_lock) {
		show_recursion_warning(*this, cfg);
		return false;
	}
	return (ability_active_impl(ability, cfg, loc) && ability_affects_self(ability, cfg, loc));
}

bool unit::get_adj_ability_bool(const config& cfg, const std::string& ability, int dir, const map_location& loc, const unit& from) const
{
	auto filter_lock = from.update_variables_recursion(cfg);
	if(!filter_lock) {
		show_recursion_warning(from, cfg);
		return false;
	}
	const auto adjacent = get_adjacent_tiles(loc);
	return (affects_side(cfg, side(), from.side()) && from.ability_active_impl(ability, cfg, adjacent[dir]) && ability_affects_adjacent(ability, cfg, dir, loc, from));
}

bool unit::get_self_ability_bool_weapon(const config& special, const std::string& tag_name, const map_location& loc, const const_attack_ptr& weapon, const const_attack_ptr& opp_weapon) const
{
	return (get_self_ability_bool(special, tag_name, loc) && ability_affects_weapon(special, weapon, false) && ability_affects_weapon(special, opp_weapon, true));
}

bool unit::get_adj_ability_bool_weapon(const config& special, const std::string& tag_name, int dir, const map_location& loc, const unit& from, const const_attack_ptr& weapon, const const_attack_ptr& opp_weapon) const
{
	return (get_adj_ability_bool(special, tag_name, dir, loc, from) && ability_affects_weapon(special, weapon, false) && ability_affects_weapon(special, opp_weapon, true));
}

bool attack_type::check_self_abilities(const config& cfg, const std::string& special) const
{
	return check_self_abilities_impl(shared_from_this(), other_attack_, cfg, self_, self_loc_, AFFECT_SELF, special, true);
}

bool attack_type::check_self_abilities_impl(const const_attack_ptr& self_attack, const const_attack_ptr& other_attack, const config& special, const unit_const_ptr& u, const map_location& loc, AFFECTS whom, const std::string& tag_name, bool leader_bool)
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

bool attack_type::check_adj_abilities_impl(const const_attack_ptr& self_attack, const const_attack_ptr& other_attack, const config& special, const unit_const_ptr& u, const unit& from, int dir, const map_location& loc, AFFECTS whom, const std::string& tag_name, bool leader_bool)
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
	//Now that filter_(second)attack in event supports special_id/type_active, including abilities used as weapons,
	//these can be detected even in placeholder attacks generated to compensate for the lack of attack in defense against an attacker using a range attack not possessed by the defender.
	//It is therefore necessary to check if the range is not empty (proof that the weapon is not a placeholder) to decide if has_weapon_ability can be returned or not.
	if(range().empty()){
		return false;
	}
	return (has_special(special, false, special_id, special_tags) || has_weapon_ability(special, special_id, special_tags));
}
//end of emulate weapon special functions.

namespace
{
	bool exclude_ability_attributes(const std::string& tag_name, const config & filter)
	{
		///check what filter attributes used can be used in type of ability checked.
		bool abilities_check = abilities_list::ability_value_tags().count(tag_name) != 0 || abilities_list::ability_no_value_tags().count(tag_name) != 0;
		if(filter.has_attribute("active_on") && tag_name != "resistance" && abilities_check)
			return false;
		if(filter.has_attribute("apply_to")  && tag_name != "resistance" && abilities_check)
			return false;

		if(filter.has_attribute("overwrite_specials") && abilities_list::weapon_number_tags().count(tag_name) == 0)
			return false;

		bool no_value_weapon_abilities_check =  abilities_list::no_weapon_number_tags().count(tag_name) != 0 || abilities_list::ability_no_value_tags().count(tag_name) != 0;
		if(filter.has_attribute("value") && no_value_weapon_abilities_check)
			return false;
		if(filter.has_attribute("add") && no_value_weapon_abilities_check)
			return false;
		if(filter.has_attribute("sub") && no_value_weapon_abilities_check)
			return false;
		if(filter.has_attribute("multiply") && no_value_weapon_abilities_check)
			return false;
		if(filter.has_attribute("divide") && no_value_weapon_abilities_check)
			return false;

		bool all_engine =  abilities_list::no_weapon_number_tags().count(tag_name) != 0 || abilities_list::weapon_number_tags().count(tag_name) != 0 || abilities_list::ability_value_tags().count(tag_name) != 0 || abilities_list::ability_no_value_tags().count(tag_name) != 0;
		if(filter.has_attribute("replacement_type") && tag_name != "damage_type" && all_engine)
			return false;
		if(filter.has_attribute("alternative_type") && tag_name != "damage_type" && all_engine)
			return false;
		if(filter.has_attribute("type") && tag_name != "plague" && all_engine)
			return false;

		return true;
	}

	bool matches_ability_filter(const config & cfg, const std::string& tag_name, const config & filter)
	{
		using namespace utils::config_filters;

		//check if attributes have right to be in type of ability checked
		if(!exclude_ability_attributes(tag_name, filter))
			return false;

		// tag_name and id are equivalent of ability ability_type and ability_id/type_active filters
		//can be extent to special_id/type_active. If tag_name or id matche if present in list.
		const std::vector<std::string> filter_type = utils::split(filter["tag_name"]);
		if ( !filter_type.empty() && std::find(filter_type.begin(), filter_type.end(), tag_name) == filter_type.end() )
			return false;

		if(!string_matches_if_present(filter, cfg, "id", ""))
			return false;

		//when affect_adjacent=yes detect presence of [affect_adjacent] in abilities, if no
		//then matches when tag not present.
		if(!filter["affect_adjacent"].empty()){
			bool adjacent = cfg.has_child("affect_adjacent");
			if(filter["affect_adjacent"].to_bool() != adjacent){
				return false;
			}
		}

		//these attributs below filter attribute used in all engine abilities.
		//matches if filter attribute have same boolean value what attribute
		if(!bool_matches_if_present(filter, cfg, "affect_self", true))
			return false;

		//here if value of affect_allies but also his presence who is checked because
		//when affect_allies not specified, ability affect unit of same side what owner only.
		if(!bool_or_empty(filter, cfg, "affect_allies"))
			return false;

		if(!bool_matches_if_present(filter, cfg, "affect_enemies", false))
			return false;


		//cumulative, overwrite_specials and active_on check attributes used in all abilities
		//who return a numerical value.
		if(!bool_matches_if_present(filter, cfg, "cumulative", false))
			return false;

		if(!string_matches_if_present(filter, cfg, "overwrite_specials", "none"))
			return false;

		if(!string_matches_if_present(filter, cfg, "active_on", "both"))
			return false;

		//value, add, sub multiply and divide check values of attribute used in engines abilities(default value of 'value' can be checked when not specified)
		//who return numericals value but can also check in non-engine abilities(in last case if 'value' not specified none value can matches)
		if(!filter["value"].empty()){
			if(tag_name == "drains"){
				if(!int_matches_if_present(filter, cfg, "value", 50)){
					return false;
				}
			} else if(tag_name == "berserk"){
				if(!int_matches_if_present(filter, cfg, "value", 1)){
					return false;
				}
			} else if(tag_name == "heal_on_hit" || tag_name == "heals" || tag_name == "regenerate" || tag_name == "leadership"){
				if(!int_matches_if_present(filter, cfg, "value" , 0)){
					return false;
				}
			} else {
				if(!int_matches_if_present(filter, cfg, "value")){
					return false;
				}
			}
		}

		if(!int_matches_if_present_or_negative(filter, cfg, "add", "sub"))
			return false;

		if(!int_matches_if_present_or_negative(filter, cfg, "sub", "add"))
			return false;

		if(!double_matches_if_present(filter, cfg, "multiply"))
			return false;

		if(!double_matches_if_present(filter, cfg, "divide"))
			return false;


		//apply_to is a special case, in resistance ability, it check a list of damage type used by [resistance]
		//but in weapon specials, check identity of unit affected by special(self, opponent tc...)
		if(tag_name == "resistance"){
			if(!set_includes_if_present(filter, cfg, "apply_to")){
				return false;
			}
		} else {
			if(!string_matches_if_present(filter, cfg, "apply_to", "self")){
				return false;
			}
		}

		//the three attribute below are used for check in specifics abilitie:
		//replacement_type and alternative_type are present in [damage_type] only for engine abilities
		//and type for [plague], but if someone want use this in non-engine abilities, these attribute can be checked outside type mentioned.
		//

		//for damage_type only(in engine cases)
		if(!string_matches_if_present(filter, cfg, "replacement_type", ""))
			return false;

		if(!string_matches_if_present(filter, cfg, "alternative_type", ""))
			return false;

		//for plague only(in engine cases)
		if(!string_matches_if_present(filter, cfg, "type", ""))
			return false;

		//the wml_filter is used in cases where the attribute we are looking for is not
		//previously listed or to check the contents of the sub_tags ([filter_adjacent],[filter_self],[filter_opponent] etc.
		//If the checked set does not exactly match the content of the capability, the function returns a false response.
		auto fwml = filter.optional_child("filter_wml");
		if (fwml){
			if(!cfg.matches(*fwml)){
				return false;
			}
		}

		// Passed all tests.
		return true;
	}

	static bool common_matches_filter(const config & cfg, const std::string& tag_name, const config & filter)
	{
		// Handle the basic filter.
		bool matches = matches_ability_filter(cfg, tag_name, filter);

		// Handle [and], [or], and [not] with in-order precedence
		for(const auto [key, condition_cfg] : filter.all_children_view() )
		{
			// Handle [and]
			if ( key == "and" )
				matches = matches && common_matches_filter(cfg, tag_name, condition_cfg);

			// Handle [or]
			else if ( key == "or" )
				matches = matches || common_matches_filter(cfg, tag_name, condition_cfg);

			// Handle [not]
			else if ( key == "not" )
				matches = matches && !common_matches_filter(cfg, tag_name, condition_cfg);
		}

		return matches;
	}
}

bool unit::ability_matches_filter(const config & cfg, const std::string& tag_name, const config & filter) const
{
	return common_matches_filter(cfg, tag_name, filter);
}

bool attack_type::special_matches_filter(const config & cfg, const std::string& tag_name, const config & filter) const
{
	return common_matches_filter(cfg, tag_name, filter);
}

bool attack_type::has_special_with_filter(const config & filter) const
{
	using namespace utils::config_filters;
	bool check_if_active = filter["active"].to_bool();
	for(const auto [key, cfg] : specials().all_children_view()) {
		if(special_matches_filter(cfg, key, filter)){
			if(!check_if_active){
				return true;
			}
			if ( special_active(cfg, AFFECT_SELF, key) ) {
				return true;
			}
		}
	}

	if(!check_if_active || !other_attack_){
		return false;
	}

	for(const auto [key, cfg] : other_attack_->specials().all_children_view()) {
		if(other_attack_->special_matches_filter(cfg, key, filter)){
			if ( other_attack_->special_active(cfg, AFFECT_OTHER, key) ) {
				return true;
			}
		}
	}

	return false;
}

bool attack_type::has_ability_with_filter(const config & filter) const
{
	if(!filter["active"].to_bool()){
		return false;
	}
	const unit_map& units = get_unit_map();
	if(self_){
		for(const auto [key, cfg] : (*self_).abilities().all_children_view()) {
			if(self_->ability_matches_filter(cfg, key, filter)){
				if(check_self_abilities(cfg, key)){
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

			for(const auto [key, cfg] : it->abilities().all_children_view()) {
				if(it->ability_matches_filter(cfg, key, filter) && check_adj_abilities(cfg, key, i , *it)){
					return true;
				}
			}
		}
	}

	if(other_){
		for(const auto [key, cfg] : (*other_).abilities().all_children_view()) {
			if(other_->ability_matches_filter(cfg, key, filter) && check_self_abilities_impl(other_attack_, shared_from_this(), cfg, other_, other_loc_, AFFECT_OTHER, key)){
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

			for(const auto [key, cfg] : it->abilities().all_children_view()) {
				if(it->ability_matches_filter(cfg, key, filter) && check_adj_abilities_impl(other_attack_, shared_from_this(), cfg, other_, *it, i, other_loc_, AFFECT_OTHER, key)){
					return true;
				}
			}
		}
	}
	return false;
}

bool attack_type::has_special_or_ability_with_filter(const config & filter) const
{
	if(range().empty()){
		return false;
	}
	return (has_special_with_filter(filter) || has_ability_with_filter(filter));
}

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
	const const_attack_ptr& self_attack,
	const const_attack_ptr& other_attack,
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
	const const_attack_ptr& att_weapon = is_attacker ? self_attack : other_attack;
	const const_attack_ptr& def_weapon = is_attacker ? other_attack : self_attack;

	// Filter firststrike here, if both units have first strike then the effects cancel out. Only check
	// the opponent if "whom" is the defender, otherwise this leads to infinite recursion.
	if (tag_name == "firststrike") {
		bool whom_is_defender = whom_is_self ? !is_attacker : is_attacker;
		if (whom_is_defender && att_weapon && att_weapon->has_special_or_ability("firststrike"))
			return false;
	}

	// Filter the units involved.
	//If filter concerns the unit on which special is applied,
	//then the type of special must be entered to avoid calling
	//the function of this special in matches_filter()
	//In apply_to=both case, tag_name must be checked in all filter because special applied to both self and opponent.
	bool applied_both = special["apply_to"] == "both";
	std::string self_check_if_recursion = (applied_both || whom_is_self) ? tag_name : "";
	if (!special_unit_matches(self, other, self_loc, self_attack, special, is_for_listing, filter_self, self_check_if_recursion))
		return false;
	std::string opp_check_if_recursion = (applied_both || !whom_is_self) ? tag_name : "";
	if (!special_unit_matches(other, self, other_loc, other_attack, special, is_for_listing, "filter_opponent", opp_check_if_recursion))
		return false;
	//in case of apply_to=attacker|defender, if both [filter_attacker] and [filter_defender] are used,
	//check what is_attacker is true(or false for (filter_defender]) in affect self case only is necessary for what unit affected by special has a tag_name check.
	bool applied_to_attacker = applied_both || (whom_is_self && is_attacker) || (!whom_is_self && !is_attacker);
	std::string att_check_if_recursion = applied_to_attacker ? tag_name : "";
	if (!special_unit_matches(att, def, att_loc, att_weapon, special, is_for_listing, "filter_attacker", att_check_if_recursion))
		return false;
	bool applied_to_defender = applied_both || (whom_is_self && !is_attacker) || (!whom_is_self && is_attacker);
	std::string def_check_if_recursion= applied_to_defender ? tag_name : "";
	if (!special_unit_matches(def, att, def_loc, def_weapon, special, is_for_listing, "filter_defender", def_check_if_recursion))
		return false;

	const auto adjacent = get_adjacent_tiles(self_loc);

	// Filter the adjacent units.
	for (const config &i : special.child_range("filter_adjacent"))
	{
		std::size_t count = 0;
		std::vector<map_location::direction> dirs = map_location::parse_directions(i["adjacent"]);
		unit_filter filter{ vconfig(i) };
		for (const map_location::direction index : dirs)
		{
			if (index == map_location::direction::indeterminate)
				continue;
			unit_map::const_iterator unit = units.find(adjacent[static_cast<int>(index)]);
			if (unit == units.end() || !filter.matches(*unit, adjacent[static_cast<int>(index)], *self))
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
		if (!in_ranges<int>(count, utils::parse_ranges_unsigned(i["count"].str()))) {
			return false;
		}
	}

	// Filter the adjacent locations.
	for (const config &i : special.child_range("filter_adjacent_location"))
	{
		std::size_t count = 0;
		std::vector<map_location::direction> dirs = map_location::parse_directions(i["adjacent"]);
		terrain_filter adj_filter(vconfig(i), resources::filter_con, false);
		for (const map_location::direction index : dirs)
		{
			if (index == map_location::direction::indeterminate)
				continue;
			if(!adj_filter.match(adjacent[static_cast<int>(index)])) {
				return false;
			}
			count++;
		}
		if (i["count"].empty() && count != dirs.size()) {
			return false;
		}
		if (!in_ranges<int>(count, utils::parse_ranges_unsigned(i["count"].str()))) {
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
	if (auto apply_filter = cfg.optional_child("filter_base_value")) {
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

effect::effect(const unit_ability_list& list, int def, const const_attack_ptr& att, EFFECTS wham) :
	effect_list_(),
	composite_value_(0)
{

	int value_set = (wham == EFFECT_CUMULABLE) ? std::max(list.highest("value").first, 0) + std::min(list.lowest("value").first, 0) : def;
	std::map<std::string,individual_effect> values_add;
	std::map<std::string,individual_effect> values_sub;
	std::map<std::string,individual_effect> values_mul;
	std::map<std::string,individual_effect> values_div;

	individual_effect set_effect_max;
	individual_effect set_effect_min;
	utils::optional<int> max_value = utils::nullopt;
	utils::optional<int> min_value = utils::nullopt;

	for (const unit_ability & ability : list) {
		const config& cfg = *ability.ability_cfg;
		const std::string& effect_id = cfg[cfg["id"].empty() ? "name" : "id"];

		if (!filter_base_matches(cfg, def))
			continue;

		if(wham != EFFECT_CUMULABLE){
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

		if(wham == EFFECT_DEFAULT || wham == EFFECT_CUMULABLE){
			if(cfg.has_attribute("max_value")){
				max_value = max_value ? std::min(*max_value, cfg["max_value"].to_int()) : cfg["max_value"].to_int();
			}
			if(cfg.has_attribute("min_value")){
				min_value = min_value ? std::max(*min_value, cfg["min_value"].to_int()) : cfg["min_value"].to_int();
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
			std::map<std::string,individual_effect>::iterator sub_effect = values_sub.find(effect_id);
			if(sub_effect == values_sub.end() || sub < sub_effect->second.value) {
				values_sub[effect_id].set(ADD, sub, ability.ability_cfg, ability.teacher_loc);
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

	if((wham != EFFECT_CUMULABLE) && set_effect_max.type != NOT_USED) {
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

	/* Additional and subtraction are independent since Wesnoth 1.19.4. Prior to that, they affected each other.
	 */
	int substraction = 0;
	for(const auto& val : values_sub) {
		substraction += val.second.value;
		effect_list_.push_back(val.second);
	}

	composite_value_ = static_cast<int>((value_set + addition + substraction) * multiplier / divisor);
	//clamp what if min_value < max_value or one attribute only used.
	if(max_value && min_value && *min_value < *max_value) {
		composite_value_ = std::clamp(*min_value, *max_value, composite_value_);
	} else if(max_value && !min_value) {
		composite_value_ = std::min(*max_value, composite_value_);
	} else if(min_value && !max_value) {
		composite_value_ = std::max(*min_value, composite_value_);
	}
}

} // end namespace unit_abilities
