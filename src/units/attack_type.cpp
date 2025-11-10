/*
	Copyright (C) 2003 - 2025
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
#include "units/types.hpp"
#include "units/unit.hpp"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/string_utils.hpp"
#include "formula/function_gamestate.hpp"
#include "deprecation.hpp"
#include "game_version.hpp"

#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/markup.hpp"
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

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)


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
	, specials_()
	, changed_(true)
{
	config specials_cfg = unit_type_data::add_registry_entries(cfg, "specials", unit_types.specials());
	unit_ability_t::parse_vector(specials_cfg, specials_, true);

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

std::string attack_type::accuracy_parry_tooltip() const
{
	if(accuracy_ == 0 && parry_ == 0) {
		return "";
	}

	std::stringstream tooltip;
	if (accuracy_) {
		tooltip << _("Accuracy:") << " " << markup::bold(utils::signed_percent(accuracy_)) << "\n";
	}
	if (parry_) {
		tooltip << _("Parry:") << " " << markup::bold(utils::signed_percent(parry_));
	}

	return tooltip.str();
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
	const std::set<std::string> filter_base_type = utils::split_set(filter["base_type"].str());
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
			if (filter_type.count(attack.effective_damage_type().first) == 0){
				return false;
			}
		}
	}

	if ( !filter_base_type.empty() && filter_base_type.count(attack.type()) == 0 )
		return false;

	if(filter.has_attribute("special")) {
		deprecated_message("special=", DEP_LEVEL::PREEMPTIVE, {1, 17, 0}, "Please use special_id or special_type instead");
	}

	if(filter.has_attribute("special") || filter.has_attribute("special_id") || filter.has_attribute("special_type")) {
		if(!attack.has_filter_special_or_ability(filter, true)) {
			return false;
		}
	}

	if(filter.has_attribute("special_active")) {
		deprecated_message("special_active=", DEP_LEVEL::PREEMPTIVE, {1, 17, 0}, "Please use special_id_active or special_type_active instead");
	}

	if(filter.has_attribute("special_active") || filter.has_attribute("special_id_active") || filter.has_attribute("special_type_active")) {
		if(!attack.has_filter_special_or_ability(filter)) {
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
			wfl::gamestate_function_symbol_table symbols;
			const wfl::formula form(filter_formula, &symbols);
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
	auto i = specials_.begin();
	while (i != specials_.end()) {
		if(special_matches_filter(**i, filter)) {
			i = specials_.erase(i);
		} else {
			++i;
		}
	}
}

void attack_type::apply_effect(const config& cfg)
{
	set_changed(true);
	const config::attribute_value& set_name = cfg["set_name"];
	const t_string& set_desc = cfg["set_description"].t_str();
	const config::attribute_value& set_type = cfg["set_type"];
	const config::attribute_value& set_range = cfg["set_range"];
	const config::attribute_value& set_attack_alignment = cfg["set_alignment"];
	const config::attribute_value& set_icon = cfg["set_icon"];
	const config::attribute_value& del_specials = cfg["remove_specials"];
	auto set_specials = cfg.optional_child("set_specials");
	const config::attribute_value& increase_min_range = cfg["increase_min_range"];
	const config::attribute_value& set_min_range = cfg["set_min_range"];
	const config::attribute_value& increase_max_range = cfg["increase_max_range"];
	const config::attribute_value& set_max_range = cfg["set_max_range"];
	auto remove_specials = cfg.optional_child("remove_specials");
	const config::attribute_value& increase_damage = cfg["increase_damage"];
	const config::attribute_value& set_damage = cfg["set_damage"];
	const config::attribute_value& increase_attacks = cfg["increase_attacks"];
	const config::attribute_value& set_attacks = cfg["set_attacks"];
	const config::attribute_value& set_attack_weight = cfg["attack_weight"];
	const config::attribute_value& set_defense_weight = cfg["defense_weight"];
	const config::attribute_value& increase_accuracy = cfg["increase_accuracy"];
	const config::attribute_value& set_accuracy = cfg["set_accuracy"];
	const config::attribute_value& increase_parry = cfg["increase_parry"];
	const config::attribute_value& set_parry = cfg["set_parry"];
	const config::attribute_value& increase_movement = cfg["increase_movement_used"];
	const config::attribute_value& set_movement = cfg["set_movement_used"];
	const config::attribute_value& increase_attacks_used = cfg["increase_attacks_used"];
	const config::attribute_value& set_attacks_used = cfg["set_attacks_used"];
	// NB: If you add something here that requires a description,
	// it needs to be added to describe_effect as well.

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
		alignment_ = unit_alignments::get_enum(set_attack_alignment.str());
	}

	if(set_icon.empty() == false) {
		icon_ = set_icon;
	}

	if(del_specials.empty() == false) {
		const std::vector<std::string>& dsl = utils::split(del_specials);
		ability_vector new_specials;
		for(ability_ptr& p_ab : specials_) {
			if(!utils::contains(dsl, p_ab->id())) {
				new_specials.emplace_back(std::move(p_ab));
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
		// expand and add registry weapon specials
		config registry_specials = unit_type_data::add_registry_entries(
					config{"specials_list", set_specials["specials_list"]}, "specials", unit_types.specials());
		for(const auto [key, cfg] : registry_specials.all_children_view()) {
			specials_.push_back(unit_ability_t::create(key, cfg, true));
		}

		for(const auto [key, cfg] : set_specials->all_children_view()) {
			specials_.push_back(unit_ability_t::create(key, cfg, true));
		}
	}

	if(set_min_range.empty() == false) {
		min_range_ = set_min_range.to_int();
	}

	if(increase_min_range.empty() == false) {
		min_range_ = utils::apply_modifier(min_range_, increase_min_range);
	}

	if(set_max_range.empty() == false) {
		max_range_ = set_max_range.to_int();
	}

	if(increase_max_range.empty() == false) {
		max_range_ = utils::apply_modifier(max_range_, increase_max_range);
	}

	if(remove_specials) {
		remove_special_by_filter(*remove_specials);
	}

	if(set_damage.empty() == false) {
		damage_ = set_damage.to_int();
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
		num_attacks_ = set_attacks.to_int();
		if (num_attacks_ < 0) {
			num_attacks_ = 0;
		}

	}

	if(increase_attacks.empty() == false) {
		num_attacks_ = utils::apply_modifier(num_attacks_, increase_attacks, 1);
	}

	if(set_accuracy.empty() == false) {
		accuracy_ = set_accuracy.to_int();
	}

	if(increase_accuracy.empty() == false) {
		accuracy_ = utils::apply_modifier(accuracy_, increase_accuracy);
	}

	if(set_parry.empty() == false) {
		parry_ = set_parry.to_int();
	}

	if(increase_parry.empty() == false) {
		parry_ = utils::apply_modifier(parry_, increase_parry);
	}

	if(set_movement.empty() == false) {
		movement_used_ = set_movement.to_int();
	}

	if(increase_movement.empty() == false) {
		movement_used_ = utils::apply_modifier(movement_used_, increase_movement, 1);
	}

	if(set_attacks_used.empty() == false) {
		attacks_used_ = set_attacks_used.to_int();
	}

	if(increase_attacks_used.empty() == false) {
		attacks_used_ = utils::apply_modifier(attacks_used_, increase_attacks_used, 1);
	}

	if(set_attack_weight.empty() == false) {
		attack_weight_ = set_attack_weight.to_double(1.0);
	}

	if(set_defense_weight.empty() == false) {
		defense_weight_ = set_defense_weight.to_double(1.0);
	}
}

std::string attack_type::describe_effect(const config& cfg)
{
	const config::attribute_value& increase_min_range = cfg["increase_min_range"];
	const config::attribute_value& set_min_range = cfg["set_min_range"];
	const config::attribute_value& increase_max_range = cfg["increase_max_range"];
	const config::attribute_value& set_max_range = cfg["set_max_range"];
	const config::attribute_value& increase_damage = cfg["increase_damage"];
	const config::attribute_value& set_damage = cfg["set_damage"];
	const config::attribute_value& increase_attacks = cfg["increase_attacks"];
	const config::attribute_value& set_attacks = cfg["set_attacks"];
	const config::attribute_value& increase_accuracy = cfg["increase_accuracy"];
	const config::attribute_value& set_accuracy = cfg["set_accuracy"];
	const config::attribute_value& increase_parry = cfg["increase_parry"];
	const config::attribute_value& set_parry = cfg["set_parry"];
	const config::attribute_value& increase_movement = cfg["increase_movement_used"];
	const config::attribute_value& set_movement = cfg["set_movement_used"];
	const config::attribute_value& increase_attacks_used = cfg["increase_attacks_used"];
	const config::attribute_value& set_attacks_used = cfg["set_attacks_used"];

	const auto format_modifier = [](const config::attribute_value& attr) -> utils::string_map {
		return {{"number_or_percent", utils::print_modifier(attr)}, {"color", attr.to_int() < 0 ? "#f00" : "#0f0"}};
	};

	std::vector<t_string> desc;

	if(!set_min_range.empty()) {
		desc.emplace_back(VGETTEXT(
			// TRANSLATORS: Current value for WML code set_min_range, documented in https://wiki.wesnoth.org/EffectWML
			"$number min range",
			{{"number", set_min_range.str()}}));
	}

	if(!increase_min_range.empty()) {
		desc.emplace_back(VGETTEXT(
			// TRANSLATORS: Current value for WML code increase_min_range, documented in https://wiki.wesnoth.org/EffectWML
			"<span color=\"$color\">$number_or_percent</span> min range",
			format_modifier(increase_min_range)));
	}

	if(!set_max_range.empty()) {
		desc.emplace_back(VGETTEXT(
			// TRANSLATORS: Current value for WML code set_max_range, documented in https://wiki.wesnoth.org/EffectWML
			"$number max range",
			{{"number", set_max_range.str()}}));
	}

	if(!increase_max_range.empty()) {
		desc.emplace_back(VGETTEXT(
			// TRANSLATORS: Current value for WML code increase_max_range, documented in https://wiki.wesnoth.org/EffectWML
			"<span color=\"$color\">$number_or_percent</span> max range",
			format_modifier(increase_max_range)));
	}

	if(!increase_damage.empty()) {
		desc.emplace_back(VNGETTEXT(
			// TRANSLATORS: Current value for WML code increase_damage, documented in https://wiki.wesnoth.org/EffectWML
			"<span color=\"$color\">$number_or_percent</span> damage",
			"<span color=\"$color\">$number_or_percent</span> damage",
			increase_damage.to_int(),
			format_modifier(increase_damage)));
	}

	if(!set_damage.empty()) {
		// TRANSLATORS: Current value for WML code set_damage, documented in https://wiki.wesnoth.org/EffectWML
		desc.emplace_back(VNGETTEXT(
			"$number damage",
			"$number damage",
			set_damage.to_int(),
			{{"number", set_damage.str()}}));
	}

	if(!increase_attacks.empty()) {
		desc.emplace_back(VNGETTEXT(
			// TRANSLATORS: Current value for WML code increase_attacks, documented in https://wiki.wesnoth.org/EffectWML
			"<span color=\"$color\">$number_or_percent</span> strike",
			"<span color=\"$color\">$number_or_percent</span> strikes",
			increase_attacks.to_int(),
			format_modifier(increase_attacks)));
	}

	if(!set_attacks.empty()) {
		desc.emplace_back(VNGETTEXT(
			// TRANSLATORS: Current value for WML code set_attacks, documented in https://wiki.wesnoth.org/EffectWML
			"$number strike",
			"$number strikes",
			set_attacks.to_int(),
			{{"number", set_attacks.str()}}));
	}

	if(!set_accuracy.empty()) {
		desc.emplace_back(VGETTEXT(
			// TRANSLATORS: Current value for WML code set_accuracy, documented in https://wiki.wesnoth.org/EffectWML
			"$number| accuracy",
			{{"number", set_accuracy.str()}}));
	}

	if(!increase_accuracy.empty()) {
		desc.emplace_back(VGETTEXT(
			// TRANSLATORS: Current value for WML code increase_accuracy, documented in https://wiki.wesnoth.org/EffectWML
			"<span color=\"$color\">$number_or_percent|%</span> accuracy",
			format_modifier(increase_accuracy)));
	}

	if(!set_parry.empty()) {
		desc.emplace_back(VGETTEXT(
			// TRANSLATORS: Current value for WML code set_parry, documented in https://wiki.wesnoth.org/EffectWML
			"$number parry",
			{{"number", set_parry.str()}}));
	}

	if(!increase_parry.empty()) {
		desc.emplace_back(VGETTEXT(
			// TRANSLATORS: Current value for WML code increase_parry, documented in https://wiki.wesnoth.org/EffectWML
			"<span color=\"$color\">$number_or_percent</span> parry",
			format_modifier(increase_parry)));
	}

	if(!set_movement.empty()) {
		desc.emplace_back(VNGETTEXT(
			// TRANSLATORS: Current value for WML code set_movement_used, documented in https://wiki.wesnoth.org/EffectWML
			"$number movement point",
			"$number movement points",
			set_movement.to_int(),
			{{"number", set_movement.str()}}));
	}

	if(!increase_movement.empty()) {
		desc.emplace_back(VNGETTEXT(
			// TRANSLATORS: Current value for WML code increase_movement_used, documented in https://wiki.wesnoth.org/EffectWML
			"<span color=\"$color\">$number_or_percent</span> movement point",
			"<span color=\"$color\">$number_or_percent</span> movement points",
			increase_movement.to_int(),
			format_modifier(increase_movement)));
	}

	if(!set_attacks_used.empty()) {
		desc.emplace_back(VNGETTEXT(
			// TRANSLATORS: Current value for WML code set_attacks_used, documented in https://wiki.wesnoth.org/EffectWML
			"$number attack used",
			"$number attacks used",
			set_attacks_used.to_int(),
			{{"number", set_attacks_used.str()}}));
	}

	if(!increase_attacks_used.empty()) {
		desc.emplace_back(VNGETTEXT(
			// TRANSLATORS: Current value for WML code increase_attacks_used, documented in https://wiki.wesnoth.org/EffectWML
			"<span color=\"$color\">$number_or_percent</span> attack used",
			"<span color=\"$color\">$number_or_percent</span> attacks used",
			increase_attacks_used.to_int(),
			format_modifier(increase_attacks_used)));
	}

	return utils::format_conjunct_list("", desc);
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

attack_type::recursion_guard::recursion_guard(attack_type::recursion_guard&& other) noexcept
{
	std::swap(parent, other.parent);
}

attack_type::recursion_guard::operator bool() const {
	return bool(parent);
}

attack_type::recursion_guard& attack_type::recursion_guard::operator=(attack_type::recursion_guard&& other) noexcept
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
	cfg.add_child("specials", specials_cfg());
}



int attack_type::composite_value(const active_ability_list& abil_list, int base_value) const
{
	return unit_abilities::effect(abil_list, base_value, shared_from_this()).get_composite_value();
}


/**
 * Calculates the number of attacks this weapon has, considering specials.
 * This returns two numbers because of the swarm special. The actual number of
 * attacks depends on the unit's health and should be:
 *   min_attacks + (max_attacks - min_attacks) * (current hp) / (max hp)
 * c.f. swarm_blows()
 */
