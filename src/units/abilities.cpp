/*
	Copyright (C) 2006 - 2025
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

#include "deprecation.hpp"
#include "display.hpp"
#include "display_context.hpp"
#include "filter_context.hpp"
#include "font/standard_colors.hpp"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/function_gamestate.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_version.hpp" // for version_info
#include "gettext.hpp"
#include "language.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "serialization/markup.hpp"
#include "serialization/string_utils.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/types.hpp"
#include "units/abilities.hpp"
#include "units/ability_tags.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"
#include "utils/config_filters.hpp"
#include "units/unit.hpp"

#include <utility>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)

namespace
{
	using namespace std::string_literals;
	const std::array numeric_keys{
		"value"s, "add"s, "sub"s, "multiply"s, "divide"s, "max_value"s, "min_value"s
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

unit_ability_t::unit_ability_t(std::string tag, config cfg, bool inside_attack)
	: tag_(std::move(tag))
	, id_(cfg["id"].str())
	, in_specials_tag_(inside_attack)
	, active_on_(active_on_t::both)
	, apply_to_(apply_to_t::self)
	, affects_allies_(affects_allies_t::same_side_only)
	, affects_self_(true)
	, affects_enemies_(false)
	, priority_(cfg["priority"].to_double(0.00))
	, cfg_(std::move(cfg))
	, currently_checked_(false)
{
	do_compat_fixes(cfg_, tag_, inside_attack);

	if (tag_ != "resistance" && tag_ != "leadership") {
		std::string apply_to = cfg_["apply_to"].str();
		apply_to_ = apply_to == "attacker" ? apply_to_t::attacker :
			apply_to == "defender" ? apply_to_t::defender :
			apply_to == "self" ? apply_to_t::self :
			apply_to == "opponent" ? apply_to_t::opponent :
			apply_to == "both" ? apply_to_t::both :
			apply_to_t::self;

	}
	if (tag_ != "leadership") {
		std::string active_on = cfg_["active_on"].str();
		active_on_ = active_on == "defense" ? active_on_t::defense :
			active_on == "offense" ? active_on_t::offense :
			active_on_t::both;
	}
	if (!cfg_.has_child("affect_adjacent")) {
		//optimisation
		affects_allies_ = affects_allies_t::no;
	}
	if (cfg_["affect_allies"].to_bool(false)) {
		affects_allies_ = affects_allies_t::yes;
	}
	if (!cfg_["affect_allies"].to_bool(true)) {
		affects_allies_ = affects_allies_t::no;
	}
	affects_self_ = cfg_["affect_self"].to_bool(true);
	affects_enemies_ = cfg_["affect_enemies"].to_bool(false);
}

void unit_ability_t::do_compat_fixes(config& cfg, const std::string& tag, bool inside_attack)
{
	// replace deprecated backstab with formula
	if (!cfg["backstab"].blank()) {
		deprecated_message("backstab= in weapon specials", DEP_LEVEL::INDEFINITE, "", "Use [filter_opponent] with a formula instead; the code can be found in data/core/macros/ in the WEAPON_SPECIAL_BACKSTAB macro.");
	}
	if (cfg["backstab"].to_bool()) {
		const std::string& backstab_formula = "enemy_of(self, flanker) and not flanker.petrified where flanker = unit_at(direction_from(loc, other.facing))";
		config& filter_opponent = cfg.child_or_add("filter_opponent");
		config& filter_opponent2 = filter_opponent.empty() ? filter_opponent : filter_opponent.add_child("and");
		filter_opponent2["formula"] = backstab_formula;
	}
	cfg.remove_attribute("backstab");

	// replace deprecated filter_adjacent/filter_adjacent_location with formula
	std::string filter_teacher = inside_attack ? "filter_self" : "filter";
	if (cfg.has_child("filter_adjacent")) {
		if (inside_attack) {
			deprecated_message("[filter_adjacent]in weapon specials in [specials] tags", DEP_LEVEL::INDEFINITE, "", "Use [filter_self][filter_adjacent] instead.");
		}
		else {
			deprecated_message("[filter_adjacent] in abilities", DEP_LEVEL::INDEFINITE, "", "Use [filter][filter_adjacent] instead or other unit filter.");
		}
	}
	if (cfg.has_child("filter_adjacent_location")) {
		if (inside_attack) {
			deprecated_message("[filter_adjacent_location]in weapon specials in [specials] tags", DEP_LEVEL::INDEFINITE, "", "Use [filter_self][filter_location][filter_adjacent_location] instead.");
		}
		else {
			deprecated_message("[filter_adjacent_location] in abilities", DEP_LEVEL::INDEFINITE, "", "Use [filter][filter_location][filter_adjacent_location] instead.");
		}
	}

	//These tags are were never supported inside [specials] according to the wiki.
	for (config& filter_adjacent : cfg.child_range("filter_adjacent")) {
		if (filter_adjacent["count"].empty()) {
			//Previously count= behaved differenty in abilities.cpp and in filter.cpp according to the wiki
			deprecated_message("omitting count= in [filter_adjacent] in abilities", DEP_LEVEL::FOR_REMOVAL, version_info("1.21"), "specify count explicitly");
			filter_adjacent["count"] = map_location::parse_directions(filter_adjacent["adjacent"]).size();
		}
		cfg.child_or_add(filter_teacher).add_child("filter_adjacent", filter_adjacent);
	}
	cfg.remove_children("filter_adjacent");
	for (config& filter_adjacent : cfg.child_range("filter_adjacent_location")) {
		if (filter_adjacent["count"].empty()) {
			//Previously count= bahves differenty in abilities.cpp and in filter.cpp according to the wiki
			deprecated_message("omitting count= in [filter_adjacent_location] in abilities", DEP_LEVEL::FOR_REMOVAL, version_info("1.21"), "specify count explicitly");
			filter_adjacent["count"] = map_location::parse_directions(filter_adjacent["adjacent"]).size();
		}
		cfg.child_or_add(filter_teacher).add_child("filter_location").add_child("filter_adjacent_location", filter_adjacent);
	}
	cfg.remove_children("filter_adjacent_location");

	if (tag == "resistance" || tag == "leadership") {
		if (auto child = cfg.optional_child("filter_second_weapon")) {
			cfg.add_child("filter_opponent").add_child("filter_weapon", *child);
		}
		if (auto child = cfg.optional_child("filter_weapon")) {
			cfg.add_child("filter_student").add_child("filter_weapon", *child);
		}
		cfg.remove_children("filter_second_weapon");
		cfg.remove_children("filter_weapon");
	}
}


std::string unit_ability_t::get_help_topic_id(const config& cfg)
{
	// NOTE: neither ability names nor ability ids are necessarily unique. Creating
	// topics for either each unique name or each unique id means certain abilities
	// will be excluded from help. So... the ability topic ref id is a combination
	// of id and (untranslated) name. It's rather ugly, but it works.
	return cfg["id"].str() + cfg["name"].t_str().base_str();
}

std::string unit_ability_t::get_help_topic_id() const
{
	return id() + cfg()["name"].t_str().base_str();
}


void unit_ability_t::parse_vector(const config& abilities_cfg, ability_vector& res, bool inside_attack)
{
	for (auto item : abilities_cfg.all_children_range()) {
		res.push_back(unit_ability_t::create(item.key, item.cfg, inside_attack));
	}
}

ability_vector unit_ability_t::cfg_to_vector(const config& abilities_cfg, bool inside_attack)
{
	ability_vector res;
	parse_vector(abilities_cfg, res, inside_attack);
	return res;
}

ability_vector unit_ability_t::filter_tag(const ability_vector& abs, const std::string& tag)
{
	ability_vector res;
	for (const ability_ptr& p_ab : abs) {
		if (p_ab->tag() == tag) {
			res.push_back(p_ab);
		}
	}
	return res;
}

ability_vector unit_ability_t::clone(const ability_vector& abs)
{
	ability_vector res;
	for (const ability_ptr& p_ab : abs) {
		res.push_back(std::make_shared<unit_ability_t>(*p_ab));
	}
	return res;
}

config unit_ability_t::vector_to_cfg(const ability_vector& abilities)
{
	config abilities_cfg;
	for (const auto& item : abilities) {
		item->write(abilities_cfg);
	}
	return abilities_cfg;
}


void unit_ability_t::write(config& abilities_cfg)
{
	abilities_cfg.add_child(tag(), cfg());
}

std::string unit_ability_t::substitute_variables(const std::string& str) const {
	// TODO add more [specials] keys

	utils::string_map symbols;

	// [plague]type= -> $type
	if(tag() == "plague") {
		// Substitute [plague]type= as $type
		const auto iter = unit_types.types().find(cfg()["type"]);

		// TODO: warn if an invalid type is specified?
		if (iter == unit_types.types().end()) {
			return str;
		}

		const unit_type& type = iter->second;
		symbols.emplace("type", type.type_name());
	}

	// weapon specials with value keys, like value, add, sub etc.
	// i.e., [heals]value= -> $value, [regenerates]add= -> $add etc.
	for(const auto& vkey : numeric_keys) {
		if(cfg().has_attribute(vkey)) {
			if(vkey == "multiply" || vkey == "divide") {
				const std::string lang_locale = get_language().localename;
				std::stringstream formatter_str;
				try {
					formatter_str.imbue(std::locale{lang_locale});
				} catch(const std::runtime_error&) {}
				formatter_str << cfg()[vkey].to_double();
				symbols.emplace(vkey, formatter_str.str());
			} else {
				symbols.emplace(vkey, std::to_string(cfg()[vkey].to_int()));
			}
		}
	}

	return symbols.empty() ? str : utils::interpolate_variables_into_string(str, &symbols);
}


namespace {
	const config_attribute_value& get_attr_four_fallback(const config& cfg, bool b1, bool b2, std::string_view s_yes_yes, std::string_view s_yes_no, std::string_view s_no_yes, std::string_view s_no_no)
	{
		if (b1 && b2) {
			if (auto* attr = cfg.get(s_yes_yes)) { return *attr; }
		}
		if (b1) {
			if (auto* attr = cfg.get(s_yes_no)) { return *attr; }
		}
		if (b2) {
			if (auto* attr = cfg.get(s_no_yes)) { return *attr; }
		}
		return cfg[s_no_no];
	}
}

std::string unit_ability_t::get_name(bool is_inactive, unit_race::GENDER gender) const
{
	bool is_female = gender == unit_race::FEMALE;
	std::string res = get_attr_four_fallback(cfg_, is_inactive, is_female, "female_name_inactive", "name_inactive", "female_name", "name").str();
	return substitute_variables(res);
}

std::string unit_ability_t::get_description(bool is_inactive, unit_race::GENDER gender) const
{
	bool is_female = gender == unit_race::FEMALE;
	std::string res = get_attr_four_fallback(cfg_, is_inactive, is_female, "female_description_inactive", "description_inactive", "female_description", "description").str();
	return substitute_variables(res);
}

bool unit_ability_t::active_on_matches(bool student_is_attacker) const
{
	if (active_on() == unit_ability_t::active_on_t::both) {
		return true;
	}
	if (active_on() == unit_ability_t::active_on_t::offense && student_is_attacker) {
		return true;
	}
	if (active_on() == unit_ability_t::active_on_t::defense && !student_is_attacker) {
		return true;
	}
	return false;
}


unit_ability_t::recursion_guard::recursion_guard(const unit_ability_t& p)
	: parent()
{
	if (!p.currently_checked_) {
		p.currently_checked_ = true;
		parent = &p;
	}
}

unit_ability_t::recursion_guard::~recursion_guard()
{
	if (parent) {
		parent->currently_checked_ = false;
	}
}

unit_ability_t::recursion_guard::operator bool() const {
	return bool(parent);
}

unit_ability_t::recursion_guard unit_ability_t::guard_against_recursion(const unit& u) const
{
	if (currently_checked_) {
		static std::vector<std::tuple<std::string, std::string>> already_shown;

		auto identifier = std::tuple<std::string, std::string>{ u.id(), cfg().debug()};
		if (!utils::contains(already_shown, identifier)) {

			std::string_view filter_text_view = std::get<1>(identifier);
			utils::trim(filter_text_view);
			ERR_NG << "Looped recursion error for unit '" << u.id()
				<< "' while checking ability '" << filter_text_view << "'";

			// Arbitrary limit, just ensuring that having a huge number of specials causing recursion
			// warnings can't lead to unbounded memory consumption here.
			if (already_shown.size() > 100) {
				already_shown.clear();
			}
			already_shown.push_back(std::move(identifier));
		}
	}
	return recursion_guard(*this);
}




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
bool affects_side(const unit_ability_t& ab, std::size_t side, std::size_t other_side)
{
	const team& side_team = get_team(side);

	if(side == other_side)
		return ab.affects_allies() != unit_ability_t::affects_allies_t::no;
	if(side_team.is_enemy(other_side))
		return ab.affects_enemies();
	else
		return ab.affects_allies() == unit_ability_t::affects_allies_t::yes;
}

/**
 * This function defines in which direction loc is relative to from_loc
 * either by pointing to the hexagon loc is on or by pointing to the hexagon adjacent to from_loc closest to loc.
 */
