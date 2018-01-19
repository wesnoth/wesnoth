/*
   Copyright (C) 2010 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
#include "config.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "formula/variant.hpp"
#include "game_board.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "resources.hpp"
#include "units/abilities.hpp"
#include "units/unit.hpp"

#include <iomanip>

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(attack_predictions)

const unsigned int attack_predictions::graph_width = 270;
const unsigned int attack_predictions::graph_height = 170;
const unsigned int attack_predictions::graph_max_rows = 10;

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
	// Each data widget in this dialog has its id prefixed by either of these identifiers.
	const std::string widget_id_prefix = attacker.stats_.is_attacker ? "attacker" : "defender";

	const auto get_prefixed_widget_id = [&widget_id_prefix](const std::string& id) {
		return (formatter() << widget_id_prefix << "_" << id).str();
	};

	// Helpers for setting or hiding labels
	const auto set_label_helper = [&](const std::string& id, const std::string& value) {
		find_widget<label>(&window, get_prefixed_widget_id(id), false).set_label(value);
	};

	const auto hide_label_helper = [&](const std::string& id) {
		find_widget<label>(&window, get_prefixed_widget_id(id), false).set_visible(widget::visibility::invisible);
		find_widget<label>(&window, get_prefixed_widget_id(id) + "_label" , false).set_visible(widget::visibility::invisible);
	};

	std::stringstream ss;

	//
	// Always visible fields
	//

	// Unscathed probability
	const color_t ndc_color = game_config::red_to_green(attacker.combatant_.untouched * 100);

	ss << font::span_color(ndc_color) << get_probability_string(attacker.combatant_.untouched) << "</span>";
	set_label_helper("chance_unscathed", ss.str());

	// HP probability graph
	drawing& graph_widget = find_widget<drawing>(&window, get_prefixed_widget_id("hp_graph"), false);
	draw_hp_graph(graph_widget, attacker, defender);

	//
	// Weapon detail fields (only shown if a weapon is present)
	//

	if(!attacker.stats_.weapon) {
		set_label_helper("base_damage", _("No usable weapon"));

		// FIXME: would rather have a list somewhere that I can loop over instead of hardcoding...
		hide_label_helper("tod_modifier");
		hide_label_helper("leadership_modifier");
		hide_label_helper("slowed_modifier");

		return;
	}

	ss.str("");

	// Set specials context (for safety, it should not have changed normally).
	const_attack_ptr weapon = attacker.stats_.weapon;
	weapon->set_specials_context(attacker.unit_.get_location(), defender.unit_.get_location(), attacker.stats_.is_attacker, defender.stats_.weapon);

	// Get damage modifiers.
	unit_ability_list dmg_specials = weapon->get_specials("damage");
	unit_abilities::effect dmg_effect(dmg_specials, weapon->damage(), attacker.stats_.backstab_pos);

	// Get the SET damage modifier, if any.
	auto set_dmg_effect = std::find_if(dmg_effect.begin(), dmg_effect.end(),
		[](const unit_abilities::individual_effect& e) { return e.type == unit_abilities::SET; }
	);

	// Either user the SET modifier or the base weapon damage.
	if(set_dmg_effect == dmg_effect.end()) {
		ss << weapon->damage() << " (<i>" << weapon->name() << "</i>)";
	} else {
		assert(set_dmg_effect->ability);
		ss << set_dmg_effect->value << " (<i>" << (*set_dmg_effect->ability)["name"] << "</i>)";
	}

	// Process the ADD damage modifiers.
	for(const auto& e : dmg_effect) {
		if(e.type == unit_abilities::ADD) {
			ss << "\n";

			if(e.value >= 0) {
				ss << '+';
			}

			ss << e.value;
			ss << " (<i>" << (*e.ability)["name"] << "</i>)";
		}
	}

	// Process the MUL damage modifiers.
	for(const auto& e : dmg_effect) {
		if(e.type == unit_abilities::MUL) {
			ss << "\n";
			ss << font::unicode_multiplication_sign << (e.value / 100);

			if(e.value % 100) {
				ss << "." << ((e.value % 100) / 10);
				if(e.value % 10) {
					ss << (e.value % 10);
				}
			}

			ss << " (<i>" << (*e.ability)["name"] << "</i>)";
		}
	}

	set_label_helper("base_damage", ss.str());

	ss.str("");

	// Resistance modifier.
	const int resistance_modifier = defender.unit_.damage_from(*weapon, !attacker.stats_.is_attacker, defender.unit_.get_location());
	if(resistance_modifier != 100) {
		if(attacker.stats_.is_attacker) {
			ss << _("Defender resistance vs") << " ";
		} else {
			ss << _("Attacker vulnerability vs") << " ";
		}

		ss << string_table["type_" + weapon->type()];

		set_label_helper("resis_label", ss.str());

		ss.str("");
		ss << font::unicode_multiplication_sign << (resistance_modifier / 100) << "." << ((resistance_modifier % 100) / 10);

		set_label_helper("resis", ss.str());
	}

	ss.str("");

	// TODO: color format the modifiers

	// Time of day modifier.
	const unit& u = attacker.unit_;

	const int tod_modifier = combat_modifier(resources::gameboard->units(), resources::gameboard->map(),
		u.get_location(), u.alignment(), u.is_fearless());

	if(tod_modifier != 0) {
		set_label_helper("tod_modifier", utils::signed_percent(tod_modifier));
	} else {
		hide_label_helper("tod_modifier");
	}

	// Leadership bonus.
	const int leadership_bonus = under_leadership(resources::gameboard->units(), attacker.unit_.get_location()).first;

	if(leadership_bonus != 0) {
		set_label_helper("leadership_modifier", utils::signed_percent(leadership_bonus));
	} else {
		hide_label_helper("leadership_modifier");
	}

	// Slowed penalty.
	if(attacker.stats_.is_slowed) {
		set_label_helper("slowed_modifier", "/ 2");
	} else {
		hide_label_helper("slowed_modifier");
	}

	// Total damage.
	const int base_damage = weapon->damage();

	color_t dmg_color = font::weapon_color;
	if(attacker.stats_.damage > base_damage) {
		dmg_color = font::good_dmg_color;
	} else if(attacker.stats_.damage < base_damage) {
		dmg_color = font::bad_dmg_color;
	}

	ss << font::span_color(dmg_color) << attacker.stats_.damage << "</span>"
	   << font::weapon_numbers_sep    << attacker.stats_.num_blows;

	set_label_helper("total_damage", ss.str());

	// Chance to hit
	const color_t cth_color = game_config::red_to_green(attacker.stats_.chance_to_hit);

	ss.str("");
	ss << font::span_color(cth_color) << attacker.stats_.chance_to_hit << "%</span>";

	set_label_helper("chance_to_hit", ss.str());
}

void attack_predictions::draw_hp_graph(drawing& hp_graph, const combatant_data& attacker, const combatant_data& defender)
{
	// Font size. If you change this, you must update the separator space.
	// TODO: probably should remove this.
	const int fs = font::SIZE_SMALL;

	// Space before HP separator.
	const int hp_sep = 30;

	// Space after percentage separator.
	const int percent_sep = 50;

	// Bar space between both separators.
	const int bar_space = graph_width - hp_sep - percent_sep - 4;

	// Set some variables for the WML portion of the graph to use.
	canvas& hp_graph_canvas = hp_graph.get_drawing_canvas();

	hp_graph_canvas.set_variable("hp_column_width", wfl::variant(hp_sep));
	hp_graph_canvas.set_variable("chance_column_width", wfl::variant(percent_sep));

	config cfg, shape;

	int i = 0;

	// Draw the rows (lower HP values are at the bottom).
	for(const auto& probability : get_hitpoint_probabilities(attacker.combatant_.hp_dist)) {

		// Get the HP and probability.
		int hp; double prob;
		std::tie(hp, prob) = probability;

		color_t row_color;

		// Death line is red.
		if(hp == 0) {
			row_color = {229, 0, 0};
		}

		// Below current hitpoints value is orange.
		else if(hp < static_cast<int>(attacker.stats_.hp)) {
			// Stone is grey.
			if(defender.stats_.petrifies) {
				row_color = {154, 154, 154};
			} else {
				row_color = {244, 201, 0};
			}
		}

		// Current hitpoints value and above is green.
		else {
			row_color = {8, 202, 0};
		}

		shape["text"] = hp;
		shape["x"] = 4;
		shape["y"] = 2 + (fs + 2) * i;
		shape["w"] = "(text_width)";
		shape["h"] = "(text_height)";
		shape["font_size"] = 12;
		shape["color"] = "255, 255, 255, 255";
		shape["text_alignment"] = "(text_alignment)";

		cfg.add_child("text", shape);

		shape.clear();
		shape["text"] = get_probability_string(prob);
		shape["x"] = graph_width - percent_sep + 2;
		shape["y"] = 2 + (fs + 2) * i;
		shape["w"] = "(text_width)";
		shape["h"] = "(text_height)";
		shape["font_size"] = 12;
		shape["color"] = "255, 255, 255, 255";
		shape["text_alignment"] = "(text_alignment)";

		cfg.add_child("text", shape);

		const int bar_len = std::max(static_cast<int>((prob * (bar_space - 4)) + 0.5), 2);

		const SDL_Rect bar_rect_1 {
			hp_sep + 4,
			6 + (fs + 2) * i,
			bar_len,
			8
		};

		shape.clear();
		shape["x"] = bar_rect_1.x;
		shape["y"] = bar_rect_1.y;
		shape["w"] = bar_rect_1.w;
		shape["h"] = bar_rect_1.h;
		shape["fill_color"] = row_color.to_rgba_string();

		cfg.add_child("rectangle", shape);

		++i;
	}

	hp_graph.append_drawing_data(cfg);
}

hp_probability_vector attack_predictions::get_hitpoint_probabilities(const std::vector<double>& hp_dist)
{
	hp_probability_vector res, temp_vec;

	// First, extract any relevant probability values
	for(int i = 0; i < static_cast<int>(hp_dist.size()); ++i) {
		const double prob = hp_dist[i];

		// We keep only values above 0.1%.
		if(prob > 0.001) {
			temp_vec.emplace_back(i, prob);
		}
	}

	// Then sort by descending probability.
	std::sort(temp_vec.begin(), temp_vec.end(), [](const hp_probability_t& pair1, const hp_probability_t& pair2) {
		return pair1.second > pair2.second;
	});

	// Take only the highest probability values.;
	std::copy_n(temp_vec.begin(), std::min<int>(graph_max_rows, temp_vec.size()), std::back_inserter(res));

	// Then, we sort the hitpoint values in descending order.
	std::sort(res.begin(), res.end(), [](const hp_probability_t& pair1, const hp_probability_t& pair2) {
		return pair1.first > pair2.first;
	});

	return res;
}

} // namespace dialogs
} // namespace gui2