void attack_type::modified_attacks(unsigned& min_attacks,
	unsigned& max_attacks) const
{
	// Apply [attacks].
	int attacks_value = composite_value(get_specials_and_abilities("attacks"), num_attacks());

	if (attacks_value < 0) {
		attacks_value = 0;
		ERR_NG << "negative number of strikes after applying weapon specials";
	}

	// Apply [swarm].
	active_ability_list swarm_specials = get_specials_and_abilities("swarm");
	if (!swarm_specials.empty()) {
		min_attacks = std::max<int>(0, swarm_specials.highest("swarm_attacks_min").first);
		max_attacks = std::max<int>(0, swarm_specials.highest("swarm_attacks_max", attacks_value).first);
	}
	else {
		min_attacks = max_attacks = attacks_value;
	}
}

std::string attack_type::select_replacement_type(const active_ability_list& damage_type_list) const
{
	std::map<std::string, unsigned int> type_count;
	unsigned int max = 0;
	for (auto& i : damage_type_list) {
		const config& c = i.ability_cfg();
		if (c.has_attribute("replacement_type")) {
			std::string type = c["replacement_type"].str();
			unsigned int count = ++type_count[type];
			if ((count > max)) {
				max = count;
			}
		}
	}

	if (type_count.empty()) return type();

	std::vector<std::string> type_list;
	for (auto& i : type_count) {
		if (i.second == max) {
			type_list.push_back(i.first);
		}
	}

	if (type_list.empty()) return type();

	return type_list.front();
}

