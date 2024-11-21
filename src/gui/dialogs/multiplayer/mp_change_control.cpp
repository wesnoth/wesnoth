/*
	Copyright (C) 2011 - 2024
	by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
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

#include "gui/dialogs/multiplayer/mp_change_control.hpp"

#include "serialization/markup.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_display.hpp"
#include "preferences/preferences.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "menu_events.hpp"
#include "serialization/markup.hpp"
#include "team.hpp"

#include <functional>

static lg::log_domain log_gui("gui/dialogs/mp_change_control");
#define ERR_GUI LOG_STREAM(err,   log_gui)
#define WRN_GUI LOG_STREAM(warn,  log_gui)
#define LOG_GUI LOG_STREAM(info,  log_gui)
#define DBG_GUI LOG_STREAM(debug, log_gui)

namespace gui2::dialogs
{

REGISTER_DIALOG(mp_change_control)

mp_change_control::mp_change_control(events::menu_handler& mh)
	: modal_dialog(window_id())
	, menu_handler_(mh)
	, selected_side_(0)
	, selected_nick_(0)
	, sides_()
	, nicks_()
{
}

void mp_change_control::pre_show()
{
	listbox& sides_list = find_widget<listbox>("sides_list");
	listbox& nicks_list = find_widget<listbox>("nicks_list");

	connect_signal_notify_modified(sides_list,
		std::bind(&mp_change_control::handle_sides_list_item_clicked, this));

	connect_signal_notify_modified(nicks_list,
		std::bind(&mp_change_control::handle_nicks_list_item_clicked, this));

	//
	// Initialize sides list
	//
	for(const team& t : menu_handler_.board().teams()) {
		if(t.hidden()) {
			continue;
		}

		const int side = sides_.emplace_back(t.side());

		widget_data data;
		widget_item item;

		std::string side_str = VGETTEXT("Side $side", {{"side", std::to_string(side)}});
		side_str = markup::span_color(team::get_side_color(side), side_str);

		item["id"] = (formatter() << "side_" << side).str();
		item["label"] = side_str;
		item["use_markup"] = "true";
		data.emplace("side", item);

		sides_list.add_row(data);
	}

	//
	// Prepare set of available nicknames
	//
	std::set<std::string> temp_nicks;
	for(const auto& team : menu_handler_.board().teams()) {
		if(!team.is_local_ai() && !team.is_network_ai() && !team.is_idle()
			&& !team.is_empty() && !team.current_player().empty())
		{
			temp_nicks.insert(team.current_player());
		}
	}

	const std::set<std::string>& observers = game_display::get_singleton()->observers();
	temp_nicks.insert(observers.begin(), observers.end());

	// In case we are an observer, it isn't in the observers set and has to be added manually.
	temp_nicks.insert(prefs::get().login());

	//
	// Initialize nick list
	//
	for(const std::string& nick : temp_nicks) {
		nicks_.push_back(nick);

		widget_data data;
		widget_item item;

		item["id"] = nick;
		item["label"] = nick;
		item["use_markup"] = "true";
		data.emplace("nick", item);

		nicks_list.add_row(data);
	}

	handle_sides_list_item_clicked();
	handle_nicks_list_item_clicked();
}

void mp_change_control::handle_sides_list_item_clicked()
{
	selected_side_ = find_widget<listbox>("sides_list").get_selected_row();

	highlight_side_nick();
}

void mp_change_control::handle_nicks_list_item_clicked()
{
	selected_nick_ = find_widget<listbox>("nicks_list").get_selected_row();
}

void mp_change_control::highlight_side_nick()
{
	listbox& nicks_list = find_widget<listbox>("nicks_list");
	const auto& teams = menu_handler_.board().teams();

	int i = 0;
	for(const std::string& nick : nicks_) {
		std::string label_str = "";

		if(selected_side_ <= static_cast<unsigned int>(teams.size()) && teams.at(selected_side_).current_player() == nick) {
			label_str = markup::bold(nick);
		} else {
			label_str = nick;
		}

		grid* row_grid = nicks_list.get_row_grid(i);
		row_grid->find_widget<label>(nick).set_label(label_str);

		++i;
	}
}

void mp_change_control::post_show()
{
	if(get_retval() == retval::OK) {
		DBG_GUI << "Main: changing control of side "
		        << sides_[selected_side_] << " to nick "
		        << nicks_[selected_nick_];

		menu_handler_.request_control_change(
			sides_[selected_side_],
			nicks_[selected_nick_]
		);
	}
}

} // namespace dialogs
