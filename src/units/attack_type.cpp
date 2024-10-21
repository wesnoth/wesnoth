/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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
 *  Handle unit-type specific attributes, animations, advancement.
 */

#include "units/attack_type.hpp"
#include "units/unit.hpp"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/string_utils.hpp"
#include "formula/function_gamestate.hpp"
#include "deprecation.hpp"
#include "game_version.hpp"

#include "lexical_cast.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "gettext.hpp"
#include "utils/math.hpp"


static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)
#define ERR_UT LOG_STREAM(err, log_unit)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)

attack_type::attack_type(const config& cfg)
	: self_loc_()
	, other_loc_()
	, is_attacker_(false)
	, other_attack_(nullptr)
	, description_(cfg["description"].t_str())
	, id_(cfg["name"])
	, type_(cfg["type"])
	, icon_(cfg["icon"])
	, range_(cfg["range"])
	, min_range_(cfg["min_range"].to_int(1))
	, max_range_(cfg["max_range"].to_int(1))
	, alignment_(unit_alignments::get_enum(cfg["alignment"].str()))
	, damage_(cfg["damage"].to_int())
	, num_attacks_(cfg["number"].to_int())
	, attack_weight_(cfg["attack_weight"].to_double(1.0))
	, defense_weight_(cfg["defense_weight"].to_double(1.0))
	, accuracy_(cfg["accuracy"].to_int())
	, movement_used_(cfg["movement_used"].to_int(100000))
	, attacks_used_(cfg["attacks_used"].to_int(1))
	, parry_(cfg["parry"].to_int())
	, specials_(cfg.child_or_empty("specials"))
	, changed_(true)
{
	if (description_.empty())
		description_ = translation::egettext(id_.c_str());

	if(icon_.empty()){
		if (!id_.empty())
			icon_ = "attacks/" + id_ + ".png";
		else
			icon_ = "attacks/blank-attack.png";
	}
}

std::string attack_type::accuracy_parry_description() const
{
	if(accuracy_ == 0 && parry_ == 0) {
		return "";
	}

	std::ostringstream s;
	s << utils::signed_percent(accuracy_);

	if(parry_ != 0) {
		s << "/" << utils::signed_percent(parry_);
	}

	return s.str();
}

/**
 * Returns whether or not *this matches the given @a filter, ignoring the
 * complexities introduced by [and], [or], and [not].
 */