std::pair<std::string, int> attack_type::select_alternative_type(const active_ability_list& damage_type_list, const active_ability_list& resistance_list) const
{
	std::map<std::string, int> type_res;
	int max_res = INT_MIN;
	if (other_) {
		for (auto& i : damage_type_list) {
			const config& c = i.ability_cfg();
			if (c.has_attribute("alternative_type")) {
				std::string type = c["alternative_type"].str();
				if (type_res.count(type) == 0) {
					type_res[type] = (*other_).resistance_value(resistance_list, type);
					max_res = std::max(max_res, type_res[type]);
				}
			}
		}
	}

	if (type_res.empty()) return { "", INT_MIN };

	std::vector<std::string> type_list;
	for (auto& i : type_res) {
		if (i.second == max_res) {
			type_list.push_back(i.first);
		}
	}
	if (type_list.empty()) return { "", INT_MIN };

	return { type_list.front(), max_res };
}

/**
 * The type of attack used and the resistance value that does the most damage.
 */
std::pair<std::string, int> attack_type::effective_damage_type() const
{
	if (attack_empty()) {
		return { "", 100 };
	}
	active_ability_list resistance_list;
	if (other_) {
		resistance_list = (*other_).get_abilities_weapons("resistance", other_loc_, other_attack_, shared_from_this());
		utils::erase_if(resistance_list, [&](const active_ability& i) {
			return (!(i.ability_cfg()["active_on"].empty() || (!is_attacker_ && i.ability_cfg()["active_on"] == "offense") || (is_attacker_ && i.ability_cfg()["active_on"] == "defense")));
			});
	}
	active_ability_list damage_type_list = get_specials_and_abilities("damage_type");
	int res = other_ ? (*other_).resistance_value(resistance_list, type()) : 100;
	if (damage_type_list.empty()) {
		return { type(), res };
	}
	std::string replacement_type = select_replacement_type(damage_type_list);
	std::pair<std::string, int> alternative_type = select_alternative_type(damage_type_list, resistance_list);

	if (other_) {
		res = replacement_type != type() ? (*other_).resistance_value(resistance_list, replacement_type) : res;
		replacement_type = alternative_type.second > res ? alternative_type.first : replacement_type;
		res = std::max(res, alternative_type.second);
	}
	return { replacement_type, res };
}

