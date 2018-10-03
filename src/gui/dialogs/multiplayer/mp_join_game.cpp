/*
   Copyright (C) 2008 - 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "chat_log.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "game_config_manager.hpp"
#include "game_initialization/mp_game_utils.hpp"
#include "preferences/credentials.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/multiplayer/faction_select.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/chatbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "log.hpp"
#include "mp_ui_alerts.hpp"
#include "statistics.hpp"
#include "units/types.hpp"
#include "utils/scope_exit.hpp"
#include "wesnothd_connection.hpp"

static lg::log_domain log_mp_connect_engine("mp/connect/engine");
#define DBG_MP LOG_STREAM(debug, log_mp_connect_engine)
#define LOG_MP LOG_STREAM(info, log_mp_connect_engine)
#define WRN_MP LOG_STREAM(warn, log_mp_connect_engine)
#define ERR_MP LOG_STREAM(err, log_mp_connect_engine)

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(mp_join_game)

mp_join_game::mp_join_game(saved_game& state, mp::lobby_info& lobby_info, wesnothd_connection& connection, const bool first_scenario, const bool observe_game)
	: level_()
	, state_(state)
	, lobby_info_(lobby_info)
	, network_connection_(connection)
	, update_timer_(0)
	, first_scenario_(first_scenario)
	, observe_game_(observe_game)
	, stop_updates_(false)
	, player_list_(nullptr)
	, flg_dialog_(nullptr)
{
	set_show_even_without_video(true);
}

mp_join_game::~mp_join_game()
{
	if(update_timer_ != 0) {
		remove_timer(update_timer_);
		update_timer_ = 0;
	}
}

/*
 * Fetch the selected game's config from the server and prompts an initial faction selection.
 */
