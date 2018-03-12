/*
   Copyright (C) 2008 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/multiplayer/mp_staging.hpp"

#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/multiplayer/faction_select.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/chatbox.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/status_label_helper.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "mp_ui_alerts.hpp"
#include "units/types.hpp"
#include "wesnothd_connection.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(mp_staging)

mp_staging::mp_staging(ng::connect_engine& connect_engine, mp::lobby_info& lobby_info, wesnothd_connection* connection)
	: connect_engine_(connect_engine)
	, ai_algorithms_(ai::configuration::get_available_ais())
	, lobby_info_(lobby_info)
	, network_connection_(connection)
	, update_timer_(0)
	, state_changed_(false)
	, team_tree_map_()
	, side_tree_map_()
	, player_list_(nullptr)
{
	set_show_even_without_video(true);

	assert(!ai_algorithms_.empty());
}

mp_staging::~mp_staging()
{
	if(update_timer_ != 0) {
		remove_timer(update_timer_);
		update_timer_ = 0;
	}
}

void mp_staging::pre_show(window& window)
{
	window.set_enter_disabled(true);

	//
	// Set title and status widget states
	//
	label& title = find_widget<label>(&window, "title", false);
	title.set_label((formatter() << title.get_label() << " " << font::unicode_em_dash << " " << connect_engine_.scenario()["name"].t_str()).str());

	update_status_label_and_buttons(window);

	//
	// Set up sides list
	//
	for(const auto& side : connect_engine_.side_engines()) {
		if(side->allow_player() || game_config::debug) {
			add_side_node(window, side);;
		}
	}

	//
	// Initialize chatbox and game rooms
	//
	chatbox& chat = find_widget<chatbox>(&window, "chat", false);

	chat.set_lobby_info(lobby_info_);

	if(network_connection_) {
		chat.set_wesnothd_connection(*network_connection_);
	}

	chat.room_window_open("this game", true, false); // TODO: better title?
	chat.active_window_changed();

	//
	// Set up player list
	//
	player_list_.reset(new player_list_helper(&window));

	//
	// Set up the network handling
	//
	update_timer_ = add_timer(game_config::lobby_network_timer, std::bind(&mp_staging::network_handler, this, std::ref(window)), true);

	//
	// Set up the Lua plugin context
	//
	plugins_context_.reset(new plugins_context("Multiplayer Staging"));

	plugins_context_->set_callback("launch", [&window](const config&) { window.set_retval(retval::OK); }, false);
	plugins_context_->set_callback("quit",   [&window](const config&) { window.set_retval(retval::CANCEL); }, false);
	plugins_context_->set_callback("chat",   [&chat](const config& cfg) { chat.send_chat_message(cfg["message"], false); }, true);
}

void mp_staging::add_side_node(window& window, ng::side_engine_ptr side)
{
	tree_view& tree = find_widget<tree_view>(&window, "side_list", false);
	static const std::map<std::string, string_map> empty_map;

	std::map<std::string, string_map> data;
	string_map item;

	item["label"] = std::to_string(side->index() + 1);
	data.emplace("side_number", item);

	// TODO: don't hardcode magenta?
	item["label"] = "units/unknown-unit.png~RC(magenta>" + side->color_id() + ")";
	data.emplace("leader_image", item);

	item["label"] = "icons/icon-random.png";
	data.emplace("leader_gender", item);

	// Check to see whether we've added a toplevel tree node for this team. If not, add one
	if(team_tree_map_.find(side->team_name()) == team_tree_map_.end()) {
		std::map<std::string, string_map> tree_data;
		string_map tree_item;

		tree_item["label"] = (formatter() << _("Team:") << " " << side->user_team_name()).str();
		tree_data.emplace("tree_view_node_label", tree_item);

		tree_view_node& team_node = tree.add_node("team_header", tree_data);
		team_node.add_sibling("side_spacer", empty_map);

		team_tree_map_[side->team_name()] = &team_node;
	}

	// Find an appropriate position to insert this node. This ensures the side nodes are always
	// arranged by descending index order in each team group.
	int position = 0;
	for(const auto& side_engine : connect_engine_.side_engines()) {
		if(side->team() == side_engine->team() && side->index() > side_engine->index()) {
			++position;
		}
	}

	// Must be *after* the above if block, or the node ptr could be invalid
	tree_view_node& node = team_tree_map_[side->team_name()]->add_child("side_panel", data, position);

	side_tree_map_[side] = &node;

	grid& row_grid = node.get_grid();

	update_leader_display(side, row_grid);

	// Status variables
	const bool fls = connect_engine_.force_lock_settings();
	const bool ums = connect_engine_.params().use_map_settings;

	const bool lock_gold   = side->cfg()["gold_lock"].to_bool(fls);
	const bool lock_income = side->cfg()["income_lock"].to_bool(fls);
	const bool lock_team   = side->cfg()["team_lock"].to_bool(fls);
	const bool lock_color  = side->cfg()["color_lock"].to_bool(fls);

	const bool saved_game = connect_engine_.params().saved_game;

	//
	// AI Algorithm
	//
	int selection = 0;

	// We use an index-based loop in order to get the index of the selected option
	std::vector<config> ai_options;
	for(unsigned i = 0; i < ai_algorithms_.size(); ++i) {
		ai_options.emplace_back(config {"label", ai_algorithms_[i]->text});

		if(ai_algorithms_[i]->id == side->ai_algorithm()) {
			selection = i;
		}
	}

	menu_button& ai_selection = find_widget<menu_button>(&row_grid, "ai_controller", false);

	ai_selection.set_values(ai_options, selection);
	ai_selection.connect_click_handler(std::bind(&mp_staging::on_ai_select, this, side, std::ref(ai_selection)));

	on_ai_select(side, ai_selection);

	//
	// Controller
	//
	std::vector<config> controller_names;
	for(const auto& controller : side->controller_options()) {
		controller_names.emplace_back(config {"label", controller.second});
	}

	menu_button& controller_selection = find_widget<menu_button>(&row_grid, "controller", false);

	controller_selection.set_values(controller_names, side->current_controller_index());
	controller_selection.set_active(controller_names.size() > 1);
	controller_selection.connect_click_handler(std::bind(&mp_staging::on_controller_select, this, side, std::ref(row_grid)));

	on_controller_select(side, row_grid);

	//
	// Leader controls
	//
	button& leader_select = find_widget<button>(&row_grid, "select_leader", false);

	leader_select.set_active(!saved_game);

	connect_signal_mouse_left_click(leader_select,
		std::bind(&mp_staging::select_leader_callback, this, side, std::ref(row_grid)));

	//
	// Team
	//
	std::vector<config> team_names;
	unsigned initial_team_selection = 0;

	for(unsigned i = 0; i < connect_engine_.team_data().size(); ++i) {
		const ng::connect_engine::team_data_pod& tdata = connect_engine_.team_data()[i];

		if(!tdata.is_player_team && !game_config::debug) {
			continue;
		}

		config entry;
		entry["label"] = t_string::from_serialized(tdata.user_team_name);

		// Since we're not necessarily displaying every every team, we need to store the
		// index a displayed team has in the connect_engine's team_data vector. This is
		// then utilized in the click callback.
		entry["team_index"] = i;

		team_names.push_back(std::move(entry));

		// Since, again, every team might not be displayed, and side_engine::team() returns
		// an index into the team_data vector, get an initial selection index for the menu
		// adjusted for the displayed named.
		if(side->team() == i) {
			initial_team_selection = team_names.size() - 1;
		}
	}

	menu_button& team_selection = find_widget<menu_button>(&row_grid, "side_team", false);

	team_selection.set_values(team_names, initial_team_selection);
	team_selection.set_active(!saved_game);
	team_selection.connect_click_handler(std::bind(&mp_staging::on_team_select, this, std::ref(window), side, std::ref(team_selection), _3, _4));

	//
	// Colors
	//
	std::vector<config> color_options;
	for(const auto& color : side->color_options()) {
		color_options.emplace_back(config {
			"label", font::get_color_string_pango(color),
			"icon", (formatter() << "misc/status.png~RC(magenta>" << color << ")").str(),
		});
	}

	menu_button& color_selection = find_widget<menu_button>(&row_grid, "side_color", false);

	color_selection.set_values(color_options, side->color());
	color_selection.set_active(!saved_game);
	color_selection.set_use_markup(true);
	color_selection.connect_click_handler(std::bind(&mp_staging::on_color_select, this, side, std::ref(row_grid)));

	//
	// Gold and Income
	//
	const auto slider_setup_helper = [](slider& slider, const int value) {
		// For the gold and income sliders, the usual min and max values are set in
		// the dialog WML. However, if a side specifies a value out of that range,
		// we adjust the bounds to accommodate it.
		slider.set_value_range(
			std::min(value, slider.get_minimum_value()),
			std::max(value, slider.get_maximum_value())
		);

		slider.set_value(value);
	};

	slider& slider_gold = find_widget<slider>(&row_grid, "side_gold_slider", false);
	slider_setup_helper(slider_gold, side->gold());

	connect_signal_notify_modified(slider_gold, std::bind(
		&mp_staging::on_side_slider_change<&ng::side_engine::set_gold>, this, side, std::ref(slider_gold)));

	slider& slider_income = find_widget<slider>(&row_grid, "side_income_slider", false);
	slider_setup_helper(slider_income, side->income());

	connect_signal_notify_modified(slider_income, std::bind(
		&mp_staging::on_side_slider_change<&ng::side_engine::set_income>, this, side, std::ref(slider_income)));

	// TODO: maybe display the saved values
	if(saved_game) {
		slider_gold.set_visible(widget::visibility::invisible);
		slider_income.set_visible(widget::visibility::invisible);
	}

	//
	// Gold, income, team, and color are only suggestions unless explicitly locked
	//
	if(!saved_game && ums) {
		team_selection.set_active(!lock_team);
		color_selection.set_active(!lock_color);

		slider_gold.set_active(!lock_gold);
		slider_income.set_active(!lock_income);
	}
}

void mp_staging::on_controller_select(ng::side_engine_ptr side, grid& row_grid)
{
	menu_button& ai_selection         = find_widget<menu_button>(&row_grid, "ai_controller", false);
	menu_button& controller_selection = find_widget<menu_button>(&row_grid, "controller", false);

	if(side->controller_changed(controller_selection.get_value())) {
		ai_selection.set_visible(side->controller() == ng::CNTR_COMPUTER ? widget::visibility::visible : widget::visibility::hidden);

		set_state_changed();
	}
}

void mp_staging::on_ai_select(ng::side_engine_ptr side, menu_button& ai_menu)
{
	side->set_ai_algorithm(ai_algorithms_[ai_menu.get_value()]->id);

	set_state_changed();
}

void mp_staging::on_color_select(ng::side_engine_ptr side, grid& row_grid)
{
	side->set_color(find_widget<menu_button>(&row_grid, "side_color", false).get_value());

	update_leader_display(side, row_grid);

	set_state_changed();
}

void mp_staging::on_team_select(window& window, ng::side_engine_ptr side, menu_button& team_menu, bool& handled, bool& halt)
{
	// Since we're not necessarily displaying every every team in the menu, we can't just
	// use the selected index to set a side's team. Instead, we grab the index we stored
	// in add_side_node from the selected config, which should correspond to the
	// appropriate entry in the connect_engine's team name vector.
	const unsigned team_index = team_menu.get_value_config()["team_index"].to_unsigned();

	if(team_index == side->team()) {
		return;
	}

	side->set_team(team_index);

	// First, remove the node from the tree
	find_widget<tree_view>(&window, "side_list", false).remove_node(side_tree_map_[side]);

	// Then add a new node as a child to the appropriate team's node
	add_side_node(window, side);

	set_state_changed();

	handled = true;
	halt = true;
}

void mp_staging::select_leader_callback(ng::side_engine_ptr side, grid& row_grid)
{
	gui2::dialogs::faction_select dlg(side->flg(), side->color_id(), side->index() + 1);
	dlg.show();

	if(dlg.get_retval() == retval::OK) {
		update_leader_display(side, row_grid);

		set_state_changed();
	}
}

template<void(ng::side_engine::*fptr)(int)>
void mp_staging::on_side_slider_change(ng::side_engine_ptr side, slider& slider)
{
	((*side).*fptr)(slider.get_value());

	set_state_changed();
}

void mp_staging::update_leader_display(ng::side_engine_ptr side, grid& row_grid)
{
	const std::string current_faction = (*side->flg().choosable_factions()[side->flg().current_faction_index()])["name"];

	// BIG FAT TODO: get rid of this shitty "null" string value in the FLG manager
	std::string current_leader = side->flg().current_leader() != "null" ? side->flg().current_leader() : font::unicode_em_dash;
	const std::string current_gender = side->flg().current_gender() != "null" ? side->flg().current_gender() : font::unicode_em_dash;

	// Sprite
	std::string new_image;

	if(side->flg().is_random_faction() || current_leader == "random") {
		new_image = ng::random_enemy_picture;
	}

	if(const unit_type* ut = unit_types.find(current_leader)) {
		const unit_type& type = ut->get_gender_unit_type(current_gender);

		new_image = formatter() << type.image() << "~RC(magenta>" << side->color_id() << ")";

		// We don't need the unit type id anymore, and can now replace this variable with the type name
		current_leader = type.type_name();
	}

	find_widget<drawing>(&row_grid, "leader_image", false).set_label(new_image);

	// Faction and leader
	if(!side->cfg()["name"].empty()) {
		current_leader = formatter() << side->cfg()["name"] << " (<i>" << current_leader << "</i>)";
	}

	find_widget<label>(&row_grid, "leader_type", false).set_label(current_leader);
	find_widget<label>(&row_grid, "leader_faction", false).set_label("<span color='#a69275'>" + current_faction + "</span>");

	// Gender
	if(current_gender != font::unicode_em_dash) {
		const std::string gender_icon = formatter() << "icons/icon-" << current_gender << ".png";

		image& icon = find_widget<image>(&row_grid, "leader_gender", false);

		icon.set_label(gender_icon);
		icon.set_tooltip(current_gender);
	}
}

void mp_staging::update_status_label_and_buttons(window& window)
{
	find_widget<label>(&window, "status_label", false).set_label(
		connect_engine_.can_start_game() ? "" : connect_engine_.sides_available()
			? _("Waiting for players to join...")
			: _("Waiting for players to choose factions...")
	);

	find_widget<button>(&window, "ok", false).set_active(connect_engine_.can_start_game());
}

void mp_staging::network_handler(window& window)
{
	// First, send off any changes if they've been accumulated
	if(state_changed_) {
		connect_engine_.update_and_send_diff();
	}

	// Next, check for any incoming changes
	config data;
	if(!state_changed_ && (!network_connection_ || !network_connection_->receive_data(data))) {
		return;
	}

	// Update chat
	find_widget<chatbox>(&window, "chat", false).process_network_data(data);

	// TODO: why is this needed...
	const bool was_able_to_start = connect_engine_.can_start_game();

	bool quit_signal_received;
	std::tie(quit_signal_received, std::ignore) = connect_engine_.process_network_data(data);

	if(quit_signal_received) {
		window.set_retval(retval::CANCEL);
	}

	// Update side leader displays
	// This is basically only needed when a new player joins and selects their faction
	for(auto& tree_entry : side_tree_map_) {
		ng::side_engine_ptr side = tree_entry.first;

		grid& row_grid = tree_entry.second->get_grid();

		update_leader_display(side, row_grid);

		std::vector<config> controller_names;
		for(const auto& controller : side->controller_options()) {
			controller_names.emplace_back(config {"label", controller.second});
		}

		menu_button& controller_selection = find_widget<menu_button>(&row_grid, "controller", false);

		controller_selection.set_values(controller_names, side->current_controller_index());
		controller_selection.set_active(controller_names.size() > 1);
	}

	// Update player list
	if(data.has_child("user")) {
		player_list_->update_list(data.child_range("user"));
	}

	// Update status label and buttons
	update_status_label_and_buttons(window);

	if(!was_able_to_start && connect_engine_.can_start_game()) {
		mp_ui_alerts::ready_for_start();
	}

	state_changed_ = false;
}

void mp_staging::post_show(window& window)
{
	if(update_timer_ != 0) {
		remove_timer(update_timer_);
		update_timer_ = 0;
	}

	if(window.get_retval() == retval::OK) {
		connect_engine_.start_game();
	} else {
		connect_engine_.leave_game();
	}
}

} // namespace dialogs
} // namespace gui2
