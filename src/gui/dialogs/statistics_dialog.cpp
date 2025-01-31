/*
	Copyright (C) 2016 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "serialization/markup.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/window.hpp"
#include "team.hpp"
#include "units/types.hpp"

#include <functional>
#include <iomanip>
#include <memory>

// TODO duplicated from attack_predictions.cpp
static std::string get_probability_string(const double prob)
{
	std::ostringstream ss;

	if(prob > 0.9995) {
		ss << "100";
	} else {
		ss << std::fixed << std::setprecision(1) << 100.0 * prob;
	}

	return ss.str();
}

namespace gui2::dialogs
{
REGISTER_DIALOG(statistics_dialog)

statistics_dialog::statistics_dialog(statistics_t& statistics, const team& current_team)
	: modal_dialog(window_id())
	, current_team_(current_team)
	, campaign_(statistics.calculate_stats(current_team.save_id_or_number()))
	, scenarios_(statistics.level_stats(current_team.save_id_or_number()))
	, selection_index_(scenarios_.size()) // The extra All Scenarios menu entry makes size() a valid initial index.
	, main_stat_table_()
{
}

void statistics_dialog::pre_show()
{
	//
	// Set title
	//
	label& title = find_widget<label>("title");
	title.set_label((formatter() << title.get_label() << (current_team_.side_name().empty() ? "" : " (" + current_team_.side_name() + ")")).str());

	//
	// Set up scenario menu
	//
	std::vector<config> menu_items;

	// Keep this first!
	menu_items.emplace_back("label", _("All Scenarios"));

	for(const auto& scenario : scenarios_) {
		menu_items.emplace_back("label", *scenario.first);
	}

	menu_button& scenario_menu = find_widget<menu_button>("scenario_menu");

	scenario_menu.set_values(menu_items, selection_index_);

	connect_signal_notify_modified(scenario_menu,
		std::bind(&statistics_dialog::on_scenario_select, this));

	//
	// Set up primary stats list
	//
	listbox& stat_list = find_widget<listbox>("stats_list_main");

	connect_signal_notify_modified(stat_list,
		std::bind(&statistics_dialog::on_primary_list_select, this));

	update_lists();
}

inline const statistics_t::stats& statistics_dialog::current_stats()
{
	return selection_index_ == 0 ? campaign_ : *scenarios_[selection_index_ - 1].second;
}

void statistics_dialog::add_stat_row(const std::string& type, const statistics_t::stats::str_int_map& value, const bool has_cost)
{
	listbox& stat_list = find_widget<listbox>("stats_list_main");

	widget_data data;
	widget_item item;

	item["label"] = type;
	data.emplace("stat_type", item);

	item["label"] = std::to_string(statistics_t::sum_str_int_map(value));
	data.emplace("stat_detail", item);

	item["label"] = has_cost ? std::to_string(statistics_t::sum_cost_str_int_map(value)) : font::unicode_em_dash;
	data.emplace("stat_cost", item);

	stat_list.add_row(data);

	main_stat_table_.push_back(&value);
}

// Generate the string for the "A + B" column of the damage and hits tables.
static std::ostream& write_actual_and_expected(std::ostream& str, const long long actual, const double expected)
{
	// This is displayed as a sum or difference, not as "actual/expected", to prevent the string in the next column, str2.str(), from being mistaken for the result of the division.
	if(expected == 0) {
		str << "+0% (0 + 0)";
	} else {
		str << (formatter() << std::showpos << std::round((actual - expected) * 100 / expected) << "% (").str();
		str << expected << (actual >= expected ? " + " : " − ")
			<< static_cast<unsigned int>(std::round(std::abs(expected - actual)));
		str << ')';
	}
	return str;
}

void statistics_dialog::add_damage_row(
		const std::string& type,
		const long long& damage,
		const long long& expected,
		const long long& turn_damage,
		const long long& turn_expected,
		const bool show_this_turn)
{
	listbox& damage_list = find_widget<listbox>("stats_list_damage");

	widget_data data;
	widget_item item;

	item["label"] = type;
	data.emplace("damage_type", item);

	static const int shift = statistics_t::stats::decimal_shift;

	const auto damage_str = [](long long damage, long long expected) {
		const long long shifted = ((expected * 20) + shift) / (2 * shift);
		std::ostringstream str;
		write_actual_and_expected(str, damage, static_cast<double>(shifted) * 0.1);
		return str.str();
	};

	item["label"] = damage_str(damage, expected);
	data.emplace("damage_overall", item);

	item["label"] = "";
	data.emplace("overall_score", item);

	if(show_this_turn) {
		label& this_turn_header = find_widget<label>("damage_this_turn_header");
		this_turn_header.set_label(_("This Turn"));

		item["label"] = damage_str(turn_damage, turn_expected);
		data.emplace("damage_this_turn", item);

		item["label"] = "";
		data.emplace("this_turn_score", item);
	} else {
		// TODO: Setting the label to "" causes "This Turn" not to be drawn when changing back to the current scenario view, so set the label to " " (a single space) instead.
		label& this_turn_header = find_widget<label>("damage_this_turn_header");
		this_turn_header.set_label(" ");
	}

	damage_list.add_row(data);
}

// Custom type to allow tally() to return two values.
struct hitrate_table_element
{
	// The string with <actual number of hits>/<expected number of hits>
	std::string hitrate_str;
	// The string with the a priori probability of that result
	std::string pvalue_str;
	// The tooltip of the table cell - shows the actual (empirical) CTH
	std::string tooltip;
};

// Return the strings to use in the "Hits" table, showing actual and expected number of hits.
static hitrate_table_element tally(const statistics_t::stats::hitrate_map& by_cth, const bool more_is_better)
{
	unsigned int overall_hits = 0;
	double expected_hits = 0;
	unsigned int overall_strikes = 0;

	std::ostringstream str, str2, tooltip;

	tooltip << '\n' << '\n' << _("Actual hit rates, by chance to hit:");
	if(by_cth.empty())
		tooltip << '\n' << _("(no attacks have taken place yet)");
	for(const auto& i : by_cth) {
		int cth = i.first;
		overall_hits += i.second.hits;
		expected_hits += (cth * 0.01) * i.second.strikes;
		overall_strikes += i.second.strikes;
		tooltip << "\n" << cth << "%: "
			<< get_probability_string(i.second.hits/static_cast<double>(i.second.strikes))
			<< "% (N=" << i.second.strikes << ")";
	}

	write_actual_and_expected(str, overall_hits, expected_hits);

	// Compute the a priori probability of this actual result, by simulating many attacks against a single defender.
	{
		config defender_cfg(
			"id", "statistics_dialog_dummy_defender",
			"hide_help", true,
			"do_not_list", true,
			"hitpoints", overall_strikes
		);
		unit_type defender_type(defender_cfg);
		unit_types.build_unit_type(defender_type, unit_type::BUILD_STATUS::FULL);

		battle_context_unit_stats defender_bc(&defender_type, nullptr, false, nullptr, nullptr, 0 /* not used */);
		auto current_defender = std::make_unique<combatant>(defender_bc);

		for(const auto& i : by_cth) {
			int cth = i.first;
			config attacker_cfg(
				"id", "statistics_dialog_dummy_attacker" + std::to_string(cth),
				"hide_help", true,
				"do_not_list", true,
				"hitpoints", 1
			);
			unit_type attacker_type(attacker_cfg);
			unit_types.build_unit_type(attacker_type, unit_type::BUILD_STATUS::FULL);

			auto attack = std::make_shared<attack_type>(config(
				"type", "blade",
				"range", "melee",
				"name", "dummy attack",
				"damage", 1,
				"number", i.second.strikes
			));

			battle_context_unit_stats attacker_bc(&attacker_type, attack, true, &defender_type, nullptr, 100 - cth);
			defender_bc = battle_context_unit_stats(&defender_type, nullptr, false, &attacker_type, attack, 0 /* not used */);

			// Update current_defender with the new defender_bc.
			current_defender.reset(new combatant(*current_defender, defender_bc));

			combatant attacker(attacker_bc);
			attacker.fight(*current_defender);
		}

		const std::vector<double>& final_hp_dist = current_defender->hp_dist;
		const auto chance_of_exactly_N_hits = [&final_hp_dist](int n) { return final_hp_dist[final_hp_dist.size() - 1 - n]; };

		// The a priori probability of scoring less hits than the actual number of hits
		// aka "percentile" or "p-value"
		double probability_lt = 0.0;
		for(unsigned int i = 0; i < overall_hits; ++i) {
			probability_lt += chance_of_exactly_N_hits(i);
		}
		// The a priori probability of scoring exactly the actual number of hits
		double probability_eq = chance_of_exactly_N_hits(overall_hits);
		// The a priori probability of scoring more hits than the actual number of hits
		double probability_gt = 1.0 - (probability_lt + probability_eq);

		if(overall_strikes == 0) {
			// Start of turn
			str2 << font::unicode_em_dash;
		} else {
			const auto add_probability = [&str2](double probability, bool more_is_better) {
				str2 << markup::span_color(
					game_config::red_to_green((more_is_better ? probability : 1.0 - probability) * 100.0, true),
					get_probability_string(probability));
			};

			// Take the average. At the end of a scenario or a campaign the sum of
			// probability_lt+probability_gt is very close to 1.0 so the percentile is
			// approximately equal to probability_lt.
			const double percentile = (probability_lt + (1.0 - probability_gt)) / 2.0;
			add_probability(percentile, more_is_better);
		}
	}

	return hitrate_table_element{str.str(), str2.str(), tooltip.str()};
}

