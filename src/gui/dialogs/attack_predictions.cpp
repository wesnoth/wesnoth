/*
   Copyright (C) 2010 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/attack_predictions.hpp"

#include "attack_prediction.hpp"
#include "color.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "units/abilities.hpp"
#include "units/unit.hpp"

#include <iomanip>

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(attack_predictions)

attack_predictions::attack_predictions(battle_context& bc, const unit& attacker, const unit& defender)
	: attacker_data_(attacker, bc.get_attacker_combatant(), bc.get_attacker_stats())
	, defender_data_(defender, bc.get_defender_combatant(), bc.get_defender_stats())
{
}

void attack_predictions::pre_show(window& window)
{
	set_data(window, attacker_data_, defender_data_);
	set_data(window, defender_data_, attacker_data_);
}

static std::string get_probability_string(const double prob)
{
	std::ostringstream ss;

	if(prob > 0.9995) {
		ss << "100%";
	} else {
		ss << std::fixed << std::setprecision(1) << 100.0 * prob << '%';
	}

	return ss.str();
}

void attack_predictions::set_data(window& window, const combatant_data& attacker, const combatant_data& defender)
{
	const std::string widget_id_prefix = attacker.stats_.is_attacker ? "attacker_" : "defender_";

	const auto set_label_helper = [&](const std::string& id, const std::string& value) {
		find_widget<label>(&window, widget_id_prefix + id, false).set_label(value);
	};

	std::stringstream ss;

	// With a weapon.
	if(attacker.stats_.weapon) {
		// Set specials context (for safety, it should not have changed normally).
		const attack_type* weapon = attacker.stats_.weapon;
		weapon->set_specials_context(attacker.unit_.get_location(), defender.unit_.get_location(), attacker.stats_.is_attacker, defender.stats_.weapon);

		// Get damage modifiers.
		unit_ability_list dmg_specials = weapon->get_specials("damage");
		unit_abilities::effect dmg_effect(dmg_specials, weapon->damage(), attacker.stats_.backstab_pos);

		// Get the SET damage modifier, if any.
		auto set_dmg_effect =
			std::find_if(dmg_effect.begin(), dmg_effect.end(), [](const unit_abilities::individual_effect& e) { return e.type == unit_abilities::SET; });

		// Either user the SET modifier or the base weapon damage.
		if(set_dmg_effect == dmg_effect.end()) {
			// TODO: formatting
			ss << weapon->name() << ": " << weapon->damage();
		} else {
			assert(set_dmg_effect->ability);
			ss << (*set_dmg_effect->ability)["name"] << ": " << set_dmg_effect->value;
		}

		// Process the ADD damage modifiers.
		for(const auto& e : dmg_effect) {
			if(e.type == unit_abilities::ADD) {
				ss << "\n";
				ss << (*e.ability)["name"] << ": ";

				if(e.value >= 0) {
					ss << '+';
				}

				ss << e.value;
			}
		}

		// Process the MUL damage modifiers.
		for(const auto& e : dmg_effect) {
			if(e.type == unit_abilities::MUL) {
				ss << "\n";
				ss << (*e.ability)["name"] << ": " << font::unicode_multiplication_sign << (e.value / 100);

				if(e.value % 100) {
					ss << "." << ((e.value % 100) / 10);
					if(e.value % 10) {
						ss << (e.value % 10);
					}
				}
			}
		}

		set_label_helper("attack", ss.str());

#if 0
		// Time of day modifier.
		int tod_modifier = combat_modifier(resources::gameboard->units(), resources::gameboard->map(), attacker.unit_.get_location(), u.alignment(), u.is_fearless());
		if(tod_modifier != 0) {
			left_strings.push_back(_("Time of day"));
			str.str("");
			ss << utils::signed_percent(tod_modifier);
			right_strings.push_back(str.str());
		}

		// Leadership bonus.
		int leadership_bonus = 0;
		under_leadership(resources::gameboard->units(), attacker.unit_.get_location(), &leadership_bonus);
		if(leadership_bonus != 0) {
			left_strings.push_back(_("Leadership"));
			str.str("");
			ss << utils::signed_percent(leadership_bonus);
			right_strings.push_back(str.str());
		}
#endif

		ss.str("");

		// Resistance modifier.
		const int resistance_modifier = defender.unit_.damage_from(*weapon, !attacker.stats_.is_attacker, defender.unit_.get_location());
		if(resistance_modifier != 100) {
			ss << string_table["type_" + weapon->type()] << ": ";
			ss << font::unicode_multiplication_sign << (resistance_modifier / 100) << "." << ((resistance_modifier % 100) / 10);

			set_label_helper("resis", ss.str());
		}

		// Slowed penalty.
#if 0
		if(attacker.stats_.is_slowed) {
			left_strings.push_back(_("Slowed"));
			right_strings.push_back("/ 2");
		}
#endif

		ss.str("");

		// Total damage.
		const int base_damage = weapon->damage();

		color_t dmg_color = font::weapon_color;
		if(attacker.stats_.damage > base_damage) {
			dmg_color = font::good_dmg_color;
		} else if(attacker.stats_.damage < base_damage) {
			dmg_color = font::bad_dmg_color;
		}

		const color_t cth_color = game_config::red_to_green(attacker.stats_.chance_to_hit);

		ss << font::span_color(dmg_color) << attacker.stats_.damage << "</span>"
		   << font::weapon_numbers_sep    << attacker.stats_.num_blows << " ("
		   << font::span_color(cth_color) << attacker.stats_.chance_to_hit << "%</span>)";

		set_label_helper("damage", ss.str());
	} else {
		// Without a weapon.
		set_label_helper("attack", _("No usable weapon"));
	}

	const color_t ndc_color = game_config::red_to_green(attacker.combatant_.untouched * 10);

	// Unscathed probability.
	set_label_helper("no_damage_chance", (formatter() << font::span_color(ndc_color) << get_probability_string(attacker.combatant_.untouched) << "</span>").str());
}

}
}
