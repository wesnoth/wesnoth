/*
	Copyright (C) 2021 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/dialogs/multiplayer/match_history.hpp"

#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "filesystem.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "network_download_file.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/markup.hpp"
#include "wesnothd_connection.hpp"

using namespace std::chrono_literals;

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

namespace gui2::dialogs
{
REGISTER_DIALOG(mp_match_history)

mp_match_history::mp_match_history(const std::string& player_name, wesnothd_connection& connection, bool wait_for_response)
	: modal_dialog(window_id())
	, player_name_(player_name)
	, connection_(connection)
	, offset_(0)
	, wait_for_response_(wait_for_response)
{
	register_label("title", true, VGETTEXT("Match History — $player", {{"player", player_name_}}));
}

void mp_match_history::pre_show()
{
	set_enter_disabled(true);

	button& newer_history = find_widget<button>("newer_history");
	button& older_history = find_widget<button>("older_history");
	connect_signal_mouse_left_click(newer_history, std::bind(&mp_match_history::newer_history_offset, this));
	connect_signal_mouse_left_click(older_history, std::bind(&mp_match_history::older_history_offset, this));

	button& search = find_widget<button>("search");
	connect_signal_mouse_left_click(search, std::bind(&mp_match_history::new_search, this));

	text_box& search_player = find_widget<text_box>("search_player");
	search_player.set_value(player_name_);

	std::vector<config> content_types;
	content_types.emplace_back("label", _("Scenario"));
	content_types.emplace_back("label", _("Era"));
	content_types.emplace_back("label", _("Modification"));

	find_widget<menu_button>("search_content_type").set_values(content_types);

	update_display();
}

void mp_match_history::new_search()
{
	int old_offset = offset_;
	std::string old_player_name = player_name_;
	text_box& search_player = find_widget<text_box>("search_player");
	player_name_ = search_player.get_value();

	// display update failed, set the offset back to what it was before
	if(!update_display()) {
		offset_ = old_offset;
		player_name_ = old_player_name;
	} else {
		label& title = find_widget<label>("title");
		title.set_label(VGETTEXT("Match History — $player", {{"player", player_name_}}));
	}
}

void mp_match_history::newer_history_offset()
{
	offset_ -= 10;
	// display update failed, set the offset back to what it was before
	if(!update_display()) {
		offset_ += 10;
	}
}

void mp_match_history::older_history_offset()
{
	offset_ += 10;
	// display update failed, set the offset back to what it was before
	if(!update_display()) {
		offset_ -= 10;
	}
}

namespace
{
std::string key_with_fallback(const config::attribute_value& val)
{
	// Note that using the fallback arg of str() doesn't work since the value is set,
	// it's just set as an empty string, and the fallback is only returned if the value
	// is actually unset.
	if(std::string s = val.str(); !s.empty()) {
		return s;
	} else {
		return font::unicode_em_dash;
	}
}
};

bool mp_match_history::update_display()
{
	const config history = request_history();

	// request failed, nothing to do
	if(history.child_count("game_history_results") == 0) {
		return false;
	}

	listbox* history_box = find_widget<listbox>("history", false, true);
	history_box->clear();

	listbox* tab_bar = find_widget<listbox>("tab_bar", false, true);
	connect_signal_notify_modified(*tab_bar, std::bind(&mp_match_history::tab_switch_callback, this));
	tab_bar->select_row(0);

	int i = 0;
	for(const config& game : history.mandatory_child("game_history_results").child_range("game_history_result")) {
		widget_data row;
		grid& history_grid = history_box->add_row(row);

		dynamic_cast<label*>(history_grid.find("game_name", false))->set_label(key_with_fallback(game["game_name"]));
		dynamic_cast<label*>(history_grid.find("scenario_name", false))->set_label(key_with_fallback(game["scenario_name"]));
		dynamic_cast<label*>(history_grid.find("era_name", false))->set_label(markup::span_color("#baac7d", _("Era: ")) + key_with_fallback(game["era_name"]));
		dynamic_cast<label*>(history_grid.find("game_start", false))->set_label(key_with_fallback(game["game_start"]) + _(" UTC+0"));
		dynamic_cast<label*>(history_grid.find("version", false))->set_label(key_with_fallback(game["version"]));

		button* replay_download = dynamic_cast<button*>(history_grid.find("replay_download", false));
		std::string replay_url = game["replay_url"].str();
		if(!replay_url.empty()) {
			std::string filename = utils::split(replay_url, '/').back();
			std::string local_save = filesystem::get_saves_dir()+"/"+filename;

			connect_signal_mouse_left_click(*replay_download, std::bind(&network::download, replay_url, local_save));
		} else {
			replay_download->set_active(false);
		}

		std::vector<std::string> player_list;
		std::vector<std::string> player_faction_list;
		for(const config& player : game.child_range("player")) {
			player_list.emplace_back(font::unicode_bullet + " " + player["name"].str() + ":");
			player_faction_list.emplace_back(player["faction"].str());
		}

		label* players = dynamic_cast<label*>(history_grid.find("players", false));
		players->set_label(utils::join(player_list, "\n"));

		label* factions = dynamic_cast<label*>(history_grid.find("player_factions", false));
		factions->set_label(utils::join(player_faction_list, "\n"));

		history_grid.find("player_grid", false)->set_visible(gui2::widget::visibility::invisible);

		label* modifications = dynamic_cast<label*>(history_grid.find("modifications", false));
		const auto& children = game.child_range("modification");
		if(!children.empty()) {
			std::vector<std::string> modifications_list;

			for(const config& modification : game.child_range("modification")) {
				modifications_list.emplace_back(font::unicode_bullet + " " + modification["name"].str());
			}

			modifications->set_label(utils::join(modifications_list, "\n"));
		}
		modifications->set_visible(gui2::widget::visibility::invisible);

		i++;
		if(i == 10) {
			break;
		}
	}

	// this is already the most recent history, can't get anything newer
	if(offset_ == 0) {
		button* newer_history = find_widget<button>("newer_history", false, true);
		newer_history->set_active(false);
	} else {
		button* newer_history = find_widget<button>("newer_history", false, true);
		newer_history->set_active(true);
	}

	// the server returns up to 11 and the client displays at most 10
	// if fewer than 11 rows are returned, then there are no older rows left to get next
	if(history.child_count("game_history_result") < 11) {
		button* older_history = find_widget<button>("older_history", false, true);
		older_history->set_active(false);
	} else {
		button* older_history = find_widget<button>("older_history", false, true);
		older_history->set_active(true);
	}

	return true;
}

const config mp_match_history::request_history()
{
	config request;
	config& child = request.add_child("game_history_request");
	child["offset"] = offset_;
	child["search_player"] = player_name_;
	child["search_game_name"] = find_widget<text_box>("search_game_name").get_value();
	child["search_content_type"] = find_widget<menu_button>("search_content_type").get_value();
	child["search_content"] = find_widget<text_box>("search_content").get_value();
	DBG_NW << request.debug();
	connection_.send_data(request);

	int times_waited = 0;
	while(true) {
		config response;

		// I'm not really sure why this works to be honest
		// I would've expected that there would be a risk of regular lobby responses showing up here since it's a reference to the lobby's network connection
		// however testing has resulted in showing that this is not the case
		// lobby responses are received in the lobby's network_handler() method when this method is not running
		// lobby responses are not received while this method is running, and are handled in the lobby after it completes
		// history results are never received in the lobby
		if(connection_.receive_data(response)) {
			if(response.child_count("game_history_results") == 0) {
				DBG_NW << "Received non-history data: " << response.debug();
				if(!response["error"].str().empty()) {
					ERR_NW << "Received error from server: " << response["error"].str();
					gui2::show_error_message(_("The server responded with an error:")+" "+response["error"].str());
					return {};
				}
			} else if(response.mandatory_child("game_history_results").child_count("game_history_result") == 0) {
				DBG_NW << "Player has no game history data.";
				gui2::show_error_message(_("No game history found."));
				return {};
			} else {
				DBG_NW << "Received history data: " << response.debug();
				return response;
			}
		} else {
			DBG_NW << "Received no data";
		}

		if(times_waited > 20 || !wait_for_response_) {
			ERR_NW << "Timed out waiting for history data, returning nothing";
			if(wait_for_response_) {
				gui2::show_error_message(_("Request timed out."));
			}
			return {};
		}

		times_waited++;
		std::this_thread::sleep_for(250ms);
	}

	DBG_NW << "Something else happened while waiting for history data, returning nothing";
	gui2::show_error_message(_("Request encountered an unexpected error, please check the logs."));
	return {};
}

void mp_match_history::tab_switch_callback()
{
	listbox* history_box = find_widget<listbox>("history", false, true);
	listbox* tab_bar = find_widget<listbox>("tab_bar", false, true);
	int tab = tab_bar->get_selected_row();

	for(unsigned i = 0; i < history_box->get_item_count(); i++) {
		grid* history_grid = history_box->get_row_grid(i);
		if(tab == 0) {
			history_grid->find("player_grid", false)->set_visible(gui2::widget::visibility::invisible);
			history_grid->find("modifications", false)->set_visible(gui2::widget::visibility::invisible);
		} else if(tab == 1) {
			history_grid->find("player_grid", false)->set_visible(gui2::widget::visibility::visible);
			history_grid->find("modifications", false)->set_visible(gui2::widget::visibility::invisible);
		} else if(tab == 2) {
			history_grid->find("player_grid", false)->set_visible(gui2::widget::visibility::invisible);
			history_grid->find("modifications", false)->set_visible(gui2::widget::visibility::visible);
		}
	}
}

} // namespace dialogs