int find_direction(const map_location& loc, const map_location& from_loc, std::size_t distance)
{
	const auto adjacent = get_adjacent_tiles(from_loc);
	for(std::size_t j = 0; j < adjacent.size(); ++j) {
		bool adj_or_dist = distance != 1 ? distance_between(adjacent[j], loc) == (distance - 1) : adjacent[j] == loc;
		if(adj_or_dist) {
			return j;
		}
	}
	return 0;
}

/// Helper function, to turn void retuning function into false retuning functions
/// Calls @a f with arguments @args, but if @f returns void this function returns false.
template<typename TFunc, typename... TArgs>
bool default_false(const TFunc& f, const TArgs&... args) {
	if constexpr (std::is_same_v<decltype(f(args...)), void>) {
		f(args...);
		return false;
	}
	else {
		return f(args...);
	}
}

template<typename TCheck, typename THandler>
bool foreach_distant_active_ability(const unit& un, const map_location& loc, TCheck&& quick_check, THandler&& handler)
{
	// If the unit does not have abilities that match the criteria, check if adjacent units or elsewhere on the map have active abilities
	// with the [affect_adjacent] subtag that could affect the unit.
	const unit_map& units = get_unit_map();

	// Check for each unit present on the map that it corresponds to the criteria
	// (possession of an ability with [affect_adjacent] via a boolean variable, not incapacitated,
	// different from the central unit, that the ability is of the right type, detailed verification of each ability),
	// if so return true.
	for(const unit& u : units) {
		//TODO: This currently doesn't use max_ability_radius_type, will be added back later.
		if (!u.max_ability_radius() || u.incapacitated() || u.underlying_id() == un.underlying_id()) {
			continue;
		}
		std::size_t max_ability_radius = u.max_ability_radius();
		const map_location& from_loc = u.get_location();
		std::size_t distance = distance_between(from_loc, loc);
		if (distance > max_ability_radius) {
			continue;
		}
		int dir = find_direction(loc, from_loc, distance);
		for (const auto& p_ab : u.abilities()) {
			if (!quick_check(p_ab)) {
				continue;
			}
			if (un.get_adj_ability_bool(*p_ab, distance, dir, loc, u, from_loc)) {
				if (default_false(handler, p_ab, u)) {
					return true;
				}
			}
		}
	}
	return false;
}

