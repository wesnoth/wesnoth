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

#include "gui/dialogs/game_stats.hpp"

#include "gui/widgets/listbox.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/window.hpp"
#include "formatter.hpp"
#include "game_classification.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"
#include "serialization/markup.hpp"
#include "resources.hpp"
#include "units/map.hpp"
#include "units/unit.hpp"

#include <functional>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2::dialogs
{

REGISTER_DIALOG(game_stats)

game_stats::game_stats(const display_context& board, const team& viewing_team, int& selected_side_number)
	: modal_dialog(window_id())
	, board_(board)
	, viewing_team_(viewing_team)
	, selected_side_number_(selected_side_number)
{
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
	static const side_controller::sized_array<t_string> names {_("controller^Idle"), _("controller^Human"), _("controller^AI"), _("controller^Reserved")};
	return markup::span_color("#808080", markup::tag("small", names[static_cast<int>(t.controller())]));
}

void game_stats::pre_show()
{
	listbox& stats_list    = find_widget<listbox>("game_stats_list");
	listbox& settings_list = find_widget<listbox>("scenario_settings_list");

	for(const auto& team : board_.teams()) {
		if(team.hidden()) {
			continue;
		}

		team_data_.emplace_back(board_, team);

		widget_data row_data_stats;
		widget_item column_stats;

		const bool known = viewing_team_.knows_about_team(team.side() - 1);
		const bool enemy = viewing_team_.is_enemy(team.side());

		const team_data& data = team_data_.back();

		unit_const_ptr leader = get_leader(team.side());

		std::string leader_name;
		std::string leader_image;

		const bool see_all = game_config::debug || (resources::controller && resources::controller->get_display().show_everything());
		if(leader) {
			const bool visible = leader->is_visible_to_team(leader->get_location(), viewing_team_, see_all);

			// Add leader image. If it's fogged/[hides], show only a random leader image.
			if(visible || known) {
				leader_image = leader->absolute_image() + leader->image_mods();
				leader_name  = leader->name();
			} else {
				leader_image = formatter() << "units/unknown-unit.png" << "~RC(magenta>" << team.color() << ")";
				leader_name  = _("Unknown");
			}

			if(resources::controller) {
				if(resources::controller->get_classification().is_multiplayer()) {
					leader_name = team.side_name();
				}
			}

			leader_name = markup::span_color(team::get_side_highlight_pango(team.side()), leader_name);
		}

		//
		// Status list
		//
		column_stats["use_markup"] = "true";

		column_stats["label"] = leader_image;
		row_data_stats.emplace("team_leader_image", column_stats);

		column_stats["label"] = leader_name + "\n" + controller_name(team);
		column_stats["tooltip"] = team::get_side_color_name_for_UI(team.side());
		row_data_stats.emplace("team_leader_name", column_stats);
		column_stats.erase("tooltip");

		column_stats["label"] = team.user_team_name().empty() ? team.team_name() : team.user_team_name().str();
		row_data_stats.emplace("team_name", column_stats);

		// Only fill in the rest of the info if the side is known...
		if(known || see_all) {
			std::string gold_str;
			if(see_all || !enemy || !viewing_team_.uses_fog()) {
				gold_str = utils::half_signed_value(team.gold());
			}

			column_stats["label"] = team.gold() < 0 ? markup::span_color("#ff0000", gold_str) : gold_str;
			row_data_stats.emplace("team_gold", column_stats);

			std::string village_count = std::to_string(team.villages().size());
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
			column_stats["label"] = data.net_income < 0 ? markup::span_color("#ff0000", income) : income;
			row_data_stats.emplace("team_income", column_stats);
		}

		stats_list.add_row(row_data_stats);

		//
		// Settings list
		//
		widget_data row_data_settings;
		widget_item column_settings;

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
	stats_list.set_sorters(
		[this](const std::size_t i) {
			unit_const_ptr leader = get_leader(i + 1);
			return leader ? leader->name() : t_string();
		},
		[this](const std::size_t i) { return board_.teams()[i].user_team_name(); },
		[this](const std::size_t i) { return board_.teams()[i].gold(); },
		[this](const std::size_t i) { return board_.teams()[i].villages(); },
		[this](const std::size_t i) { return team_data_[i].units; },
		[this](const std::size_t i) { return team_data_[i].upkeep; },
		[this](const std::size_t i) { return team_data_[i].net_income; }
	);

	// Sorting options for the settings list
	settings_list.set_sorters(
		[this](const std::size_t i) {
			unit_const_ptr leader = get_leader(i + 1);
			return leader ? leader->name() : t_string();
		},
		[this](const std::size_t i) { return board_.teams()[i].side(); },
		[this](const std::size_t i) { return board_.teams()[i].start_gold(); },
		[this](const std::size_t i) { return board_.teams()[i].base_income(); },
		[this](const std::size_t i) { return board_.teams()[i].village_gold(); },
		[this](const std::size_t i) { return board_.teams()[i].village_support(); },
		[this](const std::size_t i) { return board_.teams()[i].uses_fog(); },
		[this](const std::size_t i) { return board_.teams()[i].uses_shroud(); }
	);

	//
	// Set up tab control
	//
	listbox& tab_bar = find_widget<listbox>("tab_bar");

	keyboard_capture(&tab_bar);

	connect_signal_notify_modified(tab_bar, std::bind(&game_stats::on_tab_select, this));

	on_tab_select();
}

void game_stats::on_tab_select()
{
	const int i = find_widget<listbox>("tab_bar").get_selected_row();

	find_widget<stacked_widget>("pager").select_layer(i);

	// There are only two tabs, so this is simple
	find_widget<label>("title").set_label(
		i == 0 ? _("Current Status") : _("Scenario Settings")
	);
}

void game_stats::post_show()
{
	if(get_retval() == retval::OK) {
		const int selected_tab = find_widget<listbox>("tab_bar").get_selected_row();

		const std::string list_id = selected_tab == 0 ? "game_stats_list" : "scenario_settings_list";
		selected_side_number_ = team_data_[find_widget<listbox>(list_id).get_selected_row()].side;
	}
}

} // namespace dialogs