static bool matches_simple_filter(const attack_type & attack, const config & filter, const std::string& check_if_recursion)
{
	const std::set<std::string> filter_range = utils::split_set(filter["range"].str());
	const std::string& filter_min_range = filter["min_range"];
	const std::string& filter_max_range = filter["max_range"];
	const std::string& filter_damage = filter["damage"];
	const std::string& filter_attacks = filter["number"];
	const std::string& filter_accuracy = filter["accuracy"];
	const std::string& filter_parry = filter["parry"];
	const std::string& filter_movement = filter["movement_used"];
	const std::string& filter_attacks_used = filter["attacks_used"];
	const std::set<std::string> filter_alignment = utils::split_set(filter["alignment"].str());
	const std::set<std::string> filter_name = utils::split_set(filter["name"].str());
	const std::set<std::string> filter_type = utils::split_set(filter["type"].str());
	const std::vector<std::string> filter_special = utils::split(filter["special"]);
	const std::vector<std::string> filter_special_id = utils::split(filter["special_id"]);
	const std::vector<std::string> filter_special_type = utils::split(filter["special_type"]);
	const std::vector<std::string> filter_special_active = utils::split(filter["special_active"]);
	const std::vector<std::string> filter_special_id_active = utils::split(filter["special_id_active"]);
	const std::vector<std::string> filter_special_type_active = utils::split(filter["special_type_active"]);
	const std::string filter_formula = filter["formula"];

	if (!filter_min_range.empty() && !in_ranges(attack.min_range(), utils::parse_ranges_int(filter_min_range)))
		return false;

	if (!filter_max_range.empty() && !in_ranges(attack.max_range(), utils::parse_ranges_int(filter_max_range)))
		return false;

	if ( !filter_range.empty() && filter_range.count(attack.range()) == 0 )
		return false;

	if ( !filter_damage.empty() && !in_ranges(attack.damage(), utils::parse_ranges_unsigned(filter_damage)) )
		return false;

	if (!filter_attacks.empty() && !in_ranges(attack.num_attacks(), utils::parse_ranges_unsigned(filter_attacks)))
		return false;

	if (!filter_accuracy.empty() && !in_ranges(attack.accuracy(), utils::parse_ranges_int(filter_accuracy)))
		return false;

	if (!filter_parry.empty() && !in_ranges(attack.parry(), utils::parse_ranges_int(filter_parry)))
		return false;

	if (!filter_movement.empty() && !in_ranges(attack.movement_used(), utils::parse_ranges_unsigned(filter_movement)))
		return false;

	if (!filter_attacks_used.empty() && !in_ranges(attack.attacks_used(), utils::parse_ranges_unsigned(filter_attacks_used)))
		return false;

	if(!filter_alignment.empty() && filter_alignment.count(attack.alignment_str()) == 0)
		return false;

	if ( !filter_name.empty() && filter_name.count(attack.id()) == 0)
		return false;

	if (!filter_type.empty()){
		// Although there's a general guard against infinite recursion, the "damage_type" special
		// should always use the base type of the weapon. Otherwise it will flip-flop between the
		// special being active or inactive based on whether ATTACK_RECURSION_LIMIT is even or odd;
		// without this it will also behave differently when calculating resistance_against.
		if(check_if_recursion == "damage_type"){
			if (filter_type.count(attack.type()) == 0){
				return false;
			}
		} else {
			//if the type is different from "damage_type" then damage_type() can be called for safe checking.
			std::pair<std::string, std::string> damage_type = attack.damage_type();
			if (filter_type.count(damage_type.first) == 0 && filter_type.count(damage_type.second) == 0){
				return false;
			}
		}
	}

	if(!filter_special.empty()) {
		deprecated_message("special=", DEP_LEVEL::PREEMPTIVE, {1, 17, 0}, "Please use special_id or special_type instead");
		bool found = false;
		for(auto& special : filter_special) {
			if(attack.has_special(special, true)) {
				found = true;
				break;
			}
		}
		if(!found) {
			return false;
		}
	}
	if(!filter_special_id.empty()) {
		bool found = false;
		for(auto& special : filter_special_id) {
			if(attack.has_special(special, true, true, false)) {
				found = true;
				break;
			}
		}
		if(!found) {
			return false;
		}
	}

	if(!filter_special_active.empty()) {
		deprecated_message("special_active=", DEP_LEVEL::PREEMPTIVE, {1, 17, 0}, "Please use special_id_active or special_type_active instead");
		bool found = false;
		for(auto& special : filter_special_active) {
			if(attack.has_special(special, false)) {
				found = true;
				break;
			}
		}
		if(!found) {
			return false;
		}
	}
	if(!filter_special_id_active.empty()) {
		bool found = false;
		for(auto& special : filter_special_id_active) {
			if(attack.has_special_or_ability(special, true, false)) {
				found = true;
				break;
			}
		}
		if(!found) {
			return false;
		}
	}
	if(!filter_special_type.empty()) {
		bool found = false;
		for(auto& special : filter_special_type) {
			if(attack.has_special(special, true, false)) {
				found = true;
				break;
			}
		}
		if(!found) {
			return false;
		}
	}
	if(!filter_special_type_active.empty()) {
		bool found = false;
		for(auto& special : filter_special_type_active) {
			if(attack.has_special_or_ability(special, false)) {
				found = true;
				break;
			}
		}
		if(!found) {
			return false;
		}
	}

	//children filter_special are checked later,
	//but only when the function doesn't return earlier
	if(auto sub_filter_special = filter.optional_child("filter_special")) {
		if(!attack.has_special_or_ability_with_filter(*sub_filter_special)) {
			return false;
		}
	}

	if (!filter_formula.empty()) {
		try {
			const wfl::attack_type_callable callable(attack);
			const wfl::formula form(filter_formula, new wfl::gamestate_function_symbol_table);
			if(!form.evaluate(callable).as_bool()) {
				return false;
			}
		} catch(const wfl::formula_error& e) {
			lg::log_to_chat() << "Formula error in weapon filter: " << e.type << " at " << e.filename << ':' << e.line << ")\n";
			ERR_WML << "Formula error in weapon filter: " << e.type << " at " << e.filename << ':' << e.line << ")";
			// Formulae with syntax errors match nothing
			return false;
		}
	}

	// Passed all tests.
	return true;
}