bool mp_join_game::fetch_game_config()
{
	// Ask for the next scenario data, if applicable
	if(!first_scenario_) {
		network_connection_.send_data(config("load_next_scenario"));
	}

	bool has_scenario_and_controllers = false;
	while(!has_scenario_and_controllers) {
		config revc;
		const bool data_res =
			network_connection_.fetch_data_with_loading_screen(revc, loading_stage::download_level_data);

		if(!data_res) {
			return false;
		}

		mp::check_response(data_res, revc);

		if(revc.child("leave_game")) {
			return false;
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
		mp::level_to_gamestate(level_, state_);
		return true;
	}

	if(first_scenario_) {
		state_.clear();
		state_.classification() = game_classification(level_);

		// Make sure that we have the same config as host, if possible.
		game_config_manager::get()->load_game_config_for_game(state_.classification());
	}

	game_config::add_color_info(get_scenario());

	// If we're just an observer, we don't need to find an appropriate side and set faction selection
	if(observe_game_) {
		return true;
	}

	// Search for an appropriate vacant slot. If a description is set (i.e. we're loading from a saved game),
	// then prefer to get the side with the same description as our login. Otherwise just choose the first
	// available side.
	const config* side_choice = nullptr;

	int side_num_choice = 1, side_num_counter = 1;
	for(const config& side : get_scenario().child_range("side")) {
		// TODO: it can happen that the scenario specifies that the controller
		//       of a side should also gain control of another side.
		if(side["controller"] == "reserved" && side["current_player"] == preferences::login()) {
			side_choice = &side;
			side_num_choice = side_num_counter;
			break;
		}

		if(side["controller"] == "human" && side["player_id"].empty()) {
			if(!side_choice) { // Found the first empty side
				side_choice = &side;
				side_num_choice = side_num_counter;
			}

			if(side["current_player"] == preferences::login()) {
				side_choice = &side;
				side_num_choice = side_num_counter;
				break;  // Found the preferred one
			}
		}

		if(side["player_id"] == preferences::login()) {
			// We already own a side in this game
			return true;
		}

		++side_num_counter;
	}

	if(!side_choice) {
		observe_game_ = true;
		return true;
	}

	// If the client is allowed to choose their team, do that here instead of having it set by the server
	if((*side_choice)["allow_changes"].to_bool(true)) {
		if(!show_flg_select(side_num_choice, true)) {
			return false;
		}
	}

	return true;
}

static std::string generate_user_description(const config& side)
{
	// Allow the host to override, since only the host knows the ai_algorithm.
	if(const config::attribute_value* desc = side.get("user_description")) {
		return desc->str();
	}

	const std::string controller_type = side["controller"].str();
	const std::string reservation = side["current_player"].str();
	const std::string owner = side["player_id"].str();

	if(controller_type == "ai") {
		return _("Computer Player");
	} else if(controller_type == "null") {
		return _("Empty slot");
	} else if(controller_type == "reserved") {
		return VGETTEXT("Reserved for $playername", {{"playername", reservation}});
	} else if(owner.empty()) {
		return _("Vacant slot");
	} else if(controller_type == "human" || controller_type == "network") {
		return owner;
	} else {
		return _("empty");
	}
}

void mp_join_game::pre_show(window& window)
{
	window.set_enter_disabled(true);

	//
	// Set title
	//
	label& title = find_widget<label>(&window, "title", false);
	title.set_label((formatter() << title.get_label() << " " << font::unicode_em_dash << " " << get_scenario()["name"].t_str()).str());

	//
	// Set up sides list
	//
	generate_side_list(window);

	//
	// Initialize chatbox and game rooms
	//
	chatbox& chat = find_widget<chatbox>(&window, "chat", false);

	chat.set_lobby_info(lobby_info_);
	chat.set_wesnothd_connection(network_connection_);

	chat.room_window_open(N_("this game"), true, false);
	chat.active_window_changed();
	chat.load_log(default_chat_log, false);

	//
	// Set up player list
	//
	player_list_.reset(new player_list_helper(&window));

	//
	// Set up the network handling
	//
	update_timer_ = add_timer(game_config::lobby_network_timer, std::bind(&mp_join_game::network_handler, this, std::ref(window)), true);

	//
	// Set up the Lua plugin context
	//
	plugins_context_.reset(new plugins_context("Multiplayer Join"));

	plugins_context_->set_callback("launch", [&window](const config&) { window.set_retval(retval::OK); }, false);
	plugins_context_->set_callback("quit",   [&window](const config&) { window.set_retval(retval::CANCEL); }, false);
	plugins_context_->set_callback("chat",   [&chat](const config& cfg) { chat.send_chat_message(cfg["message"], false); }, true);
}

bool mp_join_game::show_flg_select(int side_num, bool first_time)
{
	if(const config& side_choice = get_scenario().child("side", side_num - 1)) {
		if(!side_choice["allow_changes"].to_bool(true)) {
			return true;
		}

		const config& era = level_.child("era");
		if(!era) {
			ERR_MP << "no era information\n";
			return false;
		}

		config::const_child_itors possible_sides = era.child_range("multiplayer_side");
		if(possible_sides.empty()) {
			WRN_MP << "no [multiplayer_side] found in era '" << era["id"] << "'.\n";
			return false;
		}

		const std::string color = side_choice["color"].str();

		std::vector<const config*> era_factions;
		//make this safe against changes to level_ that might make possible_sides invalid pointers.
		config era_copy;
		for(const config& side : possible_sides) {
			config& side_new = era_copy.add_child("multiplayer_side", side);
			era_factions.push_back(&side_new);
		}

		const bool is_mp = state_.classification().is_normal_mp_game();
		const bool lock_settings = get_scenario()["force_lock_settings"].to_bool(!is_mp);
		const bool use_map_settings = level_.child("multiplayer")["mp_use_map_settings"].to_bool();
		const bool saved_game = level_.child("multiplayer")["savegame"].to_bool();

		ng::flg_manager flg(era_factions, side_choice, lock_settings, use_map_settings, saved_game);

		{
			gui2::dialogs::faction_select flg_dialog(flg, color, side_num);
			flg_dialog_ = &flg_dialog;
			utils::scope_exit se([this]() { flg_dialog_ = nullptr; });

			if(!flg_dialog.show() && !first_time) {
				return true;
			}
		}

		config faction;
		config& change = faction.add_child("change_faction");
		change["change_faction"] = true;
		change["name"] = preferences::login();
		change["faction"] = flg.current_faction()["id"];
		change["leader"] = flg.current_leader();
		change["gender"] = flg.current_gender();
		// TODO: the host cannot yet handle this and always uses the first side owned by that player.
		change["side_num"] = side_num;

		network_connection_.send_data(faction);
	}

	return true;
}

void mp_join_game::generate_side_list(window& window)
{
	if(stop_updates_) {
		return;
	}

	tree_view& tree = find_widget<tree_view>(&window, "side_list", false);

	window.keyboard_capture(&tree);

	tree.clear();
	team_tree_map_.clear();
	const widget_data empty_map;

	int side_num = 0;
	for(const auto& side : get_scenario().child_range("side")) {
		++side_num;
		if(!side["allow_player"].to_bool(true)) {
			continue;
		}

		// Check to see whether we've added a toplevel tree node for this team. If not, add one
		if(team_tree_map_.find(side["team_name"].str()) == team_tree_map_.end()) {
			widget_data data;
			widget_item item;

			item["label"] = (formatter() << _("Team:") << " " << t_string::from_serialized(side["user_team_name"])).str();
			data.emplace("tree_view_node_label", item);

			tree_view_node& team_node = tree.add_node("team_header", data);
			team_node.add_sibling("side_spacer", empty_map);

			team_tree_map_[side["team_name"].str()] = &team_node;
		}

		widget_data data;
		widget_item item;

		const std::string color = !side["color"].empty() ? side["color"] : side["side"].str();

		item["label"] = (formatter() << "<span color='" << font::get_pango_color_from_id(color) << "'>" << side["side"] << "</span>").str();
		data.emplace("side_number", item);

		std::string leader_image = ng::random_enemy_picture;
		std::string leader_type = side["type"];
		std::string leader_gender = side["gender"];
		std::string leader_name;

		// If there is a unit which can recruit, use it as a leader.
		// Necessary to display leader information when loading saves.
		for(const config& side_unit : side.child_range("unit")) {
			if(side_unit["canrecruit"].to_bool()) {
				leader_type = side_unit["type"].str();
				leader_gender = side_unit["gender"].str();
				break;
			}
		}

		if(const unit_type* ut = unit_types.find(leader_type)) {
			const unit_type& type = ut->get_gender_unit_type(leader_gender);

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
		if(leader_gender != "null") {
			gender_icon = formatter() << "icons/icon-" << leader_gender << ".png";
			item["tooltip"] = leader_gender;
		}

		item["label"] = gender_icon;
		data.emplace("leader_gender", item);

		item.clear();

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

		tree_view_node& node = team_tree_map_[side["team_name"].str()]->add_child("side_panel", data);

		grid& row_grid = node.get_grid();

		auto* select_leader_button = find_widget<button>(&row_grid, "select_leader", false, false);
		if(select_leader_button) {
			if(side["player_id"] == preferences::login() && side["allow_changes"].to_bool(true)) {
				//
				// Small wrapper function in order to set the handled and halt parameters and prevent
				// crashes in case the dialog closes and the original button to which the callback was
				// bound had already been destroyed. The other use of show_flg_select doesn't need these
				// parameters, so it's easier not to declare them as function arguments.
				//
				const auto handler = [this, side_num](bool& handled, bool& halt) {
					show_flg_select(side_num);
					// note: this function is called from a std::function object stored in the widget 
					//       and show_flg_select which internally calls
					//       show_dialog -> pump -> ... -> network_handler -> ... -> generate_side_list
					//       might destroy that std::function object while it is executed, this means that
					//       using the captured variables 'this' and 'side_num' after this will result in
					//       unexpected behaviour or crashes.
					handled = halt = true;
				};

				connect_signal_mouse_left_click(*select_leader_button, std::bind(handler, _3, _4));
			} else {
				select_leader_button->set_visible(widget::visibility::hidden);
			}
		}

		if(income_amt == 0) {
			find_widget<image>(&row_grid, "income_icon", false).set_visible(widget::visibility::invisible);
			find_widget<label>(&row_grid, "side_income", false).set_visible(widget::visibility::invisible);
		}
	}
}

void mp_join_game::close_faction_select_dialog_if_open()
{
	if(flg_dialog_) {
		if(window* w = flg_dialog_->get_window()) {
			w->set_retval(retval::CANCEL);
		}
	}
}

void mp_join_game::network_handler(window& window)
{
	// If the game has already started, close the dialog immediately.
	if(level_["started"].to_bool()) {
		window.set_retval(retval::OK);
		return;
	}

	config data;
	if(!network_connection_.receive_data(data)) {
		return;
	}

	// Update chat
	find_widget<chatbox>(&window, "chat", false).process_network_data(data);

	if(!data["message"].empty()) {
		gui2::show_transient_message(_("Response") , data["message"]);
	}

	if(data["failed"].to_bool()) {
		close_faction_select_dialog_if_open();

		window.set_retval(retval::CANCEL);
	} else if(data.child("start_game")) {
		close_faction_select_dialog_if_open();

		level_["started"] = true;
		window.set_retval(retval::OK);
	} else if(data.child("leave_game")) {
		close_faction_select_dialog_if_open();

		window.set_retval(retval::CANCEL);
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

		if(flg_dialog_ && flg_dialog_->get_side_num() == change["side"].to_int()) {
			close_faction_select_dialog_if_open();
		}
	} else if(data.has_child("scenario") || data.has_child("snapshot") || data.child("next_scenario")) {
		level_ = first_scenario_ ? data : data.child("next_scenario");

		generate_side_list(window);
	}

	if(data.has_child("turn")) {
		ERR_MP << "received replay data\n" << data << "\n in mp join\n";
	}

	// Update player list
	if(data.has_child("user")) {
		player_list_->update_list(data.child_range("user"));
	}
}

config& mp_join_game::get_scenario()
{
	if(config& scenario = level_.child("scenario")) {
		return scenario;
	} else if(config& snapshot = level_.child("snapshot")) {
		return snapshot;
	}

	return level_;
}

void mp_join_game::post_show(window& window)
{
	if(update_timer_ != 0) {
		remove_timer(update_timer_);
		update_timer_ = 0;
	}

	if(window.get_retval() == retval::OK) {
		if(const config& stats = level_.child("statistics")) {
			statistics::fresh_stats();
			statistics::read_stats(stats);
		}

		mp::level_to_gamestate(level_, state_);

		mp_ui_alerts::game_has_begun();
	} else if(observe_game_) {
		network_connection_.send_data(config("observer_quit", config { "name", preferences::login() }));
	} else {
		network_connection_.send_data(config("leave_game"));
	}
}

} // namespace dialogs
} // namespace gui2
