/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 *  Handle unit-type specific attributes, animations, advancement.
 */

#include "units/attack_type.hpp"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/string_utils.hpp"
#include "formula/function_gamestate.hpp"

#include "lexical_cast.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "gettext.hpp"
#include "utils/math.hpp"

#include <cassert>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)
#define ERR_UT LOG_STREAM(err, log_unit)

attack_type::attack_type(const config& cfg) :
	self_loc_(),
	other_loc_(),
	is_attacker_(false),
	other_attack_(nullptr),
	description_(cfg["description"].t_str()),
	id_(cfg["name"]),
	type_(cfg["type"]),
	icon_(cfg["icon"]),
	range_(cfg["range"]),
	min_range_(cfg["min_range"].to_int(1)),
	max_range_(cfg["max_range"].to_int(1)),
	damage_(cfg["damage"]),
	num_attacks_(cfg["number"]),
	attack_weight_(cfg["attack_weight"].to_double(1.0)),
	defense_weight_(cfg["defense_weight"].to_double(1.0)),
	accuracy_(cfg["accuracy"]),
	movement_used_(cfg["movement_used"].to_int(100000)),
	parry_(cfg["parry"]),
	specials_(cfg.child_or_empty("specials"))
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
static bool matches_simple_filter(const attack_type & attack, const config & filter)
{
	const std::vector<std::string>& filter_range = utils::split(filter["range"]);
	const std::string& filter_damage = filter["damage"];
	const std::string& filter_attacks = filter["number"];
	const std::string& filter_accuracy = filter["accuracy"];
	const std::string& filter_parry = filter["parry"];
	const std::string& filter_movement = filter["movement_used"];
	const std::vector<std::string> filter_name = utils::split(filter["name"]);
	const std::vector<std::string> filter_type = utils::split(filter["type"]);
	const std::string filter_special = filter["special"];
	const std::string filter_special_active = filter["special_active"];
	const std::string filter_formula = filter["formula"];

	if ( !filter_range.empty() && std::find(filter_range.begin(), filter_range.end(), attack.range()) == filter_range.end() )
		return false;

	if ( !filter_damage.empty() && !in_ranges(attack.damage(), utils::parse_ranges(filter_damage)) )
		return false;

	if (!filter_attacks.empty() && !in_ranges(attack.num_attacks(), utils::parse_ranges(filter_attacks)))
		return false;

	if (!filter_accuracy.empty() && !in_ranges(attack.accuracy(), utils::parse_ranges(filter_accuracy)))
		return false;

	if (!filter_parry.empty() && !in_ranges(attack.parry(), utils::parse_ranges(filter_parry)))
		return false;

	if (!filter_movement.empty() && !in_ranges(attack.movement_used(), utils::parse_ranges(filter_movement)))
		return false;

	if ( !filter_name.empty() && std::find(filter_name.begin(), filter_name.end(), attack.id()) == filter_name.end() )
		return false;

	if ( !filter_type.empty() && std::find(filter_type.begin(), filter_type.end(), attack.type()) == filter_type.end() )
		return false;

	if ( !filter_special.empty() && !attack.get_special_bool(filter_special, true) )
		return false;

	if ( !filter_special_active.empty() && !attack.get_special_bool(filter_special_active, false) )
		return false;

	if (!filter_formula.empty()) {
		try {
			const wfl::attack_type_callable callable(attack);
			const wfl::formula form(filter_formula, new wfl::gamestate_function_symbol_table);
			if(!form.evaluate(callable).as_bool()) {
				return false;
			}
		} catch(wfl::formula_error& e) {
			lg::wml_error() << "Formula error in weapon filter: " << e.type << " at " << e.filename << ':' << e.line << ")\n";
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
bool attack_type::matches_filter(const config& filter) const
{
	// Handle the basic filter.
	bool matches = matches_simple_filter(*this, filter);

	// Handle [and], [or], and [not] with in-order precedence
	for (const config::any_child &condition : filter.all_children_range() )
	{
		// Handle [and]
		if ( condition.key == "and" )
			matches = matches && matches_filter(condition.cfg);

		// Handle [or]
		else if ( condition.key == "or" )
			matches = matches || matches_filter(condition.cfg);

		// Handle [not]
		else if ( condition.key == "not" )
			matches = matches && !matches_filter(condition.cfg);
	}

	return matches;
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

	const std::string& set_name = cfg["set_name"];
	const t_string& set_desc = cfg["set_description"];
	const std::string& set_type = cfg["set_type"];
	const std::string& set_icon = cfg["set_icon"];
	const std::string& del_specials = cfg["remove_specials"];
	const config &set_specials = cfg.child("set_specials");
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

	if(set_icon.empty() == false) {
		icon_ = set_icon;
	}

	if(del_specials.empty() == false) {
		const std::vector<std::string>& dsl = utils::split(del_specials);
		config new_specials;
		for (const config::any_child &vp : specials_.all_children_range()) {
			std::vector<std::string>::const_iterator found_id =
				std::find(dsl.begin(), dsl.end(), vp.cfg["id"].str());
			if (found_id == dsl.end()) {
				new_specials.add_child(vp.key, vp.cfg);
			}
		}
		specials_ = new_specials;
	}

	if (set_specials) {
		const std::string &mode = set_specials["mode"];
		if (mode != "append") {
			specials_.clear();
		}
		for (const config::any_child &value : set_specials.all_children_range()) {
			specials_.add_child(value.key, value.cfg);
		}
	}

	if(set_damage.empty() == false) {
		damage_ = std::stoi(set_damage);
		if (damage_ < 0) {
			damage_ = 0;
		}
	}

	if(increase_damage.empty() == false) {
		damage_ = utils::apply_modifier(damage_, increase_damage, 0);
		if (damage_ < 0) {
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
		accuracy_ = utils::apply_modifier(accuracy_, increase_accuracy, 1);
	}

	if(set_parry.empty() == false) {
		parry_ = std::stoi(set_parry);
	}

	if(increase_parry.empty() == false) {
		parry_ = utils::apply_modifier(parry_, increase_parry, 1);
	}

	if(set_movement.empty() == false) {
		movement_used_ = std::stoi(set_movement);
	}

	if(increase_movement.empty() == false) {
		movement_used_ = utils::apply_modifier(movement_used_, increase_movement, 1);
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

		std::vector<t_string> desc;

		if(!increase_damage.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code increase_damage, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent</span> damage",
				"<span color=\"$color\">$number_or_percent</span> damage",
				std::stoi(increase_damage),
				{{"number_or_percent", utils::print_modifier(increase_damage)}, {"color", increase_damage[0] == '-' ? "red" : "green"}}));
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
				{{"number_or_percent", utils::print_modifier(increase_attacks)}, {"color", increase_attacks[0] == '-' ? "red" : "green"}}));
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
				{{"number_or_percent", utils::print_modifier(increase_accuracy)}, {"color", increase_accuracy[0] == '-' ? "red" : "green"}}));
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
				{{"number_or_percent", utils::print_modifier(increase_parry)}, {"color", increase_parry[0] == '-' ? "red" : "green"}}));
		}

		if(!set_movement.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code set_movement, documented in https://wiki.wesnoth.org/EffectWML
				"$number movement point",
				"$number movement points",
				std::stoi(set_movement),
				{{"number", set_movement}}));
		}

		if(!increase_movement.empty()) {
			desc.emplace_back(VNGETTEXT(
				// TRANSLATORS: Current value for WML code increase_movement, documented in https://wiki.wesnoth.org/EffectWML
				"<span color=\"$color\">$number_or_percent movement</span> point",
				"<span color=\"$color\">$number_or_percent movement</span> points",
				std::stoi(increase_movement),
				{{"number_or_percent", utils::print_modifier(increase_movement)}, {"color", increase_movement[0] == '-' ? "red" : "green"}}));
		}

		*description = utils::format_conjunct_list("", desc);
	}

	return true;
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
	cfg["damage"] = damage_;
	cfg["number"] = num_attacks_;
	cfg["attack_weight"] = attack_weight_;
	cfg["defense_weight"] = defense_weight_;
	cfg["accuracy"] = accuracy_;
	cfg["movement_used"] = movement_used_;
	cfg["parry"] = parry_;
	cfg.add_child("specials", specials_);
}