/**
 * Returns whether or not *this matches the given @a filter.
 */
bool attack_type::matches_filter(const config& filter, const std::string& check_if_recursion) const
{
	// Handle the basic filter.
	bool matches = matches_simple_filter(*this, filter, check_if_recursion);

	// Handle [and], [or], and [not] with in-order precedence
	for(const auto [key, condition_cfg] : filter.all_children_view() )
	{
		// Handle [and]
		if ( key == "and" )
			matches = matches && matches_filter(condition_cfg, check_if_recursion);

		// Handle [or]
		else if ( key == "or" )
			matches = matches || matches_filter(condition_cfg, check_if_recursion);

		// Handle [not]
		else if ( key == "not" )
			matches = matches && !matches_filter(condition_cfg, check_if_recursion);
	}

	return matches;
}

void attack_type::remove_special_by_filter(const config& filter)
{
	config::all_children_iterator i = specials_.ordered_begin();
	while (i != specials_.ordered_end()) {
		if(special_matches_filter(i->cfg, i->key, filter)) {
			i = specials_.erase(i);
		} else {
			++i;
		}
	}
}

/**
 * Modifies *this using the specifications in @a cfg, but only if *this matches
 * @a cfg viewed as a filter.
 *
 * @returns whether or not @c this matched the @a cfg as a filter.
 */