/**
 * Return a type()/replacement_type and a list of alternative_types that should be displayed in the selected unit's report.
 */
std::pair<std::string, std::set<std::string>> attack_type::damage_types() const
{
	active_ability_list damage_type_list = get_specials_and_abilities("damage_type");
	std::set<std::string> alternative_damage_types;
	if (damage_type_list.empty()) {
		return { type(), alternative_damage_types };
	}
	std::string replacement_type = select_replacement_type(damage_type_list);
	for (auto& i : damage_type_list) {
		const config& c = i.ability_cfg();
		if (c.has_attribute("alternative_type")) {
			alternative_damage_types.insert(c["alternative_type"].str());
		}
	}

	return { replacement_type, alternative_damage_types };
}

/**
 * Returns the damage per attack of this weapon, considering specials.
 */
double attack_type::modified_damage() const
{
	double damage_value = unit_abilities::effect(get_specials_and_abilities("damage"), damage(), shared_from_this()).get_composite_double_value();
	return damage_value;
}

int attack_type::modified_chance_to_hit(int cth) const
{
	int parry = other_attack_ ? other_attack_->parry() : 0;
	active_ability_list chance_to_hit_list = get_specials_and_abilities("chance_to_hit");
	cth = std::clamp(cth + accuracy_ - parry, 0, 100);
	return composite_value(chance_to_hit_list, cth);
}
