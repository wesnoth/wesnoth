/*
   Copyright (C) 2008 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/multiplayer/mp_join_game.hpp"

#include "config_assign.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "game_initialization/mp_game_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/faction_select.hpp"
#include "gui/dialogs/network_transmission.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/chatbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/status_label_helper.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/text_box.hpp"
#include "game_config.hpp"
#include "mp_ui_alerts.hpp"
#include "settings.hpp"
#include "statistics.hpp"
#include "units/types.hpp"
#include "formatter.hpp"
#include "wesnothd_connection.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "utils/functional.hpp"
#endif

namespace gui2
{

REGISTER_DIALOG(mp_join_game)

tmp_join_game::tmp_join_game(saved_game& state, lobby_info& lobby_info, twesnothd_connection& wesnothd_connection, const bool first_scenario, const bool observe_game)
	: level_()
	, state_(state)
	, lobby_info_(lobby_info)
	, wesnothd_connection_(wesnothd_connection)
	, update_timer_(0)
	, first_scenario_(first_scenario)
	, observe_game_(observe_game)
	, stop_updates_(false)
{
	set_show_even_without_video(true);
}

tmp_join_game::~tmp_join_game()
{
	if(update_timer_ != 0) {
		remove_timer(update_timer_);
		update_timer_ = 0;
	}
}

/*
 * Fetch the selected game's config from the server and prompts an initial faction selection.
 */
void tmp_join_game::post_build(twindow& window)
{
	// Ask for the next scenario data, if applicable
	if(!first_scenario_) {
		wesnothd_connection_.send_data(config("load_next_scenario"));
	}

	bool has_scenario_and_controllers = false;
	while(!has_scenario_and_controllers) {
		config revc;
		const bool data_res = gui2::tnetwork_transmission::wesnothd_receive_dialog(
			window.video(), "download level data", revc, wesnothd_connection_);

		if(!data_res) {
			window.set_retval(twindow::CANCEL);
		}

		mp::check_response(data_res, revc);

		if(revc.child("leave_game")) {
			window.set_retval(twindow::CANCEL);
		} else if(config& next_scenario = revc.child("next_scenario")) {
			level_.swap(next_scenario);
		} else if(revc.has_attribute("version")) {
			level_.swap(revc);

			has_scenario_and_controllers = true;
		} else if(config& controllers = revc.child("controllers")) {
			int index = 0;
			for(const config& controller : controllers.child_range("controller")) {
				if(config& side = get_scenario().child("side", index)) {
					side["is_local"] = controller["is_local"];
				}
				++index;
			}

			has_scenario_and_controllers = true;
		}

	}

	if(level_["started"].to_bool()) {
		window.set_retval(twindow::OK);
	}

	if(first_scenario_) {
		state_ = saved_game();
		state_.classification() = game_classification(level_);

		// Make sure that we have the same config as host, if possible.
		game_config_manager::get()->load_game_config_for_game(state_.classification());
	}

	game_config::add_color_info(get_scenario());

	// If we're just an observer, we don't need to find an appropriate side and set faction selection
	if(observe_game_) {
		return;
	}

	// Search for an appropriate vacant slot. If a description is set (i.e. we're loading from a saved game),
	// then prefer to get the side with the same description as our login. Otherwise just choose the first
	// available side.
	const config* side_choice = nullptr;

	int side_num = 0, nb_sides = 0;
	for(const config& side : get_scenario().child_range("side")) {
		if(side["controller"] == "reserved" && side["current_player"] == preferences::login()) {
			side_choice = &side;
			side_num = nb_sides;
			break;
		}

		if(side["controller"] == "human" && side["player_id"].empty()) {
			if(!side_choice) { // Found the first empty side
				side_choice = &side;
				side_num = nb_sides;
			}

			if(side["current_player"] == preferences::login()) {
				side_choice = &side;
				side_num = nb_sides;
				break;  // Found the preferred one
			}
		}

		if(side["player_id"] == preferences::login()) {
			// We already own a side in this game
			return;
		}

		++nb_sides;
	}

	if(!side_choice) {
		window.set_retval(twindow::CANCEL);
	}

	// If the client is allowed to choose their team, do that here instead of having it set by the server
	if((*side_choice)["allow_changes"].to_bool(true)) {
		events::event_context context;

		const config& era = level_.child("era");
		// TODO: Check whether we have the era. If we don't, inform the player
		if(!era) {
			throw config::error(_("No era information found."));
		}

		config::const_child_itors possible_sides = era.child_range("multiplayer_side");
		if(possible_sides.empty()) {
			// TODO: is this set_retval needed?
			window.set_retval(twindow::CANCEL);

			throw config::error(_("No multiplayer sides found"));
		}

		const std::string color = (*side_choice)["color"].str();

		std::vector<const config*> era_factions;
		for(const config& side : possible_sides) {
			era_factions.push_back(&side);
		}

		const bool is_mp = state_.classification().is_normal_mp_game();
		const bool lock_settings = get_scenario()["force_lock_settings"].to_bool(!is_mp);
		const bool use_map_settings = level_.child("multiplayer")["mp_use_map_settings"].to_bool();
		const bool saved_game = level_.child("multiplayer")["savegame"].to_bool();

		ng::flg_manager flg(era_factions, *side_choice, lock_settings, use_map_settings, saved_game);

		// FIXME: this dialog doesn't show!
		gui2::tfaction_select dlg(flg, color, side_num);
		dlg.show(window.video());

		if(dlg.get_retval() != gui2::twindow::OK) {
			window.set_retval(twindow::CANCEL);
			return;
		}

		config faction;
		config& change = faction.add_child("change_faction");
		change["change_faction"] = true;
		change["name"] = preferences::login();
		change["faction"] = flg.current_faction()["id"];
		change["leader"] = flg.current_leader();
		change["gender"] = flg.current_gender();

		wesnothd_connection_.send_data(faction);
	}
}