bool attack_type::apply_modification(const config& cfg)
{
	if( !matches_filter(cfg) )
		return false;

	set_changed(true);
	const std::string& set_name = cfg["set_name"];
	const t_string& set_desc = cfg["set_description"];
	const std::string& set_type = cfg["set_type"];
	const std::string& set_range = cfg["set_range"];
	const std::string& set_attack_alignment = cfg["set_alignment"];
	const std::string& set_icon = cfg["set_icon"];
	const std::string& del_specials = cfg["remove_specials"];
	auto set_specials = cfg.optional_child("set_specials");
	const std::string& increase_min_range = cfg["increase_min_range"];
	const std::string& set_min_range = cfg["set_min_range"];
	const std::string& increase_max_range = cfg["increase_max_range"];
	const std::string& set_max_range = cfg["set_max_range"];
	auto remove_specials = cfg.optional_child("remove_specials");
	const std::string& increase_damage = cfg["increase_damage"];
	const std::string& set_damage = cfg["set_damage"];
	const std::string& increase_attacks = cfg["increase_attacks"];
	const std::string& set_attacks = cfg["set_attacks"];
	const std::string& set_attack_weight = cfg["attack_weight"];
	const std::string& set_defense_weight = cfg["defense_weight"];
	const std::string& increase_accuracy = cfg["increase_accuracy"];
	const std::string& set_accuracy = cfg["set_accuracy"];
	const std::string& increase_parry = cfg["increase_parry"];
	const std::string& set_parry = cfg["set_parry"];
	const std::string& increase_movement = cfg["increase_movement_used"];
	const std::string& set_movement = cfg["set_movement_used"];
	const std::string& increase_attacks_used = cfg["increase_attacks_used"];
	const std::string& set_attacks_used = cfg["set_attacks_used"];
	// NB: If you add something here that requires a description,
	// it needs to be added to describe_modification as well.

	if(set_name.empty() == false) {
		id_ = set_name;
	}

	if(set_desc.empty() == false) {
		description_ = set_desc;
	}

	if(set_type.empty() == false) {
		type_ = set_type;
	}

	if(set_range.empty() == false) {
		range_ = set_range;
	}

	if(set_attack_alignment.empty() == false) {
		alignment_ = unit_alignments::get_enum(set_attack_alignment);
	}

	if(set_icon.empty() == false) {
		icon_ = set_icon;
	}

	if(del_specials.empty() == false) {
		const std::vector<std::string>& dsl = utils::split(del_specials);
		config new_specials;
		for(const auto [key, cfg] : specials_.all_children_view()) {
			std::vector<std::string>::const_iterator found_id =
				std::find(dsl.begin(), dsl.end(), cfg["id"].str());
			if (found_id == dsl.end()) {
				new_specials.add_child(key, cfg);
			}
		}
		specials_ = new_specials;
	}

	if(set_specials) {
		const std::string &mode = set_specials["mode"];
		if(mode.empty()){
			deprecated_message("[set_specials]mode=<unset>", DEP_LEVEL::INDEFINITE, "",
				"The mode defaults to 'replace', but should often be 'append' instead. The default may change in a future version, or the attribute may become mandatory.");
			// fall through to mode != "append"
		}
		if(mode != "append") {
			specials_.clear();
		}
		for(const auto [key, cfg] : set_specials->all_children_view()) {
			specials_.add_child(key, cfg);
		}
	}

	if(set_min_range.empty() == false) {
		min_range_ = std::stoi(set_min_range);
	}

	if(increase_min_range.empty() == false) {
		min_range_ = utils::apply_modifier(min_range_, increase_min_range);
	}

	if(set_max_range.empty() == false) {
		max_range_ = std::stoi(set_max_range);
	}

	if(increase_max_range.empty() == false) {
		max_range_ = utils::apply_modifier(max_range_, increase_max_range);
	}

	if(remove_specials) {
		remove_special_by_filter(*remove_specials);
	}

	if(set_damage.empty() == false) {
		damage_ = std::stoi(set_damage);
		if (damage_ < 0) {
			damage_ = 0;
		}
	}

	if(increase_damage.empty() == false) {
		damage_ = utils::apply_modifier(damage_, increase_damage);
		if(damage_ < 0) {
			damage_ = 0;
		}
	}

	if(set_attacks.empty() == false) {
		num_attacks_ = std::stoi(set_attacks);
		if (num_attacks_ < 0) {
			num_attacks_ = 0;
		}

	}

	if(increase_attacks.empty() == false) {
		num_attacks_ = utils::apply_modifier(num_attacks_, increase_attacks, 1);
	}

	if(set_accuracy.empty() == false) {
		accuracy_ = std::stoi(set_accuracy);
	}

	if(increase_accuracy.empty() == false) {
		accuracy_ = utils::apply_modifier(accuracy_, increase_accuracy);
	}

	if(set_parry.empty() == false) {
		parry_ = std::stoi(set_parry);
	}

	if(increase_parry.empty() == false) {
		parry_ = utils::apply_modifier(parry_, increase_parry);
	}

	if(set_movement.empty() == false) {
		movement_used_ = std::stoi(set_movement);
	}

	if(increase_movement.empty() == false) {
		movement_used_ = utils::apply_modifier(movement_used_, increase_movement, 1);
	}

	if(set_attacks_used.empty() == false) {
		attacks_used_ = std::stoi(set_attacks_used);
	}

	if(increase_attacks_used.empty() == false) {
		attacks_used_ = utils::apply_modifier(attacks_used_, increase_attacks_used, 1);
	}

	if(set_attack_weight.empty() == false) {
		attack_weight_ = lexical_cast_default<double>(set_attack_weight,1.0);
	}

	if(set_defense_weight.empty() == false) {
		defense_weight_ = lexical_cast_default<double>(set_defense_weight,1.0);
	}

	return true;
}

/**
 * Trimmed down version of apply_modification(), with no modifications actually
 * made. This can be used to get a description of the modification(s) specified
 * by @a cfg (if *this matches cfg as a filter).
 *
 * If *description is provided, it will be set to a (translated) description
 * of the modification(s) applied (currently only changes to the number of
 * strikes, damage, accuracy, and parry are included in this description).
 *
 * @returns whether or not @c this matched the @a cfg as a filter.
 */