template<typename TCheck, typename THandler>
bool foreach_self_active_ability(const unit& un, map_location loc, const TCheck& quick_check, const THandler& handler)
{
	for (const auto& p_ab : un.abilities()) {
		if (!quick_check(p_ab)) {
			continue;
		}
		if (un.get_self_ability_bool(*p_ab, loc)) {
			if (default_false(handler, p_ab, un)) {
				return true;
			}
		}
	}
	return false;
}

// enum class loop_type_t { self_only, distant_only, both };

/*
 * execeutes a given function for each active ability of @a unit, including
 * abilitied thought by other units
 * @param un the unit receiving the abilities
 * @param loc the location we assume the unit to be at.
 * @param quick_check a quick check that is exceuted before the ability tested
 * @param handler the function that is called for each acive ability.
 *        if this is a boolean function and returns true the execeution
 *        is aborted, used for "have any active ability"-like checks.
 * @returns true iff any of the handlers returned true.
 */
template<typename TCheck, typename THandler>
bool foreach_active_ability(const unit& un, map_location loc, const TCheck& quick_check, const THandler& handler, bool skip_adjacent = false)
{
	// Check that the unit has an ability of tag_name type which meets the conditions to be active.
	// If so, return true.
	if (foreach_self_active_ability(un, loc, quick_check, handler)) {
		return true;
	}
	if (!skip_adjacent && foreach_distant_active_ability(un, loc, quick_check, handler)) {
		return true;
	}
	return false;
}

auto return_true = [](const auto&...) { return true; };

/*
 * executes the given handler for each active special/ability affecting an attack during combat.
 * a simple_check parameter can be as a predicate to filter out the abilities/specials that we
 * are not interested in, simple_check is executed before checking whether a given ability is active.
 * the @a skip_adjacent parameter can be set to true if we are not intereted in abilities from
 * adjacent units, used by the [filter_special] code as an optimisation.
 * @param context the abilities context we are in, describing attacker defender etc.
 * @param self the combatant whose weapons are affected by the specials
 * @param quick_check a predicate describing in whiich abilities we are interested in. (usually checking for tag name)
 * @handler handler a callback that is executed for each active speical that affects @self in the current context, it takes 3 parameters:
 *          - a const ability_t& describing the ability
 *          - a specials_combatant& student
 *          - a 'source', this is either am attack_type& or a unit&. so handler operator() has to be able to handle both cases
 *          handler can return a bool, if it returns true, the seacrch is aborted and this functions returns true withaout checking mroe abilities.
 * @skip_adjacent whether we should skip looking into adjacent units effecting the weapon via leadership-like abilities. (used by the [filter_special] as a optimisation)
 */
template<typename TCheck, typename THandler>
bool foreach_active_special(
	const specials_context_t& context,
	const specials_context_t::specials_combatant& self,
	const TCheck& quick_check,
	const THandler& handler,
	bool skip_adjacent = false)
{
	auto& other = context.other(self);
	// "const auto&..." because foreach_active_ability calls this with a unit& argument.
	auto handler_self = [&](const ability_ptr& p_ab, const auto& source) {
		return context.is_special_active(self, *p_ab, unit_ability_t::affects_t::SELF) && default_false(handler, p_ab, self, source);
	};
	auto handler_other = [&](const ability_ptr& p_ab, const auto& source) {
		return context.is_special_active(other, *p_ab, unit_ability_t::affects_t::OTHER) && default_false(handler, p_ab, other, source);
	};

	//search in the attacks [specials]
	if (self.at) {
		for (const ability_ptr& p_ab : self.at->specials()) {
			if (quick_check(p_ab) && handler_self(p_ab, *self.at)) {
				return true;
			}
		}
	}
	//search in the opponents attacks [specials]
	if (other.at) {
		for (const ability_ptr& p_ab : other.at->specials()) {
			if (quick_check(p_ab) && handler_other(p_ab, *other.at)) {
				return true;
			}
		}
	}
	//search in unit [abilities] including abilities tought via loadship like abilities.
	if (self.un) {
		if (foreach_active_ability(*self.un, self.loc, quick_check, handler_self, skip_adjacent)) {
			return true;
		}
	}
	//search in the opponents [abilities] including abilities tought via loadship like abilities.
	if (other.un) {
		if (foreach_active_ability(*other.un, other.loc, quick_check, handler_other, skip_adjacent)) {
			return true;
		}
	}
	return false;
}


}

bool unit::get_ability_bool(const std::string& tag_name, const map_location& loc) const
{
	return foreach_active_ability(*this, loc,
		[&](const ability_ptr& p_ab) {
			return p_ab->tag() == tag_name;
		},
		[&](const ability_ptr&, const unit&) {
			return true;
		});
}

active_ability_list unit::get_abilities(const std::string& tag_name, const map_location& loc) const
{
	active_ability_list res(loc_);
	foreach_active_ability(*this, loc,
		[&](const ability_ptr& p_ab) {
			return p_ab->tag() == tag_name;
		},
		[&](const ability_ptr& p_ab, const unit& u2) {
			res.emplace_back(p_ab, loc, u2.get_location());
		});
	return res;
}