void statistics_dialog::add_hits_row(
		const std::string& type,
		const bool more_is_better,
		const statistics_t::stats::hitrate_map& by_cth,
		const statistics_t::stats::hitrate_map& turn_by_cth,
		const bool show_this_turn)
{
	listbox& hits_list = find_widget<listbox>("stats_list_hits");

	widget_data data;
	widget_item item;

	hitrate_table_element element;

	item["label"] = type;
	data.emplace("hits_type", item);

	const auto tooltip_static_part = _(
		"stats dialog^Difference of actual outcome to expected outcome, as a percentage.\n"
		"The first number in parentheses is the expected number of hits inflicted/taken.\n"
		"The sum (or difference) of the two numbers in parentheses is the actual number of hits inflicted/taken.");
	element = tally(by_cth, more_is_better);
	item["tooltip"] = tooltip_static_part + element.tooltip;
	item["label"] = element.hitrate_str;
	data.emplace("hits_overall", item);

	// Don't set the tooltip; it's set in WML.
	data.emplace("overall_score", widget_item { { "label", element.pvalue_str } });

	if(show_this_turn) {
		label& this_turn_header = find_widget<label>("hits_this_turn_header");
		this_turn_header.set_label(_("This Turn"));

		element = tally(turn_by_cth, more_is_better);
		item["tooltip"] = tooltip_static_part + element.tooltip;
		item["label"] = element.hitrate_str;
		data.emplace("hits_this_turn", item);

		// Don't set the tooltip; it's set in WML.
		data.emplace("this_turn_score", widget_item { { "label", element.pvalue_str } });
	} else {
		// TODO: Setting the label to "" causes "This Turn" not to be drawn when changing back to the current scenario view, so set the label to " " (a single space) instead.
		label& this_turn_header = find_widget<label>("hits_this_turn_header");
		this_turn_header.set_label(" ");
	}

	hits_list.add_row(data);
}