bool attack_type::describe_modification(const config& cfg,std::string* description)
{
	if( !matches_filter(cfg) )
		return false;

	// Did the caller want the description?
	if(description != nullptr) {
		const std::string& increase_min_range = cfg["increase_min_range"];
		const std::string& set_min_range = cfg["set_min_range"];
		const std::string& increase_max_range = cfg["increase_max_range"];
		const std::string& set_max_range = cfg["set_max_range"];
		const std::string& increase_damage = cfg["increase_damage"];
		const std::string& set_damage = cfg["set_damage"];
		const std::string& increase_attacks = cfg["increase_attacks"];
		const std::string& set_attacks = cfg["set_attacks"];
		const std::string& increase_accuracy = cfg["increase_accuracy"];
		const std::string& set_accuracy = cfg["set_accuracy"];
		const std::string& increase_parry = cfg["increase_parry"];
		const std::string& set_parry = cfg["set_parry"];
		const std::string& increase_movement = cfg["increase_movement_used"];
		const std::string& set_movement = cfg["set_movement_used"];
		const std::string& increase_attacks_used = cfg["increase_attacks_used"];
		const std::string& set_attacks_used = cfg["set_attacks_used"];

		std::vector<t_string> desc;

		if(!set_min_range.empty()) {
			desc.emplace_back(VGETTEXT(
				// TRANSLATORS: Current value for WML code set_min_range, documented in https://wiki.wesnoth.org/EffectWML
				"$number min range",
				{{"number", set_min_range}}));
		}

		if(!increase_min_range.empty()) {
			desc.emplace_back(VGETTEXT(
				// TRANSLATORS: Current value for WML code increase_min_range, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent</span> min range",
				{{"number_or_percent", utils::print_modifier(increase_min_range)}, {"color", increase_min_range[0] == '-' ? "#f00" : "#0f0"}}));
		}

		if(!set_max_range.empty()) {
			desc.emplace_back(VGETTEXT(
				// TRANSLATORS: Current value for WML code set_max_range, documented in https://wiki.wesnoth.org/EffectWML
				"$number max range",
				{{"number", set_max_range}}));
		}

		if(!increase_max_range.empty()) {
			desc.emplace_back(VGETTEXT(
				// TRANSLATORS: Current value for WML code increase_max_range, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent</span> max range",
				{{"number_or_percent", utils::print_modifier(increase_max_range)}, {"color", increase_max_range[0] == '-' ? "#f00" : "#0f0"}}));
		}

		if(!increase_damage.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code increase_damage, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent</span> damage",
				"<span color=\"$color\">$number_or_percent</span> damage",
				std::stoi(increase_damage),
				{{"number_or_percent", utils::print_modifier(increase_damage)}, {"color", increase_damage[0] == '-' ? "#f00" : "#0f0"}}));
		}

		if(!set_damage.empty()) {
			// TRANSLATORS: Current value for WML code set_damage, documented in https://wiki.wesnoth.org/EffectWML
			desc.emplace_back(VNGETTEXT(
				"$number damage",
				"$number damage",
				std::stoi(set_damage),
				{{"number", set_damage}}));
		}

		if(!increase_attacks.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code increase_attacks, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent</span> strike",
				"<span color=\"$color\">$number_or_percent</span> strikes",
				std::stoi(increase_attacks),
				{{"number_or_percent", utils::print_modifier(increase_attacks)}, {"color", increase_attacks[0] == '-' ? "#f00" : "#0f0"}}));
		}

		if(!set_attacks.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code set_attacks, documented in https://wiki.wesnoth.org/EffectWML
				"$number strike",
				"$number strikes",
				std::stoi(set_attacks),
				{{"number", set_attacks}}));
		}

		if(!set_accuracy.empty()) {
			desc.emplace_back(VGETTEXT(
				// TRANSLATORS: Current value for WML code set_accuracy, documented in https://wiki.wesnoth.org/EffectWML
				"$number| accuracy",
				{{"number", set_accuracy}}));
		}

		if(!increase_accuracy.empty()) {
			desc.emplace_back(VGETTEXT(
				// TRANSLATORS: Current value for WML code increase_accuracy, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent|%</span> accuracy",
				{{"number_or_percent", utils::print_modifier(increase_accuracy)}, {"color", increase_accuracy[0] == '-' ? "#f00" : "#0f0"}}));
		}

		if(!set_parry.empty()) {
			desc.emplace_back(VGETTEXT(
				// TRANSLATORS: Current value for WML code set_parry, documented in https://wiki.wesnoth.org/EffectWML
				"$number parry",
				{{"number", set_parry}}));
		}

		if(!increase_parry.empty()) {
			desc.emplace_back(VGETTEXT(
				// TRANSLATORS: Current value for WML code increase_parry, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent</span> parry",
				{{"number_or_percent", utils::print_modifier(increase_parry)}, {"color", increase_parry[0] == '-' ? "#f00" : "#0f0"}}));
		}

		if(!set_movement.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code set_movement_used, documented in https://wiki.wesnoth.org/EffectWML
				"$number movement point",
				"$number movement points",
				std::stoi(set_movement),
				{{"number", set_movement}}));
		}

		if(!increase_movement.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code increase_movement_used, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent</span> movement point",
				"<span color=\"$color\">$number_or_percent</span> movement points",
				std::stoi(increase_movement),
				{{"number_or_percent", utils::print_modifier(increase_movement)}, {"color", increase_movement[0] == '-' ? "#f00" : "#0f0"}}));
		}

		if(!set_attacks_used.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code set_attacks_used, documented in https://wiki.wesnoth.org/EffectWML
				"$number attack used",
				"$number attacks used",
				std::stoi(set_attacks_used),
				{{"number", set_attacks_used}}));
		}

		if(!increase_attacks_used.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code increase_attacks_used, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent</span> attack used",
				"<span color=\"$color\">$number_or_percent</span> attacks used",
				std::stoi(increase_attacks_used),
				{{"number_or_percent", utils::print_modifier(increase_attacks_used)}, {"color", increase_attacks_used[0] == '-' ? "#f00" : "#0f0"}}));
		}

		*description = utils::format_conjunct_list("", desc);
	}

	return true;
}

