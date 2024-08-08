/*
	Copyright (C) 2008 - 2024
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

#include "gui/dialogs/multiplayer/mp_staging.hpp"

#include "ai/configuration.hpp"
#include "chat_log.hpp"
#include "formula/string_utils.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/multiplayer/faction_select.hpp"
#include "gui/dialogs/multiplayer/player_list_helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/chatbox.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "mp_ui_alerts.hpp"
#include "units/types.hpp"
#include "wesnothd_connection.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(mp_staging)

mp_staging::mp_staging(ng::connect_engine& connect_engine, wesnothd_connection* connection)
	: modal_dialog(window_id())
	, connect_engine_(connect_engine)
	, ai_algorithms_(ai::configuration::get_available_ais())
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
	window.set_escape_disabled(true);

	// Ctrl+G triggers 'I'm Ready' (ok) button's functionality
	connect_signal<event::SDL_KEY_DOWN>(std::bind(
		&mp_staging::signal_handler_sdl_key_down, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5, std::placeholders::_6));
	std::stringstream tooltip;
    tooltip << vgettext_impl("wesnoth", "Hotkey(s): ",  {{}});
    #ifdef __APPLE__
        tooltip << "cmd+g";
    #else
        tooltip << "ctrl+g";
    #endif
	find_widget<button>(get_window(), "ok", false).set_tooltip(tooltip.str());

	//
	// Set title and status widget states
	//
	label& title = find_widget<label>(&window, "title", false);
	title.set_label((formatter() << connect_engine_.params().name << " " << font::unicode_em_dash << " " << connect_engine_.scenario()["name"].t_str()).str());

	update_status_label_and_buttons();

	//
	// Set up sides list
	//
	for(const auto& side : connect_engine_.side_engines()) {
		if(side->allow_player() || game_config::debug) {
			add_side_node(side);;
		}
	}

	//
	// Initialize chatbox and game rooms
	//
	chatbox& chat = find_widget<chatbox>(&window, "chat", false);

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
	update_timer_ = add_timer(game_config::lobby_network_timer, std::bind(&mp_staging::network_handler, this), true);

	//
	// Set up the Lua plugin context
	//
	plugins_context_.reset(new plugins_context("Multiplayer Staging"));

	plugins_context_->set_callback("launch", [&window](const config&) { window.set_retval(retval::OK); }, false);
	plugins_context_->set_callback("quit",   [&window](const config&) { window.set_retval(retval::CANCEL); }, false);
	plugins_context_->set_callback("chat",   [&chat](const config& cfg) { chat.send_chat_message(cfg["message"], false); }, true);
}

int mp_staging::get_side_node_position(ng::side_engine_ptr side) const
{
	int position = 0;
	for(const auto& side_engine : connect_engine_.side_engines()) {
		if(side->team() == side_engine->team() && side->index() > side_engine->index()) {
			++position;
		}
	}

	return position;
}

template<typename... T>
tree_view_node& mp_staging::add_side_to_team_node(ng::side_engine_ptr side, T&&... params)
{
	static const widget_data empty_map;

	// If there is no team node in the map, this will return nullptr
	tree_view_node* team_node = team_tree_map_[side->team_name()];

	// Add a team node if none exists
	if(team_node == nullptr) {
		tree_view& tree = find_widget<tree_view>(get_window(), "side_list", false);

		widget_data tree_data;
		widget_item tree_item;

		tree_item["label"] = side->user_team_name();
		tree_data.emplace("tree_view_node_label", tree_item);

		team_node = &tree.add_node("team_header", tree_data);
		team_node->add_sibling("side_spacer", empty_map);

		team_tree_map_[side->team_name()] = team_node;
	}

	assert(team_node && "No team node found!");
	return team_node->add_child(std::forward<T>(params)...);
}

void mp_staging::add_side_node(ng::side_engine_ptr side)
{
	widget_data data;
	widget_item item;

	item["label"] = std::to_string(side->index() + 1);
	data.emplace("side_number", item);

	// TODO: don't hardcode magenta?
	item["label"] = "units/unknown-unit.png~RC(magenta>" + side->color_id() + ")";
	data.emplace("leader_image", item);

	item["label"] = "icons/icon-random.png";
	data.emplace("leader_gender", item);

	tree_view_node& node = add_side_to_team_node(side, "side_panel", data, get_side_node_position(side));

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

	const bool saved_game = connect_engine_.params().saved_game == saved_game_mode::type::midgame;

	//
	// AI Algorithm
	//
	int selection = 0;

	// We use an index-based loop in order to get the index of the selected option
	std::vector<config> ai_options;
	// If this is loading a saved game, we add an option at the beginning of the menu.
	// This results in a mismatch between the indices of ai_options and ai_algorithms_
	// that we need to account for later.
	if(saved_game) {
		ai_options.emplace_back("label", "Keep saved AI");
	}
	for(unsigned i = 0; i < ai_algorithms_.size(); ++i) {
		ai_options.emplace_back("label", ai_algorithms_[i]->text);

		if(ai_algorithms_[i]->id == side->ai_algorithm()) {
			selection = i;
		}
	}

	menu_button& ai_selection = find_widget<menu_button>(&row_grid, "ai_controller", false);

	ai_selection.set_values(ai_options, selection);

	connect_signal_notify_modified(ai_selection,
		std::bind(&mp_staging::on_ai_select, this, side, std::ref(ai_selection), saved_game));

	on_ai_select(side, ai_selection, saved_game);
	//
	// Controller
	//
	std::vector<config> controller_names;
	for(const auto& controller : side->controller_options()) {
		controller_names.emplace_back("label", controller.second);
	}

	menu_button& controller_selection = find_widget<menu_button>(&row_grid, "controller", false);

	controller_selection.set_values(controller_names, side->current_controller_index());
	controller_selection.set_active(controller_names.size() > 1);

	connect_signal_notify_modified(controller_selection,
		std::bind(&mp_staging::on_controller_select, this, side, std::ref(row_grid)));

	on_controller_select(side, row_grid);

	//
	// Leader controls
	//
	button& leader_select = find_widget<button>(&row_grid, "select_leader", false);

	//todo: shouldn't this also be disabled when the flg settings are locked.
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
	//todo: shouldn't this also be disabled when team settings are locked.
	team_selection.set_active(!saved_game);

	connect_signal_notify_modified(team_selection,
		std::bind(&mp_staging::on_team_select, this, side, std::ref(team_selection)));

	//
	// Colors
	//
	std::vector<config> color_options;
	for(const auto& color : side->color_options()) {
		color_options.emplace_back(
			"label", font::get_color_string_pango(color),
			"icon", (formatter() << "misc/status.png~RC(magenta>" << color << ")").str()
		);
	}

	menu_button& color_selection = find_widget<menu_button>(&row_grid, "side_color", false);

	color_selection.set_values(color_options, side->color());
	color_selection.set_active(!saved_game);
	color_selection.set_use_markup(true);

	connect_signal_notify_modified(color_selection,
		std::bind(&mp_staging::on_color_select, this, side, std::ref(row_grid)));

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

void mp_staging::on_ai_select(ng::side_engine_ptr side, menu_button& ai_menu, const bool saved_game)
{
	// If this is a saved game, we need to reduce the index by one, to account for
	// the "Keep saved AI" option having been added to the computer player menu
	int i = ai_menu.get_value();
	if(saved_game) {
		i--;
	}

	if(i < 0) {
		side->set_ai_algorithm("use_saved");
	} else {
		side->set_ai_algorithm(ai_algorithms_[i]->id);
	}

	set_state_changed();
}

void mp_staging::on_color_select(ng::side_engine_ptr side, grid& row_grid)
{
	side->set_color(find_widget<menu_button>(&row_grid, "side_color", false).get_value());

	update_leader_display(side, row_grid);

	set_state_changed();
}

void mp_staging::on_team_select(ng::side_engine_ptr side, menu_button& team_menu)
{
	// Since we're not necessarily displaying every every team in the menu, we can't just
	// use the selected index to set a side's team. Instead, we grab the index we stored
	// in add_side_node from the selected config, which should correspond to the
	// appropriate entry in the connect_engine's team name vector.
	const unsigned team_index = team_menu.get_value_config()["team_index"].to_unsigned();

	if(team_index == side->team()) {
		return;
	}

	// Note the old team so we can remove the node if empty after the side move.
	// Do this *before* setting the new team!
	const std::string old_team = side->team_name();
	side->set_team(team_index);

	auto& tree = find_widget<tree_view>(get_window(), "side_list", false);

	// First, remove the node from the tree
	auto node = tree.remove_node(side_tree_map_[side]);

	// Then add a new node as a child to the appropriate team's node
	add_side_to_team_node(side, std::move(node.first), get_side_node_position(side));

	tree_view_node* old_team_node = team_tree_map_[old_team];

	// Last, remove the old team node if it's now empty
	if(old_team_node->empty()) {
		// Decor node will be immediately after team node. Remove this first!
		tree.remove_node(old_team_node->get_node_below());
		tree.remove_node(old_team_node);

		team_tree_map_[old_team] = nullptr;
	}

	set_state_changed();
}

void mp_staging::select_leader_callback(ng::side_engine_ptr side, grid& row_grid)
{
	if(gui2::dialogs::faction_select::execute(side->flg(), side->color_id(), side->index() + 1)) {
		update_leader_display(side, row_grid);

		set_state_changed();
	}
}

template<void(ng::side_engine::*fptr)(int)>
void mp_staging::on_side_slider_change(ng::side_engine_ptr side, slider& slider)
{
	std::invoke(fptr, side, slider.get_value());

	set_state_changed();
}

void mp_staging::update_leader_display(ng::side_engine_ptr side, grid& row_grid)
{
	const std::string current_faction = side->flg().current_faction()["name"];

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

	find_widget<label>(&row_grid, "leader_type", false).set_label(current_leader == "random" ? _("Random") : current_leader);
	find_widget<label>(&row_grid, "leader_faction", false).set_label("<span color='#a69275'>" + current_faction + "</span>");

	// Gender
	if(current_gender != font::unicode_em_dash) {
		const std::string gender_icon = formatter() << "icons/icon-" << current_gender << ".png";

		image& icon = find_widget<image>(&row_grid, "leader_gender", false);
		icon.set_label(gender_icon);
	}
}

void mp_staging::update_status_label_and_buttons()
{
	find_widget<label>(get_window(), "status_label", false).set_label(
		connect_engine_.can_start_game() ? "" : connect_engine_.sides_available()
			? _("Waiting for players to join...")
			: _("Waiting for players to choose factions...")
	);

	find_widget<button>(get_window(), "ok", false).set_active(connect_engine_.can_start_game());
}

void mp_staging::network_handler()
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
	find_widget<chatbox>(get_window(), "chat", false).process_network_data(data);

	// TODO: why is this needed...
	const bool was_able_to_start = connect_engine_.can_start_game();

	bool quit_signal_received;
	std::tie(quit_signal_received, std::ignore) = connect_engine_.process_network_data(data);

	if(quit_signal_received) {
		set_retval(retval::CANCEL);
	}

	// Update side leader displays
	// This is basically only needed when a new player joins and selects their faction
	for(auto& tree_entry : side_tree_map_) {
		ng::side_engine_ptr side = tree_entry.first;

		grid& row_grid = tree_entry.second->get_grid();

		update_leader_display(side, row_grid);

		std::vector<config> controller_names;
		for(const auto& controller : side->controller_options()) {
			controller_names.emplace_back("label", controller.second);
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
	update_status_label_and_buttons();

	if(!was_able_to_start && connect_engine_.can_start_game()) {
		mp::ui_alerts::ready_for_start();
	}

	state_changed_ = false;
}

void mp_staging::signal_handler_sdl_key_down(const event::ui_event /*event*/,
										 bool& handled,
										 const SDL_Keycode key,
										 SDL_Keymod modifier)
{
    handled = true;

    #ifdef __APPLE__
        // Idiomatic modifier key in macOS computers.
        const SDL_Keycode modifier_key = KMOD_GUI;
    #else
        // Idiomatic modifier key in Microsoft desktop environments. Common in
        // GNU/Linux as well, to some extent.
        const SDL_Keycode modifier_key = KMOD_CTRL;
    #endif

    if ((key == SDLK_g) && (modifier & modifier_key)) {
        get_window()->set_retval(retval::OK);
        return;
    }
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
