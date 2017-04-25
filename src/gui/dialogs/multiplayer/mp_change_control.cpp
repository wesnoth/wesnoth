/*
   Copyright (C) 2011 - 2017 by Lukasz Dobrogowski
   <lukasz.dobrogowski@gmail.com>
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

#include "gui/dialogs/multiplayer/mp_change_control.hpp"

#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "menu_events.hpp"
#include "resources.hpp"
#include "team.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_gui("gui/dialogs/mp_change_control");
#define ERR_GUI LOG_STREAM(err,   log_gui)
#define WRN_GUI LOG_STREAM(warn,  log_gui)
#define LOG_GUI LOG_STREAM(info,  log_gui)
#define DBG_GUI LOG_STREAM(debug, log_gui)

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_change_control
 *
 * == Change control dialog ==
 *
 * This shows the multiplayer change control dialog.
 *
 * @begin{table}{dialog_widgets}
 * sides_list & & listbox & m &
 *         List of sides participating in the MP game. $
 *
 * nicks_list & & listbox & m &
 *         List of nicks of all clients playing or observing the MP game. $
 *
 * @end{table}
 *
 */

REGISTER_DIALOG(mp_change_control)

mp_change_control::mp_change_control(events::menu_handler* mh)
	: menu_handler_(mh)
	, selected_side_(0)
	, selected_nick_(0)
	, sides_()
	, nicks_()
{
}

void mp_change_control::pre_show(window& window)
{
	listbox& sides_list = find_widget<listbox>(&window, "sides_list", false);
	listbox& nicks_list = find_widget<listbox>(&window, "nicks_list", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(sides_list,
		std::bind(&mp_change_control::handle_sides_list_item_clicked, this, std::ref(window)));

	connect_signal_notify_modified(nicks_list,
		std::bind(&mp_change_control::handle_nicks_list_item_clicked, this, std::ref(window)));
#else
	sides_list.set_callback_value_change(
		dialog_callback<mp_change_control, &mp_change_control::handle_sides_list_item_clicked>);

	nicks_list.set_callback_value_change(
		dialog_callback<mp_change_control, &mp_change_control::handle_nicks_list_item_clicked>);
#endif

	//
	// Initialize sides list
	//
	const unsigned int num_sides = resources::gameboard
		? static_cast<unsigned int>(resources::gameboard->teams().size())
		: 0;

	for(unsigned int side = 1; side <= num_sides; ++side) {
		if(resources::gameboard->teams().at(side - 1).hidden()) {
			continue;
		}

		sides_.push_back(side);

		std::map<std::string, string_map> data;
		string_map item;

		std::string side_str = vgettext("Side $side", {{"side", std::to_string(side)}});
		side_str = font::span_color(team::get_side_color(side)) + side_str + "</span>";

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
	for(const auto& team : resources::gameboard->teams()) {
		if(!team.is_local_ai() && !team.is_network_ai() && !team.is_idle()
			&& !team.is_empty() && !team.current_player().empty())
		{
			temp_nicks.insert(team.current_player());
		}
	}

	const std::set<std::string>& observers = resources::screen->observers();
	temp_nicks.insert(observers.begin(), observers.end());

	// In case we are an observer, it isn't in the observers set and has to be added manually.
	temp_nicks.insert(preferences::login());

	//
	// Initialize nick list
	//
	for(const std::string& nick : temp_nicks) {
		nicks_.push_back(nick);

		std::map<std::string, string_map> data;
		string_map item;

		item["id"] = nick;
		item["label"] = nick;
		item["use_markup"] = "true";
		data.emplace("nick", item);

		nicks_list.add_row(data);
	}

	handle_sides_list_item_clicked(window);
	handle_nicks_list_item_clicked(window);
}

void mp_change_control::handle_sides_list_item_clicked(window& window)
{
	selected_side_ = find_widget<listbox>(&window, "sides_list", false).get_selected_row();

	highlight_side_nick(window);
}

void mp_change_control::handle_nicks_list_item_clicked(window& window)
{
	selected_nick_ = find_widget<listbox>(&window, "nicks_list", false).get_selected_row();
}

void mp_change_control::highlight_side_nick(window& window)
{
	listbox& nicks_list = find_widget<listbox>(&window, "nicks_list", false);
	const auto& teams = resources::gameboard->teams();

	int i = 0;
	for(const std::string& nick : nicks_) {
		std::string label_str = "";

		if(selected_side_ <= static_cast<unsigned int>(teams.size()) && teams.at(selected_side_).current_player() == nick) {
			label_str = formatter() << "<b>" << nick << "</b>";
		} else {
			label_str = nick;
		}

		grid* row_grid = nicks_list.get_row_grid(i);
		find_widget<label>(row_grid, nick, false).set_label(label_str);

		++i;
	}
}

void mp_change_control::post_show(window& window)
{
	if(window.get_retval() == window::OK) {
		DBG_GUI << "Main: changing control of side "
		        << sides_[selected_side_] << " to nick "
		        << nicks_[selected_nick_] << std::endl;

		if(menu_handler_) { // since in unit tests we pass a null pointer to it
			menu_handler_->request_control_change(
				sides_[selected_side_],
				nicks_[selected_nick_]
			);
		}
	}
}

} // namespace dialogs
} // namespace gui2