attack_type::recursion_guard attack_type::update_variables_recursion(const config& special) const
{
	if(utils::contains(open_queries_, &special)) {
		return recursion_guard();
	}
	return recursion_guard(*this, special);
}

attack_type::recursion_guard::recursion_guard() = default;

attack_type::recursion_guard::recursion_guard(const attack_type& weapon, const config& special)
	: parent(weapon.shared_from_this())
{
	parent->open_queries_.emplace_back(&special);
}

attack_type::recursion_guard::recursion_guard(attack_type::recursion_guard&& other)
{
	std::swap(parent, other.parent);
}

attack_type::recursion_guard::operator bool() const {
	return bool(parent);
}

attack_type::recursion_guard& attack_type::recursion_guard::operator=(attack_type::recursion_guard&& other)
{
	// This is only intended to move ownership to a longer-living variable. Assigning to an instance that
	// already has a parent implies that the caller is going to recurse and needs a recursion allocation,
	// but is accidentally dropping one of the allocations that it already has; hence the asserts.
	assert(this != &other);
	assert(!parent);
	std::swap(parent, other.parent);
	return *this;
}

attack_type::recursion_guard::~recursion_guard()
{
	if(parent) {
		// As this only expects nested recursion, simply pop the top of the open_queries_ stack
		// without checking that the top of the stack matches the filter passed to the constructor.
		assert(!parent->open_queries_.empty());
		parent->open_queries_.pop_back();
	}
}

void attack_type::write(config& cfg) const
{
	cfg["description"] = description_;
	cfg["name"] = id_;
	cfg["type"] = type_;
	cfg["icon"] = icon_;
	cfg["range"] = range_;
	cfg["min_range"] = min_range_;
	cfg["max_range"] = max_range_;
	cfg["alignment"] = alignment_str();
	cfg["damage"] = damage_;
	cfg["number"] = num_attacks_;
	cfg["attack_weight"] = attack_weight_;
	cfg["defense_weight"] = defense_weight_;
	cfg["accuracy"] = accuracy_;
	cfg["movement_used"] = movement_used_;
	cfg["attacks_used"] = attacks_used_;
	cfg["parry"] = parry_;
	cfg.add_child("specials", specials_);
}