std::vector<std::string> unit::get_ability_id_list() const
{
	std::vector<std::string> res;

	for(const auto& p_ab : this->abilities()) {
		std::string id = p_ab->id();
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
	bool add_ability_tooltip(const unit_ability_t& ab, unit_race::GENDER gender, std::vector<unit_ability_t::tooltip_info>& res, bool active)
	{
		auto name = ab.get_name(!active, gender);
		auto desc = ab.get_description(!active, gender);

		if (name.empty()) {
			return false;
		}

		res.AGGREGATE_EMPLACE(
			name,
			desc,
			ab.get_help_topic_id()
		);
		return true;
	}
}

std::vector<unit_ability_t::tooltip_info> unit::ability_tooltips() const
{
	std::vector<unit_ability_t::tooltip_info> res;

	for(const auto& p_ab : abilities())
	{
		add_ability_tooltip(*p_ab, gender_, res, true);
	}

	return res;
}

std::vector<unit_ability_t::tooltip_info> unit::ability_tooltips(boost::dynamic_bitset<>& active_list, const map_location& loc) const
{
	std::vector<unit_ability_t::tooltip_info> res;
	active_list.clear();

	for(const auto& p_ab : abilities())
	{
		bool active = ability_active(*p_ab, loc);
		if(add_ability_tooltip(*p_ab, gender_, res, active))
		{
			active_list.push_back(active);
		}
	}
	return res;
}


bool unit::ability_active(const unit_ability_t& ab, const map_location& loc) const
{
	auto filter_lock = ab.guard_against_recursion(*this);
	if(!filter_lock) {
		return false;
	}
	return ability_active_impl(ab, loc);
}

bool unit::ability_active_impl(const unit_ability_t& ab,const map_location& loc) const
{
	bool illuminates = ab.tag() == "illuminates";

	if(auto afilter = ab.cfg().optional_child("filter")) {
		if(!unit_filter(vconfig(*afilter)).set_use_flat_tod(illuminates).matches(*this, loc)) {
			return false;
		}
	}

	return true;
}

bool unit::ability_affects_adjacent(const unit_ability_t& ab, std::size_t dist, int dir, const map_location& loc, const unit& from) const
{
	if(!ab.cfg().has_child("affect_adjacent")) {
		return false;
	}
	bool illuminates = ab.tag() == "illuminates";

	assert(dir >=0 && dir <= 5);
	map_location::direction direction{ dir };

	for (const config &i : ab.cfg().child_range("affect_adjacent"))
	{
		if(i["radius"] != "all_map") {
			int radius = i["radius"].to_int(1);
			if(radius <= 0) {
				continue;
			}
			if(dist > size_t(radius)) {
				continue;
			}
		}
		if (i.has_attribute("adjacent")) { //key adjacent defined
			if(!utils::contains(map_location::parse_directions(i["adjacent"]), direction)) {
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

bool unit::ability_affects_self(const unit_ability_t& ab, const map_location& loc) const
{
	auto filter = ab.cfg().optional_child("filter_self");
	bool affect_self = ab.affects_self();
	if (!filter || !affect_self) return affect_self;
	return unit_filter(vconfig(*filter)).set_use_flat_tod(ab.tag() == "illuminates").matches(*this, loc);
}

bool unit::has_ability_type(const std::string& ability) const
{
	return !abilities(ability).empty();
}

//these two functions below are used in order to add to the unit
//a second set of halo encoded in the abilities (like illuminates halo in [illuminates] ability for example)
static void add_string_to_vector(std::vector<std::string>& image_list, const config& cfg, const std::string& attribute_name)
{
	if(!utils::contains(image_list, cfg[attribute_name].str())) {
		image_list.push_back(cfg[attribute_name].str());
	}
}

std::vector<std::string> unit::halo_or_icon_abilities(const std::string& image_type) const
{
	std::vector<std::string> image_list;
	for(const auto& p_ab : abilities()){
		bool is_active = ability_active(*p_ab, loc_);
		//Add halo/overlay to owner of ability if active and affect_self is true.
		if( !p_ab->cfg()[image_type + "_image"].str().empty() && is_active && ability_affects_self(*p_ab, loc_)){
			add_string_to_vector(image_list, p_ab->cfg(), image_type + "_image");
		}
		//Add halo/overlay to owner of ability who affect adjacent only if active.
		if(!p_ab->cfg()[image_type + "_image_self"].str().empty() && is_active){
			add_string_to_vector(image_list, p_ab->cfg(), image_type + "_image_self");
		}
	}

	foreach_distant_active_ability(*this, loc_,
		[&](const ability_ptr& p_ab) {
			return !p_ab->cfg()[image_type + "_image"].str().empty();
		},
		[&](const ability_ptr& p_ab, const unit&) {
			add_string_to_vector(image_list, p_ab->cfg(), image_type + "_image");
		});

	//rearranges vector alphabetically when its size equals or exceeds two.
	if(image_list.size() >= 2){
		std::sort(image_list.begin(), image_list.end());
	}
	return image_list;
}

void attack_type::add_formula_context(wfl::map_formula_callable& callable) const
{
	//TODO: remove this function and make the caller use specials_context_t::add_formula_context directly.
	if (context_) {
		context_->add_formula_context(callable);
	}
}

void specials_context_t::add_formula_context(wfl::map_formula_callable& callable) const
{
	if(const unit_const_ptr & att = attacker.un) {
		callable.add("attacker", wfl::variant(std::make_shared<wfl::unit_callable>(*att)));
	}
	if(const unit_const_ptr & def = defender.un) {
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
T get_single_ability_value(const config::attribute_value& v, T def, const active_ability& ability_info, const map_location& receiver_loc, const const_attack_ptr& att, const TFuncFormula& formula_handler)
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
				return formula_handler(wfl::formula(s, new wfl::gamestate_function_symbol_table, true), callable);
			} catch(const wfl::formula_error& e) {
				lg::log_to_chat() << "Formula error in ability or weapon special: " << e.type << " at " << e.filename << ':' << e.line << ")\n";
				ERR_WML << "Formula error in ability or weapon special: " << e.type << " at " << e.filename << ':' << e.line << ")";
				return def;
			}
	}));
}
}

template<typename TComp>
std::pair<int,map_location> active_ability_list::get_extremum(const std::string& key, int def, const TComp& comp) const
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
	for (const active_ability& p : cfgs_)
	{
		int value = std::round(get_single_ability_value(p.ability_cfg()[key], static_cast<double>(def), p, loc(), const_attack_ptr(), [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
			return std::round(formula.evaluate(callable).as_int());
		}));

		if (p.ability_cfg()["cumulative"].to_bool()) {
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

template std::pair<int, map_location> active_ability_list::get_extremum<std::less<int>>(const std::string& key, int def, const std::less<int>& comp) const;
template std::pair<int, map_location> active_ability_list::get_extremum<std::greater<int>>(const std::string& key, int def, const std::greater<int>& comp) const;

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

specials_context_t::specials_context_t(specials_combatant&& att, specials_combatant&& def)
	: attacker(std::move(att))
	, defender(std::move(def))
{
	if (attacker.at) {
		attacker.at->context_ = this;
	}
	if (defender.at) {
		defender.at->context_ = this;
	}
}

specials_context_t::~specials_context_t()
{
	if (attacker.at) {
		attacker.at->context_ = nullptr;
	}
	if (defender.at) {
		defender.at->context_ = nullptr;
	}
}


static bool is_enemy(std::size_t side, std::size_t other_side)
{
	const team& side_team = get_team(side);
	return side_team.is_enemy(other_side);
}

/**
 * Returns a comma-separated string of active names for the specials of *this.
 * Empty names are skipped.
 *
 * Whether or not a special is active depends
 * on the current context (see set_specials_context)
 */
std::string specials_context_t::describe_weapon_specials(const attack_type& at) const
{
	auto s_a_o = self_and_other(at);
	const auto& [self, other] = s_a_o;

	std::vector<std::string> special_names;
	std::set<std::string> ability_names;

	for(const auto& p_ab : at.specials()) {
		const bool active = is_special_active(self, *p_ab, unit_ability_t::affects_t::EITHER);
		std::string name = p_ab->get_name(!active);
		if(!name.empty()) {
			special_names.push_back(active ? std::move(name) : markup::span_color(font::INACTIVE_COLOR, name));
		}
	}

	// FIXME: clean this up...

	if (self.un) {
		const std::set<std::string>& checking_tags = abilities_list::all_weapon_tags();
		auto quick_check = [&](const ability_ptr& p_ab) {
			return checking_tags.count(p_ab->tag()) != 0;
		};

		foreach_active_ability(*self.un, self.loc, quick_check,
			[&](const ability_ptr& p_ab, const unit& source) {
				if (is_enemy(source.side(), s_a_o.self.un->side())) {
					return;
				}
				if (!is_special_active(s_a_o.self, *p_ab, unit_ability_t::affects_t::SELF)) {
					return;
				}
				const std::string& name_affected = p_ab->cfg().get_or("name_affected", "name").str();
				ability_names.insert(p_ab->substitute_variables(name_affected));
			});
	}

	if(!ability_names.empty()) {
		special_names.push_back("\n" + utils::join(ability_names, ", "));
	}

	return utils::join(special_names, ", ");
}

std::string specials_context_t::describe_weapon_specials_value(const attack_type& at, const std::set<std::string>& checking_tags) const
{
	auto s_a_o = self_and_other(at);
	const auto& [self, other] = s_a_o;

	std::string res;

	std::set<std::string> wespon_specials;
	std::set<std::string> abilities_self;
	std::set<std::string> abilities_allies;
	std::set<std::string> abilities_enemies;
	std::set<std::string> opponents_abilities;

	auto quick_check = [&](const ability_ptr& p_ab) {
		return checking_tags.count(p_ab->tag()) != 0;
	};

	auto add_to_list = [&](const ability_ptr& p_ab, const specials_combatant& student, const auto& source) {
		if (&student == &s_a_o.other) {
			opponents_abilities.insert(p_ab->substitute_variables(p_ab->cfg()["name"].str()));
		} else if constexpr (utils::decayed_is_same<decltype(source), attack_type>) {
			wespon_specials.insert(p_ab->substitute_variables(p_ab->cfg()["name"].str()));
		} else if (&source == s_a_o.self.un.get()) {
			const std::string& name_affected = p_ab->cfg().get_or("name_affected", "name").str();
			abilities_self.insert(p_ab->substitute_variables(name_affected));
		} else if (!is_enemy(source.side(), s_a_o.self.un->side())) {
			const std::string& name_affected = p_ab->cfg().get_or("name_affected", "name").str();
			abilities_allies.insert(p_ab->substitute_variables(name_affected));
		} else {
			const std::string& name_affected = p_ab->cfg().get_or("name_affected", "name").str();
			abilities_enemies.insert(p_ab->substitute_variables(name_affected));
		}
	};

	auto add_to_res = [&](std::set<std::string>& to_add, const std::string& category_name) {
		to_add.erase("");
		if (!to_add.empty()) {
			//TODO: markup::span_color(font::TITLE_COLOR) ??
			res += (res.empty() ? "\n" : "") + category_name + utils::join(to_add, ", ");
		}
	};


	foreach_active_special(*this, self, quick_check, add_to_list);

	add_to_res(wespon_specials, "");
	add_to_res(abilities_self, _("Owned: "));
	// TRANSLATORS: Past-participle of "teach", used for an ability similar to leadership
	add_to_res(abilities_allies, _("Taught: "));
	// TRANSLATORS: Past-participle of "teach", used for an ability similar to leadership
	add_to_res(abilities_enemies, _("Taught: (by an enemy): "));
	add_to_res(opponents_abilities, _("Used by opponent: "));

	return res;
}


namespace { // Helpers for attack_type::special_active()

	/**
	 * Returns whether or not the given special affects the opponent of the unit
	 * with the special.
	 * @param ab                the ability/special
	 * @param[in]  is_attacker  whether or not the unit with the special is the attacker
	 */
	bool special_affects_opponent(const unit_ability_t& ab, bool is_attacker)
	{
		using apply_to_t = unit_ability_t::apply_to_t;
		const auto apply_to = ab.apply_to();
		if ( apply_to == apply_to_t::both)
			return true;
		if ( apply_to == apply_to_t::opponent )
			return true;
		if ( is_attacker  && apply_to == apply_to_t::defender)
			return true;
		if ( !is_attacker && apply_to == apply_to_t::attacker)
			return true;
		return false;
	}

	/**
	 * Returns whether or not the given special affects the unit with the special.
	 * @param ab                the ability/special
	 * @param[in]  is_attacker  whether or not the unit with the special is the attacker
	 */
	bool special_affects_self(const unit_ability_t& ab, bool is_attacker)
	{
		using apply_to_t = unit_ability_t::apply_to_t;
		const auto apply_to = ab.apply_to();
		if ( apply_to == apply_to_t::both )
			return true;
		if ( apply_to == apply_to_t::self)
			return true;
		if ( is_attacker  &&  apply_to == apply_to_t::attacker)
			return true;
		if ( !is_attacker &&  apply_to == apply_to_t::defender)
			return true;
		return false;
	}

	static bool buildin_is_immune(const unit_ability_t& ab, const unit_const_ptr& them, map_location their_loc)
	{
		if (ab.tag() == "drains" && them && them->get_state("undrainable")) {
			return true;
		}
		if (ab.tag() == "plague" && them &&
			(them->get_state("unplagueable") ||
				resources::gameboard->map().is_village(their_loc))) {
			return true;
		}
		if (ab.tag() == "poison" && them &&
			(them->get_state("unpoisonable") || them->get_state(unit::STATE_POISONED))) {
			return true;
		}
		if (ab.tag() == "slow" && them &&
			(them->get_state("unslowable") || them->get_state(unit::STATE_SLOWED))) {
			return true;
		}
		if (ab.tag() == "petrifies" && them &&
			them->get_state("unpetrifiable")) {
			return true;
		}
		return false;
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
	 * @param[in]  applies_to_checked    Parameter used for don't have infinite recusion for some filter attribute.
	 */
	static bool special_unit_matches(const unit_const_ptr & u,
		                             const unit_const_ptr & u2,
		                             const map_location & loc,
		                             const const_attack_ptr& weapon,
		                             const unit_ability_t& ab,
									 const bool for_listing,
		                             const std::string & child_tag, bool applies_to_checked)
	{
		if (for_listing && !loc.valid())
			// The special's context was set to ignore this unit, so assume we pass.
			// (This is used by reports.cpp to show active specials when the
			// opponent is not known. From a player's perspective, the special
			// is active, in that it can be used, even though the player might
			// need to select an appropriate opponent.)
			return true;

		const config& filter = ab.cfg();
		const config& filter_backstab = filter;

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


		auto filter_lock = ab.guard_against_recursion(*u);
		if(!filter_lock) {
			return false;
		}
		// Check for a weapon match.
		if (auto filter_weapon = filter_child->optional_child("filter_weapon") ) {
			std::string check_if_recursion = applies_to_checked ? ab.tag() : "";
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


/**
 * Returns a vector of names and descriptions for the specials of *this.
 * Each std::pair in the vector has first = name and second = description.
 *
 * This uses either the active or inactive name/description for each special,
 * based on the current context (see set_specials_context), provided
 * @a active_list is not nullptr. Otherwise specials are assumed active.
 * If the appropriate name is empty, the special is skipped.
 */
std::vector<unit_ability_t::tooltip_info> specials_context_t::special_tooltips(const attack_type& at,
	boost::dynamic_bitset<>& active_list) const
{
	//log_scope("special_tooltips");
	auto [self, other] = self_and_other(at);

	std::vector<unit_ability_t::tooltip_info> res;
	active_list.clear();

	for (const auto& p_ab : self.at->specials()) {
		bool active = is_special_active(self, *p_ab, unit_ability_t::affects_t::EITHER);
		auto name = p_ab->get_name(!active);
		auto desc = p_ab->get_description(!active);

		if (name.empty()) {
			continue;
		}

		res.AGGREGATE_EMPLACE(
			name,
			desc,
			p_ab->get_help_topic_id()
		);

		active_list.push_back(active);
	}
	return res;
}

namespace {
	/**
	 * Returns whether or not the given special is active for the specified unit disregarding other units,
	 * based on the current context (see specials_context).
	 * @param ab                the ability/special
	 */
	bool special_tooltip_active(const specials_context_t& context, const specials_context_t::specials_combatant& self, const unit_ability_t& ab)
	{
		bool is_for_listing = context.is_for_listing;

		auto& other = context.other(self);
		bool is_attacker = &self == &context.attacker;
		//log_scope("special_tooltip_active");

		//here 'active_on' and checking of opponent weapon shouldn't implemented
		//because other_attack_ don't exist in sidebar display.
		//'apply_to' and some filters like [filter_student] are checked for know if
		//special must be displayed in sidebar.

		//only special who affect self are valid here.
		bool whom_is_self = special_affects_self(ab, is_attacker);
		if (!whom_is_self)
			return false;

		//this part of checking is similar to special_active but not the same.
		//"filter_opponent" is not checked here, and "filter_attacker/defender" only
		//if attacker/defender is self_.
		bool applied_both = ab.apply_to() == unit_ability_t::apply_to_t::both;

		if (!special_unit_matches(self.un, other.un, self.loc, self.at, ab, is_for_listing, "filter_student", applied_both || whom_is_self))
			return false;
		bool applied_to_attacker = applied_both || (whom_is_self && is_attacker);
		if (is_attacker && !special_unit_matches(self.un, other.un, self.loc, self.at, ab, is_for_listing, "filter_attacker", applied_to_attacker))
			return false;
		bool applied_to_defender = applied_both || (whom_is_self && !is_attacker);
		if (!is_attacker && !special_unit_matches(self.un, other.un, self.loc, self.at, ab, is_for_listing, "filter_defender", applied_to_defender))
			return false;

		return true;
	}

}


std::vector<unit_ability_t::tooltip_info> specials_context_t::abilities_special_tooltips(const attack_type& at,
	boost::dynamic_bitset<>& active_list) const
{
	auto s_a_o = self_and_other(at);
	const auto& [self, other] = s_a_o;

	std::vector<unit_ability_t::tooltip_info> res;
	active_list.clear();
	std::set<std::string> checking_name;
	if (!self.un) {
		return res;
	}
	foreach_active_ability(*self.un, self.loc,
		[&](const ability_ptr&) {
			return true;
		},
		[&](const ability_ptr& p_ab, const unit&) {
			if (special_tooltip_active(*this, s_a_o.self, *p_ab)) {
				bool active = is_special_active(s_a_o.self, *p_ab, unit_ability_t::affects_t::SELF);
				const std::string name = p_ab->substitute_variables(p_ab->cfg()["name_affected"]);
				const std::string desc = p_ab->substitute_variables(p_ab->cfg()["description_affected"]);

				if (name.empty() || checking_name.count(name) != 0) {
					return;
				}
				res.AGGREGATE_EMPLACE(name, desc, p_ab->get_help_topic_id());
				checking_name.insert(name);
				active_list.push_back(active);
			}
		});
	return res;
}


//The following functions are intended to allow the use in combat of capacities
//identical to special weapons and therefore to be able to use them on adjacent
//units (abilities of type 'aura') or else on all types of weapons even if the
//beneficiary unit does not have a corresponding weapon
//(defense against ranged weapons abilities for a unit that only has melee attacks)
static bool overwrite_special_affects(const unit_ability_t& ab)
{
	const std::string& apply_to = ab.cfg()["overwrite_specials"];
	return (apply_to == "one_side" || apply_to == "both_sides");
}

active_ability_list attack_type::overwrite_special_overwriter(active_ability_list overwriters) const
{
	//remove element without overwrite_specials key, if list empty after check return empty list.
	utils::erase_if(overwriters, [&](const active_ability& i) {
		return (!overwrite_special_affects(i.ability()));
	});

	// if empty, nothing is doing any overwriting
	if(overwriters.empty()){
		return overwriters;
	}

	// if there are specials/"specials as abilities" that could potentially overwrite each other
	if(overwriters.size() >= 2){
		// sort them by overwrite priority from highest to lowest (default priority is 0)
		utils::sort_if(overwriters,[](const active_ability& i, const active_ability& j){
			auto oi = i.ability_cfg().optional_child("overwrite");
			double l = 0;
			if(oi && !oi["priority"].empty()){
				l = oi["priority"].to_double(0);
			}
			auto oj = j.ability_cfg().optional_child("overwrite");
			double r = 0;
			if(oj && !oj["priority"].empty()){
				r = oj["priority"].to_double(0);
			}
			return l > r;
		});
		// remove any that need to be overwritten
		utils::erase_if(overwriters, [&](const active_ability& i) {
			return (overwrite_special_checking(overwriters, i));
		});
	}
	return overwriters;
}

bool attack_type::overwrite_special_checking(active_ability_list& overwriters, const active_ability& i) const
{
	auto ctx = fallback_context();
	auto [self, other] = context_->self_and_other(*this);
	if(overwriters.empty()){
		return false;
	}

	const unit_ability_t& ab = i.ability();
	const map_location& loc = i.student_loc;

	for(const auto& j : overwriters) {
		const map_location& ov_loc = j.student_loc;
		// whether the overwriter affects a single side
		bool affect_side = (j.ability_cfg()["overwrite_specials"] == "one_side");
		// the overwriter's priority, default of 0
		auto overwrite_specials = j.ability_cfg().optional_child("overwrite");
		double priority = overwrite_specials ? overwrite_specials["priority"].to_double(0) : 0.00;
		// the cfg being checked for whether it will be overwritten
		auto has_overwrite_specials = ab.cfg().optional_child("overwrite");
		// if the overwriter's priority is greater than 0, then true if the cfg being checked has a higher priority
		// else true
		bool prior = (priority > 0) ? (has_overwrite_specials && has_overwrite_specials["priority"].to_double(0) >= priority) : true;
		// true if the cfg being checked affects one or both sides and doesn't have a higher priority, or if it doesn't affect one or both sides
		// aka whether the cfg being checked can potentially be overwritten by the current overwriter
		bool is_overwritable = (overwrite_special_affects(ab) && !prior) || !overwrite_special_affects(ab);
		bool one_side_overwritable = true;

		// if the current overwriter affects one side and the cfg being checked can be overwritten by this overwriter
		// then check that the current overwriter and the cfg being checked both affect either this unit or its opponent
		if(affect_side && is_overwritable) {
			if(ov_loc == self.loc) {
				one_side_overwritable = loc == self.loc;
			}
			else if(other.un && (ov_loc == other.loc)) {
				one_side_overwritable = loc == other.loc;
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
				special_matches = ab.matches_filter(*overwrite_filter);
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

bool unit::get_self_ability_bool(const unit_ability_t& ab, const map_location& loc) const
{
	auto filter_lock = ab.guard_against_recursion(*this);
	if(!filter_lock) {
		return false;
	}
	return (ability_active_impl(ab, loc) && ability_affects_self(ab, loc));
}

bool unit::get_adj_ability_bool(const unit_ability_t& ab, std::size_t dist, int dir, const map_location& loc, const unit& from, const map_location& from_loc) const
{
	auto filter_lock = ab.guard_against_recursion(from);;
	if(!filter_lock) {
		return false;
	}
	return (affects_side(ab, side(), from.side()) && from.ability_active_impl(ab, from_loc) && ability_affects_adjacent(ab, dist, dir, loc, from));
}

/**
 * Returns whether or not @a *this has a special ability with a tag or id equal to
 * @a special. the Check is for a special ability
 * active in the current context (see set_specials_context), including
 * specials obtained from the opponent's attack.
 */
bool specials_context_t::has_active_special(const attack_type & at, const std::string & tag_name) const
{

	auto quick_check = [&](const ability_ptr& p_ab) {
		return p_ab->tag() == tag_name;
	};

	auto [self, other] = self_and_other(at);
	return foreach_active_special(*this, self, quick_check, return_true);
}

bool specials_context_t::has_active_special_id(const attack_type& at, const std::string& special_id) const
{
	//Now that filter_(second)attack in event supports special_id/type_active, including abilities used as weapons,
	//these can be detected even in placeholder attacks generated to compensate for the lack of attack in defense against an attacker using a range attack not possessed by the defender.
	//It is therefore necessary to check if the range is not empty (proof that the weapon is not a placeholder) to decide if has_weapon_ability can be returned or not.
	if (at.range().empty()) {
		return false;
	}


	auto quick_check = [&](const ability_ptr& p_ab) {
		return p_ab->id() == special_id;
	};

	auto [self, other] = self_and_other(at);
	return foreach_active_special(*this, self, quick_check, return_true);
}

active_ability_list specials_context_t::get_active_combat_teachers(const attack_type& at) const
{
	auto s_a_o = self_and_other(at);
	const auto& [self, other] = s_a_o;

	const map_location loc = self.un ? self.un->get_location() : self.loc;
	active_ability_list res(loc);
	const std::set<std::string>& checking_tags = abilities_list::all_weapon_tags();
	auto quick_check = [&](const ability_ptr& p_ab) {
		return checking_tags.count(p_ab->tag()) != 0;
	};

	if (self.un) {
		foreach_distant_active_ability(*self.un, self.loc, quick_check,
			[&](const ability_ptr& p_ab, const unit& u_teacher) {
				if (is_special_active(s_a_o.self, *p_ab, unit_ability_t::affects_t::SELF)) {
					res.emplace_back(p_ab, s_a_o.self.un->get_location(), u_teacher.get_location());
				}
			}
		);
	}
	if (other.un) {
		foreach_distant_active_ability(*other.un, other.loc, quick_check,
			[&](const ability_ptr& p_ab, const unit& u_teacher) {
				if (is_special_active(s_a_o.other, *p_ab, unit_ability_t::affects_t::OTHER)) {
					res.emplace_back(p_ab, s_a_o.other.un->get_location(), u_teacher.get_location());
				}
			}
		);
	}
	return res;
}

active_ability_list specials_context_t::get_active_specials(const attack_type& at, const std::string& tag_name) const
{
	auto [self, other] = self_and_other(at);
	const map_location loc = self.un ? self.un->get_location() : self.loc;
	active_ability_list res(loc);

	auto quick_check = [&](const ability_ptr& p_ab) {
		return p_ab->tag() == tag_name;
	};

	auto add_to_list = utils::overload {
		[&](const ability_ptr& p_ab, const specials_combatant& student, const attack_type&) {
			res.emplace_back(p_ab, student.loc, student.loc);
		},
		[&](const ability_ptr& p_ab, const specials_combatant& student, const unit& source) {
			res.emplace_back(p_ab, student.un->get_location(), source.get_location());
		}
	};


	foreach_active_special(*this, self, quick_check, add_to_list);
	return res;
}

active_ability_list specials_context_t::get_abilities_weapons(const std::string& tag_name, const unit& un) const
{
	auto s_a_o = self_and_other(un);
	const auto& [self, other] = s_a_o;
	//TODO: fall back to un.get_location() ?
	active_ability_list res = un.get_abilities(tag_name, self.loc);

	utils::erase_if(res, [&](const active_ability& i) {
		//If no weapon is given, assume the ability is active. this is used by ai code.
		return !is_special_active(s_a_o.self, i.ability(), unit_ability_t::affects_t::SELF);
		});
	return res;

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

		if(filter.has_attribute("overwrite_specials") && abilities_list::weapon_math_tags().count(tag_name) == 0)
			return false;

		bool no_value_weapon_abilities_check =  abilities_list::no_weapon_math_tags().count(tag_name) != 0 || abilities_list::ability_no_value_tags().count(tag_name) != 0;
		if(filter.has_attribute("cumulative") && no_value_weapon_abilities_check && (tag_name != "swarm" || tag_name != "berserk"))
			return false;
		if(filter.has_attribute("value") && (no_value_weapon_abilities_check && tag_name != "berserk"))
			return false;
		if(filter.has_attribute("add") && no_value_weapon_abilities_check)
			return false;
		if(filter.has_attribute("sub") && no_value_weapon_abilities_check)
			return false;
		if(filter.has_attribute("multiply") && no_value_weapon_abilities_check)
			return false;
		if(filter.has_attribute("divide") && no_value_weapon_abilities_check)
			return false;
		if(filter.has_attribute("priority") && no_value_weapon_abilities_check)
			return false;

		bool all_engine =  abilities_list::no_weapon_math_tags().count(tag_name) != 0 || abilities_list::weapon_math_tags().count(tag_name) != 0 || abilities_list::ability_value_tags().count(tag_name) != 0 || abilities_list::ability_no_value_tags().count(tag_name) != 0;
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
		if(!filter_type.empty() && !utils::contains(filter_type, tag_name))
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

		if(abilities_list::weapon_math_tags().count(tag_name) != 0 || abilities_list::ability_value_tags().count(tag_name) != 0) {
			if(!double_matches_if_present(filter, cfg, "priority", 0.00)) {
				return false;
			}
		} else {
			if(!double_matches_if_present(filter, cfg, "priority")) {
				return false;
			}
		}

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

bool unit_ability_t::matches_filter(const config & filter) const
{
	return common_matches_filter(cfg(), tag(), filter);
}

bool specials_context_t::has_active_special_matching_filter(const attack_type& at, const config & filter) const
{
	if(at.range().empty()){
		return false;
	}

	bool skip_adjacent = !filter["affect_adjacent"].to_bool(true);

	auto quick_check = [&](const ability_ptr& p_ab) {
		return p_ab->matches_filter(filter);
	};

	auto [self, other] = self_and_other(at);
	return foreach_active_special(*this, self, quick_check, return_true, skip_adjacent);
}

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
/**
 * Returns whether or not the given special is active for the specified unit,
 * based on the current context (see set_specials_context).
 * @param self              this combatant
 * @param ab                the ability
 * @param whom              specifies which combatant we care about
 */
bool specials_context_t::is_special_active(const specials_combatant& self, const unit_ability_t& ab, unit_ability_t::affects_t whom) const
{
	bool is_attacker = &self == &attacker;
	const auto& other = this->other(self);

	bool is_for_listing = this->is_for_listing;
	//log_scope("special_active");


	// Does this affect the specified unit?
	if ( whom == unit_ability_t::affects_t::SELF ) {
		if ( !special_affects_self(ab, is_attacker) )
			return false;
	}
	if ( whom == unit_ability_t::affects_t::OTHER ) {
		if ( !special_affects_opponent(ab, is_attacker) )
			return false;
	}

	// Is this active on attack/defense?
	if (!ab.active_on_matches(is_attacker)) {
		return false;
	}

	// Get the units involved.
	const unit_map& units = get_unit_map();

	unit_const_ptr self_u = self.un;
	unit_const_ptr other_u = other.un;

	// We also set the weapons context during (attack) wml events, in that case we identify the units via locations because wml might change
	// the actual unit and usually does so via replacing, in that case self_ is set to nullptr.
	// TODO: does this really make sense? if wml replaces the unit it also replaces the attack object, deleting the attack context properties
	if(self_u == nullptr) {
		unit_map::const_iterator it = units.find(self.loc);
		if(it.valid()) {
			self_u = it.get_shared_ptr();
		}
	}
	if(other_u == nullptr) {
		unit_map::const_iterator it = units.find(other.loc);
		if(it.valid()) {
			other_u = it.get_shared_ptr();
		}
	}

	// Make sure they're facing each other.
	temporary_facing self_facing(self_u, self.loc.get_relative_dir(other.loc));
	temporary_facing other_facing(other_u, other.loc.get_relative_dir(self.loc));

	// Filter poison, plague, drain, slow, petrifies
	// True if "whom" corresponds to "self", false if "whom" is "other"
	bool whom_is_self = ((whom == unit_ability_t::affects_t::SELF) || ((whom == unit_ability_t::affects_t::EITHER) && special_affects_self(ab, is_attacker)));
	unit_const_ptr them = whom_is_self ? other_u : self_u;
	map_location their_loc = whom_is_self ? other.loc : self.loc;

	if (buildin_is_immune(ab, them, their_loc)) {
		return false;
	}


	// Translate our context into terms of "attacker" and "defender".
	unit_const_ptr & att = is_attacker ? self_u : other_u;
	unit_const_ptr & def = is_attacker ? other_u : self_u;

	// Filter firststrike here, if both units have first strike then the effects cancel out. Only check
	// the opponent if "whom" is the defender, otherwise this leads to infinite recursion.
	if (ab.tag() == "firststrike") {
		bool whom_is_defender = whom_is_self ? !is_attacker : is_attacker;
		if (whom_is_defender && attacker.at && attacker.at->has_special_or_ability("firststrike"))
			return false;
	}

	// Filter the units involved.
	//If filter concerns the unit on which special is applied,
	//then the type of special must be entered to avoid calling
	//the function of this special in matches_filter()
	//In apply_to=both case, ab.tag() must be checked in all filter because special applied to both self and opponent.
	bool applied_both = ab.apply_to() == unit_ability_t::apply_to_t::both;
	const std::string& filter_self = ab.in_specials_tag() ? "filter_self" : "filter_student";

	bool applied_to_self = (applied_both || whom_is_self);
	if (!special_unit_matches(self_u, other_u, self.loc, self.at, ab, is_for_listing, filter_self, applied_to_self))
		return false;
	bool applied_to_opp = (applied_both || !whom_is_self);
	if (!special_unit_matches(other_u, self_u, other.loc, other.at, ab, is_for_listing, "filter_opponent", applied_to_opp))
		return false;
	//in case of apply_to=attacker|defender, if both [filter_attacker] and [filter_defender] are used,
	//check what is_attacker is true(or false for (filter_defender]) in affect self case only is necessary for what unit affected by special has a tag_name check.
	bool applied_to_attacker = applied_both || (whom_is_self && is_attacker) || (!whom_is_self && !is_attacker);
	if (!special_unit_matches(att, def, attacker.loc, attacker.at, ab, is_for_listing, "filter_attacker", applied_to_attacker))
		return false;
	bool applied_to_defender = applied_both || (whom_is_self && !is_attacker) || (!whom_is_self && is_attacker);
	if (!special_unit_matches(def, att, defender.loc, defender.at, ab, is_for_listing, "filter_defender", applied_to_defender))
		return false;

	return true;
}

namespace unit_abilities
{

void individual_effect::set(value_modifier t, int val, const config& abil, const map_location &l)
{
	type = t;
	value = val;
	ability = &abil;
	loc = l;
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

static int individual_value_int(const config::attribute_value *v, int def, const active_ability & ability, const map_location& loc, const const_attack_ptr& att) {
	int value = std::round(get_single_ability_value(*v, static_cast<double>(def), ability, loc, att, [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
		callable.add("base_value", wfl::variant(def));
		return std::round(formula.evaluate(callable).as_int());
	}));
	return value;
}

static int individual_value_double(const config::attribute_value *v, int def, const active_ability & ability, const map_location& loc, const const_attack_ptr& att) {
	int value = std::round(get_single_ability_value(*v, static_cast<double>(def), ability, loc, att, [&](const wfl::formula& formula, wfl::map_formula_callable& callable) {
		callable.add("base_value", wfl::variant(def));
		return formula.evaluate(callable).as_decimal() / 1000.0 ;
	}) * 100);
	return value;
}

effect::effect(const active_ability_list& list, int def, const const_attack_ptr& att, EFFECTS wham) :
	effect_list_(),
	composite_value_(def),
	composite_double_value_(def)
{
	std::map<double, active_ability_list> base_list;
	for(const active_ability& i : list) {
		double priority = i.ability().priority();
		if(base_list[priority].empty()) {
			base_list[priority] = active_ability_list(list.loc());
		}
		base_list[priority].emplace_back(i);
	}
	int value = def;
	for(auto base : base_list) {
		effect::effect_impl(base.second, value, att, wham);
		value = composite_value_;
	}
}

void effect::effect_impl(const active_ability_list& list, int def, const const_attack_ptr& att, EFFECTS wham )
{
	int value_set = def;
	std::map<std::string,individual_effect> values_add;
	std::map<std::string,individual_effect> values_sub;
	std::map<std::string,individual_effect> values_mul;
	std::map<std::string,individual_effect> values_div;

	individual_effect set_effect_max;
	individual_effect set_effect_min;
	individual_effect set_effect_cum;
	utils::optional<int> max_value = utils::nullopt;
	utils::optional<int> min_value = utils::nullopt;

	for (const active_ability & ability : list) {
		const config& cfg = ability.ability_cfg();
		const std::string& effect_id = cfg[cfg["id"].empty() ? "name" : "id"];

		if (!filter_base_matches(cfg, def))
			continue;

		if (const config::attribute_value *v = cfg.get("value")) {
			int value = individual_value_int(v, def, ability, list.loc(), att);
			int value_cum = wham != EFFECT_CUMULABLE && cfg["cumulative"].to_bool() ? std::max(def, value) : value;
			if(set_effect_cum.type != NOT_USED && wham == EFFECT_CUMULABLE && cfg["cumulative"].to_bool()) {
				set_effect_cum.set(SET, set_effect_cum.value + value_cum, ability.ability_cfg(), ability.teacher_loc);
			} else if(wham == EFFECT_CUMULABLE && cfg["cumulative"].to_bool()) {
				set_effect_cum.set(SET, value_cum, ability.ability_cfg(), ability.teacher_loc);
			} else {
				assert((set_effect_min.type != NOT_USED) == (set_effect_max.type != NOT_USED));
				if(set_effect_min.type == NOT_USED) {
					set_effect_min.set(SET, value_cum, ability.ability_cfg(), ability.teacher_loc);
					set_effect_max.set(SET, value_cum, ability.ability_cfg(), ability.teacher_loc);
				}
				else {
					if(value_cum > set_effect_max.value) {
						set_effect_max.set(SET, value_cum, ability.ability_cfg(), ability.teacher_loc);
					}
					if(value_cum < set_effect_min.value) {
						set_effect_min.set(SET, value_cum, ability.ability_cfg(), ability.teacher_loc);
					}
				}
			}
		}

		if(wham != EFFECT_WITHOUT_CLAMP_MIN_MAX) {
			if(const config::attribute_value *v = cfg.get("max_value")) {
				int value = individual_value_int(v, def, ability, list.loc(), att);
				max_value = max_value ? std::min(*max_value, value) : value;
			}
			if(const config::attribute_value *v = cfg.get("min_value")) {
				int value = individual_value_int(v, def, ability, list.loc(), att);
				min_value = min_value ? std::max(*min_value, value) : value;
			}
		}

		if (const config::attribute_value *v = cfg.get("add")) {
			int add = individual_value_int(v, def, ability, list.loc(), att);
			std::map<std::string,individual_effect>::iterator add_effect = values_add.find(effect_id);
			if(add_effect == values_add.end() || add > add_effect->second.value) {
				values_add[effect_id].set(ADD, add, ability.ability_cfg(), ability.teacher_loc);
			}
		}
		if (const config::attribute_value *v = cfg.get("sub")) {
			int sub = - individual_value_int(v, def, ability, list.loc(), att);
			std::map<std::string,individual_effect>::iterator sub_effect = values_sub.find(effect_id);
			if(sub_effect == values_sub.end() || sub < sub_effect->second.value) {
				values_sub[effect_id].set(ADD, sub, ability.ability_cfg(), ability.teacher_loc);
			}
		}
		if (const config::attribute_value *v = cfg.get("multiply")) {
			int multiply = individual_value_double(v, def, ability, list.loc(), att);
			std::map<std::string,individual_effect>::iterator mul_effect = values_mul.find(effect_id);
			if(mul_effect == values_mul.end() || multiply > mul_effect->second.value) {
				values_mul[effect_id].set(MUL, multiply, ability.ability_cfg(), ability.teacher_loc);
			}
		}
		if (const config::attribute_value *v = cfg.get("divide")) {
			int divide = individual_value_double(v, def, ability, list.loc(), att);

			if (divide == 0) {
				ERR_NG << "division by zero with divide= in ability/weapon special " << effect_id;
			}
			else {
				std::map<std::string,individual_effect>::iterator div_effect = values_div.find(effect_id);
				if(div_effect == values_div.end() || divide > div_effect->second.value) {
					values_div[effect_id].set(DIV, divide, ability.ability_cfg(), ability.teacher_loc);
				}
			}
		}
	}

	if(set_effect_max.type != NOT_USED) {
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

	if(set_effect_cum.type != NOT_USED) {
		value_set += set_effect_cum.value;
		effect_list_.push_back(set_effect_cum);
	}

	composite_double_value_ = (value_set + addition + substraction) * multiplier / divisor;
	//clamp what if min_value < max_value or one attribute only used.
	if(max_value && min_value && *min_value < *max_value) {
		composite_double_value_ = std::clamp(static_cast<double>(*min_value), static_cast<double>(*max_value), composite_double_value_);
	} else if(max_value && !min_value) {
		composite_double_value_ = std::min(static_cast<double>(*max_value), composite_double_value_);
	} else if(min_value && !max_value) {
		composite_double_value_ = std::max(static_cast<double>(*min_value), composite_double_value_);
	}
	composite_value_ = std::round(composite_double_value_);
}

} // end namespace unit_abilities