void statistics_dialog::update_lists()
{
	//
	// Update primary stats list
	//
	listbox& stat_list = find_widget<listbox>("stats_list_main");
	const int last_selected_stat_row = stat_list.get_selected_row();

	stat_list.clear();
	main_stat_table_.clear();

	const statistics_t::stats& stats = current_stats();

	add_stat_row(_("stats^Recruits"),     stats.recruits);
	add_stat_row(_("Recalls"),      stats.recalls);
	add_stat_row(_("Advancements"), stats.advanced_to, false);
	add_stat_row(_("Losses"),       stats.deaths);
	add_stat_row(_("Kills"),        stats.killed);

	// Reselect previously selected row. Do this *before* calling on_primary_list_select.
	if(last_selected_stat_row != -1) {
		stat_list.select_row(last_selected_stat_row);
	}

	// Update unit count list
	on_primary_list_select();

	//
	// Update damage stats list
	//
	const bool show_this_turn = selection_index_ == scenarios_.size();

	listbox& damage_list = find_widget<listbox>("stats_list_damage");

	damage_list.clear();

	listbox& hits_list = find_widget<listbox>("stats_list_hits");
	hits_list.clear();

	add_damage_row(_("Inflicted"),
		stats.damage_inflicted,
		stats.expected_damage_inflicted,
		stats.turn_damage_inflicted,
		stats.turn_expected_damage_inflicted,
		show_this_turn
	);
	add_hits_row(_("Inflicted"), true,
		stats.by_cth_inflicted,
		stats.turn_by_cth_inflicted,
		show_this_turn
	);

	add_damage_row(_("Taken"),
		stats.damage_taken,
		stats.expected_damage_taken,
		stats.turn_damage_taken,
		stats.turn_expected_damage_taken,
		show_this_turn
	);
	add_hits_row(_("Taken"), false,
		stats.by_cth_taken,
		stats.turn_by_cth_taken,
		show_this_turn
	);
}

void statistics_dialog::on_scenario_select()
{
	const std::size_t new_index = find_widget<menu_button>("scenario_menu").get_value();

	if(selection_index_ != new_index) {
		selection_index_ = new_index;
		update_lists();
	}
}

void statistics_dialog::on_primary_list_select()
{
	const int selected_row = find_widget<listbox>("stats_list_main").get_selected_row();
	if(selected_row == -1) {
		return;
	}

	listbox& unit_list = find_widget<listbox>("stats_list_units");

	unit_list.clear();

	for(const auto& i : *main_stat_table_[selected_row]) {
		const unit_type* type = unit_types.find(i.first);
		if(!type) {
			continue;
		}

		widget_data data;
		widget_item item;

		item["label"] = (formatter() << type->image() << "~RC(" << type->flag_rgb() << ">" << current_team_.color() << ")").str();
		data.emplace("unit_image", item);

		// Note: the x here is a font::unicode_multiplication_sign
		item["label"] = VGETTEXT("$count|× $name", {{"count", std::to_string(i.second)}, {"name", type->type_name()}});
		data.emplace("unit_name", item);

		unit_list.add_row(data);
	}
}

} // namespace dialogs
