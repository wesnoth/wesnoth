/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/statistics_dialog.hpp"

#include "actions/attack.hpp" // for battle_context_unit_stats
#include "font/constants.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "team.hpp"
#include "units/types.hpp"
#include "utils/functional.hpp"

#include <iomanip>
#include <iostream>
#include <memory>

// TODO duplicated from attack_predictions.cpp
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

namespace gui2
{
namespace dialogs
{
REGISTER_DIALOG(statistics_dialog)

statistics_dialog::statistics_dialog(const team& current_team)
	: current_team_(current_team)
	, campaign_(statistics::calculate_stats(current_team.save_id_or_number()))
	, scenarios_(statistics::level_stats(current_team.save_id_or_number()))
	, selection_index_(scenarios_.size()) // The extra All Scenarios menu entry makes size() a valid initial index.
	, main_stat_table_()
{
	set_restore(true);
}

void statistics_dialog::pre_show(window& window)
{
	//
	// Set title
	//
	label& title = find_widget<label>(&window, "title", false);
	title.set_label((formatter() << title.get_label() << " (" << current_team_.side_name() << ")").str());

	//
	// Set up scenario menu
	//
	std::vector<config> menu_items;

	// Keep this first!
	menu_items.emplace_back("label", _("All Scenarios"));

	for(const auto& scenario : scenarios_) {
		menu_items.emplace_back("label", *scenario.first);
	}

	menu_button& scenario_menu = find_widget<menu_button>(&window, "scenario_menu", false);

	scenario_menu.set_values(menu_items, selection_index_);

	connect_signal_notify_modified(scenario_menu,
		std::bind(&statistics_dialog::on_scenario_select, this, std::ref(window)));

	//
	// Set up primary stats list
	//
	listbox& stat_list = find_widget<listbox>(&window, "stats_list_main", false);

	connect_signal_notify_modified(stat_list,
		std::bind(&statistics_dialog::on_primary_list_select, this, std::ref(window)));

	update_lists(window);
}

inline const statistics::stats& statistics_dialog::current_stats()
{
	return selection_index_ == 0 ? campaign_ : *scenarios_[selection_index_ - 1].second;
}

void statistics_dialog::add_stat_row(window& window, const std::string& type, const statistics::stats::str_int_map& value, const bool has_cost)
{
	listbox& stat_list = find_widget<listbox>(&window, "stats_list_main", false);

	std::map<std::string, string_map> data;
	string_map item;

	item["label"] = type;
	data.emplace("stat_type", item);

	item["label"] = std::to_string(statistics::sum_str_int_map(value));
	data.emplace("stat_detail", item);

	item["label"] = has_cost ? std::to_string(statistics::sum_cost_str_int_map(value)) : font::unicode_em_dash;
	data.emplace("stat_cost", item);

	stat_list.add_row(data);

	main_stat_table_.push_back(&value);
}

static const auto& spacer = "    ";
void statistics_dialog::add_damage_row(
		window& window,
		const std::string& type,
		const long long& damage,
		const long long& expected,
		const long long& turn_damage,
		const long long& turn_expected,
		const bool show_this_turn)
{
	listbox& damage_list = find_widget<listbox>(&window, "stats_list_damage", false);

	std::map<std::string, string_map> data;
	string_map item;

	item["label"] = type;
	data.emplace("damage_type", item);

	const int shift = statistics::stats::decimal_shift;

	const long long dsa = shift * damage      - expected;
	const long long dst = shift * turn_damage - turn_expected;

	const long long shifted = ((expected * 20) + shift) / (2 * shift);
	std::ostringstream str;
	str << damage << " / "
		<< static_cast<double>(shifted) * 0.1
		<< spacer // TODO: should probably make this two columns
		<< (((dsa < 0) ^ (expected < 0)) ? "" : "+")
		<< (expected == 0 ? 0 : 100 * dsa / expected) << '%';

	item["label"] = str.str();
	data.emplace("damage_overall", item);

	str.str("");

	if(show_this_turn) {
		const long long turn_shifted = ((turn_expected * 20) + shift) / (2 * shift);
		str << turn_damage << " / "
			<< static_cast<double>(turn_shifted) * 0.1
			<< spacer // TODO: should probably make this two columns
			<< (((dst < 0) ^ (turn_expected < 0)) ? "" : "+")
			<< (turn_expected == 0 ? 0 : 100 * dst / turn_expected) << '%';

		item["label"] = str.str();
		data.emplace("damage_this_turn", item);
	}

	damage_list.add_row(data);
}

// Return the string to use in the "Hits" table, showing actual and expected number of hits.
static std::string tally(const std::map<int, struct statistics::stats::by_cth_t>& by_cth)
{
	unsigned int overall_hits = 0;
	double expected_hits = 0;
	unsigned int overall_strikes = 0;

	for(const auto& i : by_cth) {
		int cth = i.first;
		overall_hits += i.second.hits;
		expected_hits += (cth * 0.01) * i.second.strikes;
		overall_strikes += i.second.strikes;
	}

	std::ostringstream str;
	str << overall_hits << " / " << expected_hits;

	// Compute the a priori probability of this actual result, by simulating many attacks against a single defender.
	{
		config defender_cfg(
			"id", "statistics_dialog_dummy_defender",
			"hide_help", "yes",
			"do_not_list", "yes",
			"hitpoints", overall_strikes
		);
		unit_type defender_type(defender_cfg);
		unit_types.build_unit_type(defender_type, unit_type::BUILD_STATUS::FULL);

		battle_context_unit_stats defender_bc(&defender_type, nullptr, false, nullptr, nullptr, 0 /* not used */);
		std::unique_ptr<combatant> current_defender(new combatant(defender_bc));

		for(const auto& i : by_cth) {
			int cth = i.first;
			config attacker_cfg(
				"id", "statistics_dialog_dummy_attacker" + std::to_string(cth),
				"hide_help", "yes",
				"do_not_list", "yes",
				"hitpoints", 1
			);
			unit_type attacker_type(attacker_cfg);
			unit_types.build_unit_type(attacker_type, unit_type::BUILD_STATUS::FULL);

			attack_ptr attack = std::make_shared<attack_type>(config(
				"type", "blade",
				"range", "melee",
				"name", "dummy attack",
				"damage", 1,
				"number", i.second.strikes
			));

			battle_context_unit_stats attacker_bc(&attacker_type, attack, true, &defender_type, nullptr, 100 - cth);
			defender_bc = battle_context_unit_stats(&defender_type, nullptr, false, &attacker_type, attack, 0 /* not used */);

			// Update current_defender with the new defender_bc.
			combatant attacker(attacker_bc);
			current_defender.reset(new combatant(*current_defender, defender_bc));

			attacker.fight(*current_defender);
		}

		const std::vector<double>& final_hp_dist = current_defender->hp_dist;
		const auto& chance_of_exactly_N_hits = [&final_hp_dist](int N) { return final_hp_dist[final_hp_dist.size() - 1 - N]; };

		double probability = 0.0;
		if(overall_hits == expected_hits) {
			probability = chance_of_exactly_N_hits(overall_hits);
		}
		else if (overall_hits > expected_hits) {
			for(unsigned int i = overall_hits; i < final_hp_dist.size(); i++)
				probability += chance_of_exactly_N_hits(i);
		}
		else {
			for(unsigned int i = 0; i <= overall_hits; i++)
				probability += chance_of_exactly_N_hits(i);
		}
		// TODO: document for users what this value is.
		str << spacer // TODO: should probably make this two columns
			<< font::span_color(game_config::red_to_green(probability * 100.0, true)) << get_probability_string(probability) << "</span>";
	}

	return str.str();
}

void statistics_dialog::add_hits_row(
		window& window,
		const std::string& type,
		const std::map<int, struct statistics::stats::by_cth_t>& by_cth,
		const std::map<int, struct statistics::stats::by_cth_t>& turn_by_cth,
		const bool show_this_turn)
{
	listbox& hits_list = find_widget<listbox>(&window, "stats_list_hits", false);

	std::map<std::string, string_map> data;
	string_map item;

	item["label"] = type;
	data.emplace("hits_type", item);
	item["label"] = tally(by_cth);
	data.emplace("hits_overall", item);

	if(show_this_turn) {
		item["label"] = tally(turn_by_cth);
		data.emplace("hits_this_turn", item);
	}

	hits_list.add_row(data);
}

void statistics_dialog::update_lists(window& window)
{
	//
	// Update primary stats list
	//
	listbox& stat_list = find_widget<listbox>(&window, "stats_list_main", false);
	const int last_selected_stat_row = stat_list.get_selected_row();

	stat_list.clear();
	main_stat_table_.clear();

	const statistics::stats& stats = current_stats();

	add_stat_row(window, _("Recruits"),     stats.recruits);
	add_stat_row(window, _("Recalls"),      stats.recalls);
	add_stat_row(window, _("Advancements"), stats.advanced_to, false);
	add_stat_row(window, _("Losses"),       stats.deaths);
	add_stat_row(window, _("Kills"),        stats.killed);

	// Reselect previously selected row. Do this *before* calling on_primary_list_select.
	if(last_selected_stat_row != -1) {
		stat_list.select_row(last_selected_stat_row);
	}

	// Update unit count list
	on_primary_list_select(window);

	//
	// Update damage stats list
	//
	const bool show_this_turn = selection_index_ == scenarios_.size();

	listbox& damage_list = find_widget<listbox>(&window, "stats_list_damage", false);

	damage_list.clear();

	listbox& hits_list = find_widget<listbox>(&window, "stats_list_hits", false);
	hits_list.clear();

	add_damage_row(window, _("Inflicted"),
		stats.damage_inflicted,
		stats.expected_damage_inflicted,
		stats.turn_damage_inflicted,
		stats.turn_expected_damage_inflicted,
		show_this_turn
	);
	add_hits_row(window, _("Inflicted"),
		stats.by_cth_inflicted,
		stats.turn_by_cth_inflicted,
		show_this_turn
	);

	add_damage_row(window, _("Taken"),
		stats.damage_taken,
		stats.expected_damage_taken,
		stats.turn_damage_taken,
		stats.turn_expected_damage_taken,
		show_this_turn
	);
	add_hits_row(window, _("Taken"),
		stats.by_cth_taken,
		stats.turn_by_cth_taken,
		show_this_turn
	);
}

void statistics_dialog::on_scenario_select(window& window)
{
	const std::size_t new_index = find_widget<menu_button>(&window, "scenario_menu", false).get_value();

	if(selection_index_ != new_index) {
		selection_index_ = new_index;
		update_lists(window);
	}
}

void statistics_dialog::on_primary_list_select(window& window)
{
	const int selected_row = find_widget<listbox>(&window, "stats_list_main", false).get_selected_row();
	if(selected_row == -1) {
		return;
	}

	listbox& unit_list = find_widget<listbox>(&window, "stats_list_units", false);

	unit_list.clear();

	for(const auto& i : *main_stat_table_[selected_row]) {
		const unit_type* type = unit_types.find(i.first);
		if(!type) {
			continue;
		}

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = (formatter() << type->image() << "~RC(" << type->flag_rgb() << ">" << current_team_.color() << ")").str();
		data.emplace("unit_image", item);

		// Note: the x here is a font::unicode_multiplication_sign
		item["label"] = VGETTEXT("$count|× $name", {{"count", std::to_string(i.second)}, {"name", type->type_name()}});
		data.emplace("unit_name", item);

		unit_list.add_row(data);
	}
}

} // namespace dialogs
} // namespace gui2