static std::string generate_user_description(const config& side)
{
	// Allow the host to override, since only the host knows the ai_algorithm.
	if(const config::attribute_value* desc = side.get("user_description")) {
		return desc->str();
	}

	const std::string controller_type = side["controller"].str();
	const std::string reservation = side["reserved_for"].str();
	const std::string owner = side["player_id"].str();

	if(controller_type == "ai") {
		return _("Computer Player");
	} else if(controller_type == "null") {
		return _("Empty slot");
	} else if(controller_type == "reserved") {
		return vgettext("Reserved for $playername", {{"playername", reservation}});
	} else if(owner.empty()) {
		return _("Vacant slot");
	} else if(controller_type == "human" || controller_type == "network") {
		return owner;
	} else {
		return _("empty");
	}
}

void tmp_join_game::pre_show(twindow& window)
{
	window.set_enter_disabled(true);

	//
	// Set title
	//
	tlabel& title = find_widget<tlabel>(&window, "title", false);
	title.set_label((formatter() << title.label() << " " << utils::unicode_em_dash << " " << get_scenario()["name"].t_str()).str());

	//
	// Set up sides list
	//
	generate_side_list(window);

	//
	// Initialize chatbox and game rooms
	//
	tchatbox& chat = find_widget<tchatbox>(&window, "chat", false);

	chat.set_lobby_info(lobby_info_);
	chat.set_wesnothd_connection(wesnothd_connection_);

	chat.room_window_open("this game", true); // TODO: better title?
	chat.active_window_changed();

	//
	// Set up player list
	//
	update_player_list(window);

	//
	// Set up the network handling
	//
	update_timer_ = add_timer(game_config::lobby_network_timer, std::bind(&tmp_join_game::network_handler, this, std::ref(window)), true);

	//
	// Set up the Lua plugin context
	//
	plugins_context_.reset(new plugins_context("Multiplayer Join Game"));

	plugins_context_->set_callback("launch", [&window](const config&) { window.set_retval(twindow::OK); }, false);
	plugins_context_->set_callback("quit",   [&window](const config&) { window.set_retval(twindow::CANCEL); }, false);
	plugins_context_->set_callback("chat",   [&chat](const config& cfg) { chat.send_chat_message(cfg["message"], false); }, true);

	// TODO: the old mp wait dialog didn't have an OK button. Evaluate if we want to add one. Hiding it for now
	find_widget<tbutton>(&window, "ok", false).set_visible(twidget::tvisible::hidden);
}

/*
 * We don't need the full widget setup as is done initially, just value setters.
 */
