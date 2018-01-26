/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "font/constants.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "team.hpp"
#include "units/types.hpp"

#include "utils/functional.hpp"
#include <iostream>

namespace gui2
{
namespace dialogs
{

static bool use_campaign = false;

REGISTER_DIALOG(statistics_dialog)

statistics_dialog::statistics_dialog(const team& current_team)
	: current_team_(current_team)
	, campaign_(statistics::calculate_stats(current_team.save_id()))
	, scenarios_(statistics::level_stats(current_team.save_id()))
	, scenario_index_(scenarios_.size() - 1)
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
	for(const auto& scenario : scenarios_) {
		menu_items.emplace_back(config {"label", *scenario.first});
	}

	menu_button& scenario_menu = find_widget<menu_button>(&window, "scenario_menu", false);

	scenario_menu.set_values(menu_items, scenario_index_);
	scenario_menu.connect_click_handler(std::bind(&statistics_dialog::on_scenario_select, this, std::ref(window)));

	//
	// Set up tab toggle
	//
	listbox& tab_bar = find_widget<listbox>(&window, "tab_bar", false);
	tab_bar.select_row(use_campaign);

	connect_signal_notify_modified(tab_bar,
		std::bind(&statistics_dialog::on_tab_select, this, std::ref(window)));

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
	return use_campaign ? campaign_ : *scenarios_[scenario_index_].second;
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
		<< "    " // TODO: should probably make this two columns
		<< (((dsa < 0) ^ (expected < 0)) ? "" : "+")
		<< (expected == 0 ? 0 : 100 * dsa / expected) << '%';

	item["label"] = str.str();
	data.emplace("damage_overall", item);

	str.str("");

	if(show_this_turn) {
		const long long turn_shifted = ((turn_expected * 20) + shift) / (2 * shift);
		str << turn_damage << " / "
			<< static_cast<double>(turn_shifted) * 0.1
			<< "    " // TODO: should probably make this two columns
			<< (((dst < 0) ^ (turn_expected < 0)) ? "" : "+")
			<< (turn_expected == 0 ? 0 : 100 * dst / turn_expected) << '%';

		item["label"] = str.str();
		data.emplace("damage_this_turn", item);
	}

	damage_list.add_row(data);
}

void statistics_dialog::update_lists(window& window)
{
	//
	// Update primary stats list
	//
	listbox& stat_list = find_widget<listbox>(&window, "stats_list_main", false);

	stat_list.clear();
	main_stat_table_.clear();

	const statistics::stats& stats = current_stats();

	add_stat_row(window, _("Recruits"),     stats.recruits);
	add_stat_row(window, _("Recalls"),      stats.recalls);
	add_stat_row(window, _("Advancements"), stats.advanced_to, false);
	add_stat_row(window, _("Losses"),       stats.deaths);
	add_stat_row(window, _("Kills"),        stats.killed);

	// Update unit count list
	on_primary_list_select(window);

	//
	// Update damage stats list
	//
	const bool show_this_turn = use_campaign || scenario_index_ + 1 == scenarios_.size();

	listbox& damage_list = find_widget<listbox>(&window, "stats_list_damage", false);

	damage_list.clear();

	add_damage_row(window, _("Inflicted"),
		stats.damage_inflicted,
		stats.expected_damage_inflicted,
		stats.turn_damage_inflicted,
		stats.turn_expected_damage_inflicted,
		show_this_turn
	);

	add_damage_row(window, _("Taken"),
		stats.damage_taken,
		stats.expected_damage_taken,
		stats.turn_damage_taken,
		stats.turn_expected_damage_taken,
		show_this_turn
	);
}

void statistics_dialog::on_tab_select(window& window)
{
	const bool is_campaign_tab = find_widget<listbox>(&window, "tab_bar", false).get_selected_row() == 1;

	if(use_campaign != is_campaign_tab) {
		use_campaign = is_campaign_tab;

		update_lists(window);
	}
}

void statistics_dialog::on_scenario_select(window& window)
{
	const size_t new_index = find_widget<menu_button>(&window, "scenario_menu", false).get_value();

	if(scenario_index_ != new_index) {
		scenario_index_ = new_index;

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

		item["label"] = type->type_name();
		data.emplace("unit_name", item);

		item["label"] = (formatter() << i.second << font::unicode_multiplication_sign).str();
		data.emplace("unit_count", item);

		unit_list.add_row(data);
	}
}

} // namespace dialogs
} // namespace gui2
