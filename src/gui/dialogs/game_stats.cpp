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

#include "gui/dialogs/game_stats.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "display.hpp"
#include "formatter.hpp"
#include "game_board.hpp"
#include "game_classification.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "units/map.hpp"
#include "units/unit.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(game_stats)

game_stats::game_stats(const display_context& board, const int viewing_team, int& selected_index)
	: board_(board)
	, viewing_team_(board_.teams()[viewing_team])
	, selected_index_(selected_index)
{
	for(const auto& team : board_.teams()) {
		team_data_.push_back(board_.calculate_team_data(team));
	}
}

unit_const_ptr game_stats::get_leader(const int side)
{
	unit_map::const_iterator leader = board_.units().find_leader(side);

	if(leader != board_.units().end()) {
		return leader.get_shared_ptr();
	}

	return nullptr;
}

static std::string controller_name(const team& t)
{
	static const std::array<t_string, 3> names {{_("controller^Human"), _("controller^AI"), _("controller^Idle")}};
	return "<span color='#808080'><small>" + names[t.controller().v] + "</small></span>";
}

void game_stats::pre_show(window& window)
{
	listbox& stats_list    = find_widget<listbox>(&window, "game_stats_list", false);
	listbox& settings_list = find_widget<listbox>(&window, "scenario_settings_list", false);

	for(const auto& team : board_.teams()) {
		if(team.hidden()) {
			continue;
		}

		std::map<std::string, string_map> row_data_stats;
		string_map column_stats;

		const bool known = viewing_team_.knows_about_team(team.side() - 1);
		const bool enemy = viewing_team_.is_enemy(team.side());

		const team_data& data = team_data_[team.side() - 1];

		unit_const_ptr leader = get_leader(team.side());

		std::string leader_name;
		std::string leader_image;

		if(leader) {
			const bool fogged = viewing_team_.fogged(leader->get_location());

			// Add leader image. If it's fogged show only a random leader image.
			if(!fogged || known || game_config::debug) {
				leader_image = leader->absolute_image() + leader->image_mods();
				leader_name  = leader->name();
			} else {
				leader_image = formatter() << "units/unknown-unit.png" << "~RC(magenta>" << team.color() << ")";
				leader_name  = _("Unknown");
			}

			if(resources::controller) {
				if(resources::controller->get_classification().campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER) {
					leader_name = team.side_name();
				}
			}

			leader_name = "<span color='" + team::get_side_highlight_pango(team.side() - 1) + "'>" + leader_name + "</span>";
		}

		//
		// Status list
		//
		column_stats["use_markup"] = "true";

		column_stats["label"] = leader_image;
		row_data_stats.emplace("team_leader_image", column_stats);

		column_stats["label"] = leader_name + "\n" + controller_name(team);
		row_data_stats.emplace("team_leader_name", column_stats);

		column_stats["label"] = data.teamname.empty() ? team.team_name() : data.teamname;
		row_data_stats.emplace("team_name", column_stats);

		// Only fill in the rest of the info if the side is known...
		if(known || game_config::debug) {
			std::string gold_str;
			if(game_config::debug || !enemy || !viewing_team_.uses_fog()) {
				gold_str = utils::half_signed_value(data.gold);
			}

			column_stats["label"] = data.gold < 0 ? "<span color='#ff0000'>" + gold_str + "</span>" : gold_str;
			row_data_stats.emplace("team_gold", column_stats);

			std::string village_count = std::to_string(data.villages);
			if(!viewing_team_.uses_fog() && !viewing_team_.uses_shroud()) {
				village_count += "/" + std::to_string(board_.map().villages().size());
			}

			column_stats["label"] = village_count;
			row_data_stats.emplace("team_villages", column_stats);

			column_stats["label"] = std::to_string(data.units);
			row_data_stats.emplace("team_units", column_stats);

			column_stats["label"] = std::to_string(data.upkeep);
			row_data_stats.emplace("team_upkeep", column_stats);

			const std::string income = utils::signed_value(data.net_income);
			column_stats["label"] = data.net_income < 0 ? "<span color='#ff0000'>" + income + "</span>" : income;
			row_data_stats.emplace("team_income", column_stats);
		}

		stats_list.add_row(row_data_stats);

		//
		// Settings list
		//
		std::map<std::string, string_map> row_data_settings;
		string_map column_settings;

		column_settings["use_markup"] = "true";

		column_settings["label"] = leader_image;
		row_data_settings.emplace("team_leader_image", column_settings);

		column_settings["label"] = leader_name + "\n" + controller_name(team);
		row_data_settings.emplace("team_leader_name", column_settings);

		column_settings["label"] = std::to_string(team.side());
		row_data_settings.emplace("team_side", column_settings);

		column_settings["label"] = std::to_string(team.start_gold());
		row_data_settings.emplace("team_start_gold", column_settings);

		column_settings["label"] = std::to_string(team.base_income());
		row_data_settings.emplace("team_base_income", column_settings);

		column_settings["label"] = std::to_string(team.village_gold());
		row_data_settings.emplace("team_village_gold", column_settings);

		column_settings["label"] = std::to_string(team.village_support());
		row_data_settings.emplace("team_village_support", column_settings);

		column_settings["label"] = team.uses_fog() ? _("yes") : _("no");
		row_data_settings.emplace("team_fog", column_settings);

		column_settings["label"] = team.uses_shroud() ? _("yes") : _("no");
		row_data_settings.emplace("team_shroud", column_settings);

		settings_list.add_row(row_data_settings);
	}

	// Sorting options for the status list
	stats_list.register_sorting_option(0, [this](const int i) {
		unit_const_ptr leader = get_leader(i + 1);
		return leader ? leader->name().str() : ""; });

	stats_list.register_sorting_option(1, [this](const int i) { return board_.teams()[i].user_team_name().str(); });
	stats_list.register_sorting_option(2, [this](const int i) { return board_.teams()[i].gold(); });
	stats_list.register_sorting_option(3, [this](const int i) { return board_.teams()[i].villages(); });
	stats_list.register_sorting_option(4, [this](const int i) { return team_data_[i].units; });
	stats_list.register_sorting_option(5, [this](const int i) { return team_data_[i].upkeep; });
	stats_list.register_sorting_option(6, [this](const int i) { return team_data_[i].net_income; });

	// Sorting options for the settings list
	settings_list.register_sorting_option(0, [this](const int i) {
		unit_const_ptr leader = get_leader(i + 1);
		return leader ? leader->name().str() : ""; });

	settings_list.register_sorting_option(1, [this](const int i) { return board_.teams()[i].side(); });
	settings_list.register_sorting_option(2, [this](const int i) { return board_.teams()[i].start_gold(); });
	settings_list.register_sorting_option(3, [this](const int i) { return board_.teams()[i].base_income(); });
	settings_list.register_sorting_option(4, [this](const int i) { return board_.teams()[i].village_gold(); });
	settings_list.register_sorting_option(5, [this](const int i) { return board_.teams()[i].village_support(); });
	settings_list.register_sorting_option(6, [this](const int i) { return board_.teams()[i].uses_fog(); });
	settings_list.register_sorting_option(7, [this](const int i) { return board_.teams()[i].uses_shroud(); });

	//
	// Set up tab control
	//
	listbox& tab_bar = find_widget<listbox>(&window, "tab_bar", false);

	window.keyboard_capture(&tab_bar);

	connect_signal_notify_modified(tab_bar, std::bind(&game_stats::on_tab_select, this, std::ref(window)));

	on_tab_select(window);
}

void game_stats::on_tab_select(window& window)
{
	const int i = find_widget<listbox>(&window, "tab_bar", false).get_selected_row();

	find_widget<stacked_widget>(&window, "pager", false).select_layer(i);

	// There are only two tabs, so this is simple
	find_widget<label>(&window, "title", false).set_label(
		i == 0 ? _("Current Status") : _("Scenario Settings")
	);
}

void game_stats::post_show(window& window)
{
	if(get_retval() == window::OK) {
		const int selected_tab = find_widget<listbox>(&window, "tab_bar", false).get_selected_row();

		const std::string list_id = selected_tab == 0 ? "game_stats_list" : "scenario_settings_list";
		selected_index_ = find_widget<listbox>(&window, list_id, false).get_selected_row();
	}
}

} // namespace dialogs
} // namespace gui2