void tmp_join_game::generate_side_list(twindow& window)
{
	if(stop_updates_) {
		return;
	}

	tlistbox& list = find_widget<tlistbox>(&window, "side_list", false);

	window.keyboard_capture(&list);

	list.clear();

	for(const auto& side : get_scenario().child_range("side")) {
		if(!side["allow_player"].to_bool(true)) {
			continue;
		}

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = side["side"];
		data.emplace("side_number", item);

		std::string leader_image = ng::random_enemy_picture;
		std::string leader_type = side["type"];
		std::string leader_name;

		const std::string leader_gender = side["gender"];

		// If there is a unit which can recruit, use it as a leader.
		// Necessary to display leader information when loading saves.
		for(const config& side_unit : side.child_range("unit")) {
			if(side_unit["canrecruit"].to_bool()) {
				leader_type = side_unit["type"].str();
				break;
			}
		}

		if(const unit_type* ut = unit_types.find(leader_type)) {
			const unit_type& type = ut->get_gender_unit_type(leader_gender);

			const std::string color = !side["color"].empty() ? side["color"] : side["side"].str();

			leader_image = formatter() << type.image() << "~RC(" << type.flag_rgb() << ">" << color << ")";
			leader_name = type.type_name();
		}

		item["label"] = leader_image;
		data.emplace("leader_image", item);

		std::string description = generate_user_description(side);
		if(!leader_name.empty()) {
			description += formatter() << " (<i>" << leader_name << "</i>)";
		}

		item["label"] = description;
		data.emplace("leader_type", item);

		item["label"] = (formatter() << "<span color='#a69275'>" << side["faction_name"] << "</span>").str();
		data.emplace("leader_faction", item);

		std::string gender_icon = "icons/icon-random.png";
		if(side["gender"] != "null") {
			gender_icon = formatter() << "icons/icon-" << leader_gender << ".png";
		}

		item["label"] = gender_icon;
		item["tooltip"] = side["gender"];
		data.emplace("leader_gender", item);

		item.clear();

		// TODO: why this tstring stuff?
		item["label"] = t_string::from_serialized(side["user_team_name"].str());
		data.emplace("side_team", item);

		// Don't show gold for saved games
		// TODO: gold icon
		if(side["allow_changes"].to_bool()) {
			item["label"] = side["gold"].str() + " " + _("Gold");
			data.emplace("side_gold", item);
		}

		const int income_amt = side["income"];
		if(income_amt != 0) {
			const std::string income_string = formatter() << (income_amt > 0 ? "+" : "") << income_amt << " " << _("Income");

			item["label"] = income_string;
			data.emplace("side_income", item);
		}

		item["label"] = (formatter() << "buttons/misc/orb-active.png~RC(magenta>" << side["color"] << ")").str();
		data.emplace("side_color", item);

		tgrid& row_grid = list.add_row(data);

		if(income_amt == 0) {
			find_widget<timage>(&row_grid, "income_icon", false).set_visible(twidget::tvisible::invisible);
			find_widget<tlabel>(&row_grid, "side_income", false).set_visible(twidget::tvisible::invisible);
		}
	}
}

void tmp_join_game::update_player_list(twindow& window)
{
	tlistbox& player_list = find_widget<tlistbox>(&window, "player_list", false);

	player_list.clear();

	for(const auto& side : get_scenario().child_range("side")) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = side["player_id"];
		data.emplace("player_name", item);

		player_list.add_row(data);
	}
}

void tmp_join_game::network_handler(twindow& window)
{
	config data;
	if(!wesnothd_connection_.receive_data(data)) {
		return;
	}

	// Update chat
	find_widget<tchatbox>(&window, "chat", false).process_network_data(data);

	if(!data["message"].empty()) {
		gui2::show_transient_message(window.video(), _("Response") , data["message"]);
	}

	if(data["failed"].to_bool()) {
		window.set_retval(twindow::CANCEL);
	} else if(data.child("start_game")) {
		window.set_retval(twindow::OK);
	} else if(data.child("leave_game")) {
		window.set_retval(twindow::CANCEL);
	}

	if(data.child("stop_updates")) {
		stop_updates_ = true;
	} else if(const config& c = data.child("scenario_diff")) {
		// TODO: We should catch config::error and then leave the game.
		level_.apply_diff(c);

		generate_side_list(window);
	} else if(const config& change = data.child("change_controller")) {
		if(config& side_to_change = get_scenario().find_child("side", "side", change["side"])) {
			side_to_change.merge_with(change);
		}
	} else if(data.has_child("scenario") || data.has_child("snapshot") || data.child("next_scenario")) {
		level_ = first_scenario_ ? data : data.child("next_scenario");

		generate_side_list(window);
	}

	// Update player list
	// TODO: optimally, it wouldn't regenerate the entire list every single refresh cycle
	update_player_list(window);
}

config& tmp_join_game::get_scenario()
{
	if(config& scenario = level_.child("scenario")) {
		return scenario;
	} else if(config& snapshot = level_.child("snapshot")) {
		return snapshot;
	}

	return level_;
}

void tmp_join_game::post_show(twindow& window)
{
	if(window.get_retval() == twindow::OK) {
		if(const config& stats = level_.child("statistics")) {
			statistics::fresh_stats();
			statistics::read_stats(stats);
		}

		mp::level_to_gamestate(level_, state_);

		mp_ui_alerts::game_has_begun();
	} else {
		wesnothd_connection_.send_data(config("leave_game"));
	}
}

} // namespace gui2
