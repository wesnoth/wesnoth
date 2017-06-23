/*
   Copyright (C) 2009 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
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

#include "gui/dialogs/lobby/lobby.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/lobby/player_info.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_join_game_password_prompt.hpp"
#include "gui/dialogs/preferences_dialog.hpp"
#include "gui/dialogs/helper.hpp"

#include "gui/core/log.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/chatbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/tree_view_node.hpp"

#include "addon/manager_ui.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "preferences/lobby.hpp"
#include "playmp_controller.hpp"
#include "wesnothd_connection.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(debug, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)
#define SCOPE_LB log_scope2(log_lobby, __func__)

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(mp_lobby)

void sub_player_list::init(window& w, const std::string& lbl, const bool unfolded)
{
	tree_view& parent_tree = find_widget<tree_view>(&w, "player_tree", false);

	std::map<std::string, string_map> tree_group_item;
	tree_group_item["tree_view_node_label"]["label"] = lbl;

	tree = &parent_tree.add_node("player_group", tree_group_item);

	if(unfolded) {
		tree->unfold();
	}

	tree_label = find_widget<label>(tree, "tree_view_node_label", false, true);
	label_player_count = find_widget<label>(tree, "player_count", false, true);

	assert(tree_label);
	assert(label_player_count);
}

void sub_player_list::update_player_count_label()
{
	assert(tree);
	assert(label_player_count);

	/**
	 * @todo Make sure setting visible resizes the widget.
	 *
	 * It doesn't work here since invalidate_layout is blocked, but the
	 * widget should also be able to handle it itself. Once done the
	 * setting of the label text can also be removed.
	 */
	label_player_count->set_label((formatter() << "(" << tree->count_children() << ")").str());
}

void player_list::init(window& w)
{
	active_game.init(w, _("Selected Game"));
	other_games.init(w, _("Other Games"));
	active_room.init(w, _("Current Room"));
	other_rooms.init(w, _("Lobby"), true);

	sort_by_name = find_widget<toggle_button>(&w, "player_list_sort_name", false, true);
	sort_by_relation = find_widget<toggle_button>(&w, "player_list_sort_relation", false, true);

	tree = find_widget<tree_view>(&w, "player_tree", false, true);
}

void player_list::update_sort_icons()
{
	sort_by_name->set_icon_name(sort_by_name->get_value() ? "lobby/sort-az.png" : "lobby/sort-az-off.png");
	sort_by_relation->set_icon_name(sort_by_relation->get_value() ? "lobby/sort-friend.png" : "lobby/sort-friend-off.png");
}

bool mp_lobby::logout_prompt()
{
	return show_prompt(_("Do you really want to log out?"));
}

mp_lobby::mp_lobby(const config& game_config, mp::lobby_info& info, wesnothd_connection &connection)
	: quit_confirmation(&mp_lobby::logout_prompt)
	, game_config_(game_config)
	, gamelistbox_(nullptr)
	, window_(nullptr)
	, lobby_info_(info)
	, filter_friends_(nullptr)
	, filter_ignored_(nullptr)
	, filter_slots_(nullptr)
	, filter_invert_(nullptr)
	, filter_text_(nullptr)
	, selected_game_id_()
	, player_list_()
	, player_list_dirty_(false)
	, gamelist_dirty_(false)
	, last_gamelist_update_(0)
	, gamelist_diff_update_(true)
	, wesnothd_connection_(connection)
	, lobby_update_timer_(0)
	, gamelist_id_at_row_()
	, delay_playerlist_update_(false)
	, delay_gamelist_update_(false)
{
	// Need to set this in the constructor, pre_show() is too late
	set_show_even_without_video(true);
	set_allow_plugin_skip(false);
}

struct lobby_delay_gamelist_update_guard
{
	lobby_delay_gamelist_update_guard(mp_lobby& l) : l(l)
	{
		l.delay_gamelist_update_ = true;
	}
	~lobby_delay_gamelist_update_guard()
	{
		l.delay_gamelist_update_ = false;
	}
	mp_lobby& l;
};

mp_lobby::~mp_lobby()
{
	if(lobby_update_timer_) {
		remove_timer(lobby_update_timer_);
	}
}

static bool fullscreen(CVideo& video)
{
	video.set_fullscreen(!preferences::fullscreen());
	return true;
}

void mp_lobby::post_build(window& win)
{
	/** @todo Should become a global hotkey after 1.8, then remove it here. */
	win.register_hotkey(hotkey::HOTKEY_FULLSCREEN, std::bind(fullscreen, std::ref(win.video())));

	/*** Local hotkeys. ***/
	win.register_hotkey(hotkey::HOTKEY_PREFERENCES, [this](event::dispatcher& w, hotkey::HOTKEY_COMMAND)->bool {
		show_preferences_button_callback(dynamic_cast<window&>(w));
		return true;
	});
}

namespace
{

void add_label_data(std::map<std::string, string_map>& map,
					const std::string& key,
					const std::string& label)
{
	string_map item;
	item["label"] = label;
	item["use_markup"] = "true";
	map.emplace(key, item);
}

void add_tooltip_data(std::map<std::string, string_map>& map,
					  const std::string& key,
					  const std::string& label)
{
	map[key]["tooltip"] = label;
}

void modify_grid_with_data(grid* grid, const std::map<std::string, string_map>& map)
{
	for(const auto& v : map) {
		const std::string& key = v.first;
		const string_map& strmap = v.second;

		widget* w = grid->find(key, false);
		if(!w) {
			continue;
		}

		styled_widget* c = dynamic_cast<styled_widget*>(w);
		if(!c) {
			continue;
		}

		for(const auto & vv : strmap) {
			if(vv.first == "label") {
				c->set_label(vv.second);
			} else if(vv.first == "tooltip") {
				c->set_tooltip(vv.second);
			}
		}
	}
}

void set_visible_if_exists(grid* grid, const std::string& id, bool visible)
{
	if(widget* w = grid->find(id, false)) {
		w->set_visible(visible ? widget::visibility::visible : widget::visibility::invisible);
	}
}

std::string colorize(const std::string& str, const std::string& color)
{
	return (formatter() << "<span color=\"" << color << "\">" << str << "</span>").str();
}

} // end anonymous namespace

void mp_lobby::update_gamelist()
{
	SCOPE_LB;
	gamelistbox_->clear();
	gamelist_id_at_row_.clear();
	lobby_info_.make_games_vector();

	int select_row = -1;
	for(unsigned i = 0; i < lobby_info_.games().size(); ++i) {
		const mp::game_info& game = *lobby_info_.games()[i];

		if(game.id == selected_game_id_) {
			select_row = i;
		}

		gamelist_id_at_row_.push_back(game.id);
		LOG_LB << "Adding game to listbox (1)" << game.id << "\n";
		grid* grid = &gamelistbox_->add_row(make_game_row_data(game));

		adjust_game_row_contents(game, gamelistbox_->get_item_count() - 1, grid);
	}

	if(select_row >= 0 && select_row != gamelistbox_->get_selected_row()) {
		gamelistbox_->select_row(select_row);
	}

	update_selected_game();
	gamelist_dirty_ = false;
	last_gamelist_update_ = SDL_GetTicks();
	lobby_info_.sync_games_display_status();
	lobby_info_.apply_game_filter();
	update_gamelist_header();
	gamelistbox_->set_row_shown(lobby_info_.games_visibility());
}

void mp_lobby::update_gamelist_diff()
{
	SCOPE_LB;
	lobby_info_.make_games_vector();
	int select_row = -1;
	unsigned list_i = 0;
	int list_rows_deleted = 0;

	std::vector<int> next_gamelist_id_at_row;
	for(unsigned i = 0; i < lobby_info_.games().size(); ++i) {
		const mp::game_info& game = *lobby_info_.games()[i];

		if(game.display_status == mp::game_info::NEW) {
			LOG_LB << "Adding game to listbox " << game.id << "\n";

			if(list_i != gamelistbox_->get_item_count()) {
				gamelistbox_->add_row(make_game_row_data(game), list_i);
				DBG_LB << "Added a game listbox row not at the end" << list_i
					   << " " << gamelistbox_->get_item_count() << "\n";
				list_rows_deleted--;
			} else {
				gamelistbox_->add_row(make_game_row_data(game));
			}

			grid* grid = gamelistbox_->get_row_grid(gamelistbox_->get_item_count() - 1);
			adjust_game_row_contents(game, gamelistbox_->get_item_count() - 1, grid);

			list_i++;
			next_gamelist_id_at_row.push_back(game.id);
		} else {
			if(list_i >= gamelistbox_->get_item_count()) {
				ERR_LB << "Ran out of listbox items -- triggering a full refresh\n";
				wesnothd_connection_.send_data(config("refresh_lobby"));
				return;
			}

			if(list_i + list_rows_deleted >= gamelist_id_at_row_.size()) {
				ERR_LB << "gamelist_id_at_row_ overflow! " << list_i << " + "
					   << list_rows_deleted
					   << " >= " << gamelist_id_at_row_.size()
					   << " -- triggering a full refresh\n";
				wesnothd_connection_.send_data(config("refresh_lobby"));
				return;
			}

			int listbox_game_id = gamelist_id_at_row_[list_i + list_rows_deleted];
			if(game.id != listbox_game_id) {
				ERR_LB << "Listbox game id does not match expected id "
					   << listbox_game_id << " " << game.id << " (row " << list_i << ")\n";
				wesnothd_connection_.send_data(config("refresh_lobby"));
				return;
			}

			if(game.display_status == mp::game_info::UPDATED) {
				LOG_LB << "Modifying game in listbox " << game.id << " (row " << list_i << ")\n";
				grid* grid = gamelistbox_->get_row_grid(list_i);
				modify_grid_with_data(grid, make_game_row_data(game));
				adjust_game_row_contents(game, list_i, grid, false);
				++list_i;
				next_gamelist_id_at_row.push_back(game.id);
			} else if(game.display_status == mp::game_info::DELETED) {
				LOG_LB << "Deleting game from listbox " << game.id << " (row "
					   << list_i << ")\n";
				gamelistbox_->remove_row(list_i);
				++list_rows_deleted;
			} else {
				// clean
				LOG_LB << "Clean game in listbox " << game.id << " (row " << list_i << ")\n";
				next_gamelist_id_at_row.push_back(game.id);
				++list_i;
			}
		}
	}

	for(unsigned i = 0; i < next_gamelist_id_at_row.size(); ++i) {
		if(next_gamelist_id_at_row[i] == selected_game_id_) {
			select_row = i;
		}
	}

	next_gamelist_id_at_row.swap(gamelist_id_at_row_);
	if(select_row >= static_cast<int>(gamelistbox_->get_item_count())) {
		ERR_LB << "Would select a row beyond the listbox" << select_row << " "
			   << gamelistbox_->get_item_count() << "\n";
		select_row = gamelistbox_->get_item_count() - 1;
	}

	if(select_row >= 0 && select_row != gamelistbox_->get_selected_row()) {
		gamelistbox_->select_row(select_row);
	}

	update_selected_game();
	gamelist_dirty_ = false;
	last_gamelist_update_ = SDL_GetTicks();
	lobby_info_.sync_games_display_status();
	lobby_info_.apply_game_filter();
	update_gamelist_header();
	gamelistbox_->set_row_shown(lobby_info_.games_visibility());
}

void mp_lobby::update_gamelist_header()
{
#ifndef GUI2_EXPERIMENTAL_LISTBOX
	const std::string games_string = vgettext("Games: showing $num_shown out of $num_total", {
		{"num_shown", std::to_string(lobby_info_.games_filtered().size())},
		{"num_total", std::to_string(lobby_info_.games().size())}
	});

	find_widget<label>(gamelistbox_, "map", false).set_label(games_string);
#endif
}

std::map<std::string, string_map> mp_lobby::make_game_row_data(const mp::game_info& game)
{
	std::map<std::string, string_map> data;

	std::string color_string;
	if(game.vacant_slots > 0) {
		color_string = (game.reloaded || game.started) ? "yellow" : "green";
	} else {
		color_string = game.observers ? "#ddd" : "red";
	}

	if(!game.have_era && (game.vacant_slots > 0 || game.observers)) {
		color_string = "#444";
	}

	add_label_data(data, "status", colorize(game.status, color_string));
	add_label_data(data, "name",   colorize(game.name, color_string));

	add_label_data(data, "era",             colorize(game.era, "#a69275"));
	add_label_data(data, "era_short",       game.era_short);
	add_label_data(data, "mods",            colorize(game.mod_info, "#a69275"));
	add_label_data(data, "map_info",        game.map_info);
	add_label_data(data, "scenario",        game.scenario);
	add_label_data(data, "map_size_text",   game.map_size_info);
	add_label_data(data, "time_limit",      game.time_limit);
	add_label_data(data, "gold_text",       game.gold);
	add_label_data(data, "xp_text",         game.xp);
	add_label_data(data, "vision_text",     game.vision);
	add_label_data(data, "time_limit_text", game.time_limit);
	add_label_data(data, "status",          game.status);

	if(game.observers) {
		add_label_data(data, "observer_icon", "misc/eye.png");
		add_tooltip_data(data, "observer_icon", _("Observers allowed"));
	} else {
		add_label_data(data, "observer_icon", "misc/no_observer.png");
		add_tooltip_data(data, "observer_icon", _("Observers not allowed"));
	}

	std::string vision_icon;
	if(game.fog) {
		vision_icon = game.shroud ? "misc/vision-fog-shroud.png" : "misc/vision-fog.png";
	} else {
		vision_icon = game.shroud ? "misc/vision-shroud.png" : "misc/vision-none.png";
	}

	add_label_data(data, "vision_icon", vision_icon);
	add_tooltip_data(data, "vision_icon", game.vision);
	return data;
}

void mp_lobby::adjust_game_row_contents(const mp::game_info& game,
										   int idx,
										   grid* grid, bool add_callbacks)
{
	find_widget<styled_widget>(grid, "name", false).set_use_markup(true);
	find_widget<styled_widget>(grid, "status", false).set_use_markup(true);

	toggle_panel& row_panel = find_widget<toggle_panel>(grid, "panel", false);

	row_panel.set_callback_mouse_left_double_click(
			std::bind(&mp_lobby::join_or_observe, this, idx));

	set_visible_if_exists(grid, "time_limit_icon",   !game.time_limit.empty());
	set_visible_if_exists(grid, "vision_fog",         game.fog);
	set_visible_if_exists(grid, "vision_shroud",      game.shroud);
	set_visible_if_exists(grid, "vision_none",      !(game.fog || game.shroud));
	set_visible_if_exists(grid, "observers_yes",      game.observers);
	set_visible_if_exists(grid, "shuffle_sides_icon", game.shuffle_sides);
	set_visible_if_exists(grid, "observers_no",      !game.observers);
	set_visible_if_exists(grid, "needs_password",     game.password_required);
	set_visible_if_exists(grid, "reloaded",           game.reloaded);
	set_visible_if_exists(grid, "started",            game.started);
	set_visible_if_exists(grid, "use_map_settings",   game.use_map_settings);
	set_visible_if_exists(grid, "registered_only",    game.registered_users_only);
	set_visible_if_exists(grid, "no_era",            !game.have_era);

	if(minimap* map = dynamic_cast<minimap*>(grid->find("minimap", false))) {
		map->set_config(&game_config_);
		map->set_map_data(game.map_data);
	}
	
	if(!add_callbacks) {
		return;
	}
		
	if(button* join_button = dynamic_cast<button*>(grid->find("join", false))) {
		connect_signal_mouse_left_click(
				*join_button,
				std::bind(&mp_lobby::join_global_button_callback,
							this,
							std::ref(*window_)));
		join_button->set_active(game.can_join());
	}

	if(button* observe_button = dynamic_cast<button*>(grid->find("observe", false))) {
		connect_signal_mouse_left_click(
				*observe_button,
				std::bind(&mp_lobby::observe_global_button_callback,
							this,
							std::ref(*window_)));
		observe_button->set_active(game.can_observe());
	}

}

void mp_lobby::update_gamelist_filter()
{
	DBG_LB << "mp_lobby::update_gamelist_filter\n";
	lobby_info_.apply_game_filter();
	DBG_LB << "Games in lobby_info: " << lobby_info_.games().size()
		   << ", games in listbox: " << gamelistbox_->get_item_count() << "\n";
	assert(lobby_info_.games().size() == gamelistbox_->get_item_count());
	gamelistbox_->set_row_shown(lobby_info_.games_visibility());

	update_gamelist_header();
}

void mp_lobby::update_playerlist()
{
	if(delay_playerlist_update_) {
		return;
	}

	SCOPE_LB;
	DBG_LB << "Playerlist update: " << lobby_info_.users().size() << "\n";
	lobby_info_.update_user_statuses(selected_game_id_, chatbox_->active_window_room());
	lobby_info_.sort_users(player_list_.sort_by_name->get_value_bool(),
						   player_list_.sort_by_relation->get_value_bool());

	bool lobby = false;
	if(mp::room_info* ri = chatbox_->active_window_room()) {
		if(ri->name() == "lobby") {
			lobby = true;
		}
	}

	assert(player_list_.active_game.tree);
	assert(player_list_.active_room.tree);
	assert(player_list_.other_games.tree);
	assert(player_list_.other_rooms.tree);

	player_list_.active_game.tree->clear();
	player_list_.active_room.tree->clear();
	player_list_.other_games.tree->clear();
	player_list_.other_rooms.tree->clear();

	for(auto userptr : lobby_info_.users_sorted()) {
		mp::user_info& user = *userptr;
		sub_player_list* target_list(nullptr);
		std::map<std::string, string_map> data;

		std::string name = user.name;

		std::stringstream icon_ss;
		icon_ss << "lobby/status";
		switch(user.state) {
			case mp::user_info::SEL_ROOM:
				icon_ss << "-lobby";
				target_list = &player_list_.active_room;
				if(lobby) {
					target_list = &player_list_.other_rooms;
				}
				break;
			case mp::user_info::LOBBY:
				icon_ss << "-lobby";
				target_list = &player_list_.other_rooms;
				break;
			case mp::user_info::SEL_GAME:
				name = colorize(name, "cyan");
				icon_ss << (user.observing ? "-obs" : "-playing");
				target_list = &player_list_.active_game;
				break;
			case mp::user_info::GAME:
				name = colorize(name, "red");
				icon_ss << (user.observing ? "-obs" : "-playing");
				target_list = &player_list_.other_games;
				break;
			default:
				ERR_LB << "Bad user state in lobby: " << user.name << ": "
					   << user.state << "\n";
				continue;
		}

		switch(user.relation) {
			case mp::user_info::ME:
				icon_ss << "-s";
				break;
			case mp::user_info::NEUTRAL:
				icon_ss << "-n";
				break;
			case mp::user_info::FRIEND:
				icon_ss << "-f";
				break;
			case mp::user_info::IGNORED:
				icon_ss << "-i";
				break;
			default:
				ERR_LB << "Bad user relation in lobby: " << user.relation
					   << "\n";
		}

		if(user.registered) {
			name = "<b>" + name + "</b>";
		}

		icon_ss << ".png";
		add_label_data(data, "player", name);
		add_label_data(data, "main_icon", icon_ss.str());

		if(!preferences::playerlist_group_players()) {
			target_list = &player_list_.other_rooms;
		}

		assert(target_list->tree);

		string_map tree_group_field;
		std::map<std::string, string_map> tree_group_item;

		/*** Add tree item ***/
		tree_group_field["label"] = icon_ss.str();
		tree_group_item["icon"] = tree_group_field;

		tree_group_field["label"] = name;
		tree_group_field["use_markup"] = "true";
		tree_group_item["name"] = tree_group_field;

		tree_view_node& player = target_list->tree->add_child("player", tree_group_item);

		find_widget<toggle_panel>(&player, "tree_view_node_label", false)
				.set_callback_mouse_left_double_click(std::bind(&mp_lobby::user_dialog_callback, this, userptr));
	}

	player_list_.active_game.update_player_count_label();
	player_list_.active_room.update_player_count_label();
	player_list_.other_rooms.update_player_count_label();
	player_list_.other_games.update_player_count_label();

	player_list_dirty_ = false;
}

void mp_lobby::update_selected_game()
{
	const int idx = gamelistbox_->get_selected_row();
	bool can_join = false, can_observe = false;

	if(idx >= 0) {
		const mp::game_info& game = *lobby_info_.games()[idx];
		can_observe = game.can_observe();
		can_join = game.can_join();
		selected_game_id_ = game.id;
	} else {
		selected_game_id_ = 0;
	}

	find_widget<button>(window_, "observe_global", false).set_active(can_observe);
	find_widget<button>(window_, "join_global", false).set_active(can_join);

	player_list_dirty_ = true;
}

bool mp_lobby::exit_hook(window& window)
{
	if(window.get_retval() == window::CANCEL) {
		return quit();
	}

	return true;
}

void mp_lobby::pre_show(window& window)
{
	SCOPE_LB;

	gamelistbox_ = find_widget<listbox>(&window, "game_list", false, true);
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(
			*gamelistbox_,
			std::bind(&mp_lobby::gamelist_change_callback, *this, std::ref(window)));
#else
	gamelistbox_->set_callback_value_change(
			dialog_callback<mp_lobby, &mp_lobby::gamelist_change_callback>);
#endif

	window.keyboard_capture(gamelistbox_);

	player_list_.init(window);

	player_list_.sort_by_name->set_value(preferences::playerlist_sort_name());
	player_list_.sort_by_relation->set_value(preferences::playerlist_sort_relation());
	player_list_.update_sort_icons();

	player_list_.sort_by_name->set_callback_state_change(
			std::bind(&mp_lobby::player_filter_callback, this, _1));
	player_list_.sort_by_relation->set_callback_state_change(
			std::bind(&mp_lobby::player_filter_callback, this, _1));

	window.set_enter_disabled(true);

	// Exit hook to add a confirmation when quitting the Lobby.
	window.set_exit_hook(std::bind(&mp_lobby::exit_hook, this, std::ref(window)));

	window_ = &window;

	chatbox_ = find_widget<chatbox>(&window, "chat", false, true);
	chatbox_->set_lobby_info(lobby_info_);
	chatbox_->set_wesnothd_connection(wesnothd_connection_);
	chatbox_->set_active_window_changed_callback([this]() { player_list_dirty_ = true; });

	find_widget<button>(&window, "create", false).set_retval(CREATE);

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "refresh", false),
		std::bind(&mp_lobby::refresh_button_callback, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "show_preferences", false),
		std::bind(&mp_lobby::show_preferences_button_callback, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "join_global", false),
		std::bind(&mp_lobby::join_global_button_callback, this, std::ref(window)));

	find_widget<button>(&window, "join_global", false).set_active(false);

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "observe_global", false),
		std::bind(&mp_lobby::observe_global_button_callback, this, std::ref(window)));

	find_widget<button>(&window, "observe_global", false).set_active(false);

	menu_button& replay_options = find_widget<menu_button>(&window, "replay_options", false);

	if(preferences::skip_mp_replay()) {
		replay_options.set_selected(1);
	}

	if(preferences::blindfold_replay()) {
		replay_options.set_selected(2);
	}

	replay_options.connect_click_handler(
			std::bind(&mp_lobby::skip_replay_changed_callback, this, std::ref(window)));

	filter_friends_ = find_widget<toggle_button>(&window, "filter_with_friends", false, true);
	filter_ignored_ = find_widget<toggle_button>(&window, "filter_without_ignored", false, true);
	filter_slots_   = find_widget<toggle_button>(&window, "filter_vacant_slots", false, true);
	filter_invert_  = find_widget<toggle_button>(&window, "filter_invert", false, true);
	filter_text_    = find_widget<text_box>(&window, "filter_text", false, true);

	filter_friends_->set_callback_state_change(
			std::bind(&mp_lobby::game_filter_change_callback, this, _1));
	filter_ignored_->set_callback_state_change(
			std::bind(&mp_lobby::game_filter_change_callback, this, _1));
	filter_slots_->set_callback_state_change(
			std::bind(&mp_lobby::game_filter_change_callback, this, _1));
	filter_invert_->set_callback_state_change(
			std::bind(&mp_lobby::game_filter_change_callback, this, _1));
	connect_signal_pre_key_press(
			*filter_text_,
			std::bind(&mp_lobby::game_filter_keypress_callback, this, _5));

	chatbox_->room_window_open("lobby", true, false);
	chatbox_->active_window_changed();
	game_filter_reload();

	// Force first update to be directly.
	mp_lobby::network_handler();
	lobby_update_timer_ = add_timer(
		game_config::lobby_network_timer, std::bind(&mp_lobby::network_handler, this), true);

	// Set up Lua plugin context
	plugins_context_.reset(new plugins_context("Multiplayer Lobby"));

	plugins_context_->set_callback("join",    [&, this](const config&) {
		if(do_game_join(get_game_index_from_id(selected_game_id_), false)) {
			window.set_retval(JOIN);
		}
	}, true);

	plugins_context_->set_callback("observe", [&, this](const config&) {
		if(do_game_join(get_game_index_from_id(selected_game_id_), true)) {
			window.set_retval(OBSERVE);
		}
	}, true);

	plugins_context_->set_callback("create", [&window](const config&) { window.set_retval(CREATE); }, true);
	plugins_context_->set_callback("quit", [&window](const config&) { window.set_retval(window::CANCEL); }, false);

	plugins_context_->set_callback("chat", [this](const config& cfg) { chatbox_->send_chat_message(cfg["message"], false); }, true);
	plugins_context_->set_callback("select_game", [this](const config& cfg) {
		selected_game_id_ = cfg.has_attribute("id") ? cfg["id"].to_int() : lobby_info_.games()[cfg["index"].to_int()]->id;
	}, true);

	plugins_context_->set_accessor("game_list",   [this](const config&) { return lobby_info_.gamelist(); });
	plugins_context_->set_accessor("game_config", [this](const config&) { return game_config_; });
}

void mp_lobby::post_show(window& /*window*/)
{
	window_ = nullptr;
	remove_timer(lobby_update_timer_);
	lobby_update_timer_ = 0;
	plugins_context_.reset();
}

void mp_lobby::network_handler()
{
	try {
		config data;
		if (wesnothd_connection_.receive_data(data)) {
			process_network_data(data);
		}
	} catch (wesnothd_error& e) {
		LOG_LB << "caught wesnothd_error in network_handler: " << e.message << "\n";
		throw;
	}

	if(gamelist_dirty_ && !delay_gamelist_update_ && (SDL_GetTicks() - last_gamelist_update_ > game_config::lobby_refresh)) {
		if(gamelist_diff_update_) {
			update_gamelist_diff();
		} else {
			update_gamelist();
			gamelist_diff_update_ = true;
		}
	}

	if(player_list_dirty_) {
		update_gamelist_filter();
		update_playerlist();
	}
}

void mp_lobby::process_network_data(const config& data)
{
	if(const config& error = data.child("error")) {
		throw wesnothd_error(error["message"]);
	} else if(data.child("gamelist")) {
		process_gamelist(data);
	} else if(const config& gamelist_diff = data.child("gamelist_diff")) {
		process_gamelist_diff(gamelist_diff);
	}

	chatbox_->process_network_data(data);
}

void mp_lobby::process_gamelist(const config& data)
{
	lobby_info_.process_gamelist(data);
	DBG_LB << "Received gamelist\n";
	gamelist_dirty_ = true;
	gamelist_diff_update_ = false;
}

void mp_lobby::process_gamelist_diff(const config& data)
{
	if(lobby_info_.process_gamelist_diff(data)) {
		DBG_LB << "Received gamelist diff\n";
		gamelist_dirty_ = true;
	} else {
		ERR_LB << "process_gamelist_diff failed!" << std::endl;
		wesnothd_connection_.send_data(config("refresh_lobby"));
	}
	const int joined = data.child_count("insert_child");
	const int left = data.child_count("remove_child");
	if(joined > 0 || left > 0) {
		if(left > joined) {
			do_notify(mp::NOTIFY_LOBBY_QUIT);
		} else {
			do_notify(mp::NOTIFY_LOBBY_JOIN);
		}
	}
}

void mp_lobby::observe_global_button_callback(window& window)
{
	if(do_game_join(gamelistbox_->get_selected_row(), true)) {
		window.set_retval(OBSERVE);
	}
}

void mp_lobby::join_global_button_callback(window& window)
{
	if(do_game_join(gamelistbox_->get_selected_row(), false)) {
		window.set_retval(JOIN);
	}
}

void mp_lobby::join_or_observe(int idx)
{
	const mp::game_info& game = *lobby_info_.games()[idx];
	if(do_game_join(idx, !game.can_join())) {
		window_->set_retval(game.can_join() ? JOIN : OBSERVE);
	}
}

static bool handle_addon_requirements_gui(CVideo& v, const std::vector<mp::game_info::required_addon>& reqs, mp::game_info::ADDON_REQ addon_outcome)
{
	if(addon_outcome == mp::game_info::CANNOT_SATISFY) {
		std::string e_title = _("Incompatible User-made Content.");
		std::string err_msg = _("This game cannot be joined because the host has out-of-date add-ons that are incompatible with your version. You might wish to suggest that the host's add-ons be updated.");

		err_msg +="\n\n";
		err_msg += _("Details:");
		err_msg += "\n";

		for(const mp::game_info::required_addon & a : reqs) {
			if (a.outcome == mp::game_info::CANNOT_SATISFY) {
				err_msg += font::unicode_bullet + " " + a.message + "\n";
			}
		}
		gui2::show_message(v, e_title, err_msg, message::auto_close);

		return false;
	} else if(addon_outcome == mp::game_info::NEED_DOWNLOAD) {
		std::string e_title = _("Missing User-made Content.");
		std::string err_msg = _("This game requires one or more user-made addons to be installed or updated in order to join.\nDo you want to try to install them?");

		err_msg +="\n\n";
		err_msg += _("Details:");
		err_msg += "\n";

		std::vector<std::string> needs_download;
		for(const mp::game_info::required_addon & a : reqs) {
			if(a.outcome == mp::game_info::NEED_DOWNLOAD) {
				err_msg += font::unicode_bullet + " " + a.message + "\n";

				needs_download.push_back(a.addon_id);
			}
		}

		assert(needs_download.size() > 0);

		if(gui2::show_message(v, e_title, err_msg, message::yes_no_buttons, true) == gui2::window::OK) {
			// Begin download session
			ad_hoc_addon_fetch_session(v, needs_download);

			return true;
		}
	}

	return false;
}

bool mp_lobby::do_game_join(int idx, bool observe)
{
	if(idx < 0 || idx >= static_cast<int>(lobby_info_.games().size())) {
		ERR_LB << "Requested join/observe of a game with index out of range: "
			   << idx << ", games size is " << lobby_info_.games().size() << "\n";
		return false;
	}
	const mp::game_info& game = *lobby_info_.games()[idx];
	if(observe) {
		if(!game.can_observe()) {
			ERR_LB << "Requested observe of a game with observers disabled" << std::endl;
			return false;
		}
	} else {
		if(!game.can_join()) {
			ERR_LB << "Requested join to a game with no vacant slots" << std::endl;
			return false;
		}
	}

	// Prompt user to download this game's required addons if its requirements have not been met
	if(game.addons_outcome != mp::game_info::SATISFIED) {
		if(game.required_addons.empty()) {
			gui2::show_error_message(window_->video(), _("Something is wrong with the addon version check database supporting the multiplayer lobby. Please report this at http://bugs.wesnoth.org."));
			return false;
		}

		if(!handle_addon_requirements_gui(window_->video(), game.required_addons, game.addons_outcome)) {
			return false;
		} else {
			// Addons have been downloaded, so the game_config and installed addons list need to be reloaded.
			// The lobby is closed and reopened.
			window_->set_retval(RELOAD_CONFIG);
			return false;
		}
	}

	config response;
	config& join = response.add_child("join");
	join["id"] = std::to_string(game.id);
	join["observe"] = observe;
	if(join && !observe && game.password_required) {
		std::string password;
		if(!gui2::dialogs::mp_join_game_password_prompt::execute(password, window_->video())) {
			return false;
		}

		join["password"] = password;
	}

	wesnothd_connection_.send_data(response);
	joined_game_id_ = game.id;
	return true;
}

void mp_lobby::refresh_button_callback(window& /*window*/)
{
	wesnothd_connection_.send_data(config("refresh_lobby"));
}

void mp_lobby::show_preferences_button_callback(window& window)
{
	gui2::dialogs::preferences_dialog::display(window.video(), game_config_);

	/**
	 * The screen size might have changed force an update of the size.
	 *
	 * @todo This might no longer be needed when gui2 is done.
	 */
	const SDL_Rect rect = screen_area();

	gui2::settings::gamemap_width  += rect.w - gui2::settings::screen_width;
	gui2::settings::gamemap_height += rect.h - gui2::settings::screen_height;
	gui2::settings::screen_width    = rect.w;
	gui2::settings::screen_height   = rect.h;

	/**
	 * The screen size might have changed force an update of the size.
	 *
	 * @todo This might no longer be needed when gui2 is done.
	 */
	window.invalidate_layout();

	wesnothd_connection_.send_data(config("refresh_lobby"));
}

void mp_lobby::game_filter_reload()
{
	lobby_info_.clear_game_filter();

	for(const auto& s : utils::split(filter_text_->get_value(), ' ')) {
		lobby_info_.add_game_filter([s](const mp::game_info& info)->bool {
			return info.match_string_filter(s);
		});
	}

	// TODO: make changing friend/ignore lists trigger a refresh
	if(filter_friends_->get_value()) {
		lobby_info_.add_game_filter([](const mp::game_info& info)->bool {
			return info.has_friends == true;
		});
	}

	if(filter_ignored_->get_value()) {
		lobby_info_.add_game_filter([](const mp::game_info& info)->bool {
			return info.has_ignored == false;
		});
	}

	if(filter_slots_->get_value()) {
		lobby_info_.add_game_filter([](const mp::game_info& info)->bool {
			return info.vacant_slots > 0;
		});
	}

	lobby_info_.set_game_filter_invert(filter_invert_->get_value_bool());
}

void mp_lobby::game_filter_keypress_callback(const SDL_Keycode key)
{
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		game_filter_reload();
		update_gamelist_filter();
	}
}

void mp_lobby::game_filter_change_callback(gui2::widget& /*widget*/)
{
	game_filter_reload();
	update_gamelist_filter();
}

void mp_lobby::gamelist_change_callback(window& /*window*/)
{
	update_selected_game();
}

void mp_lobby::player_filter_callback(gui2::widget& /*widget*/)
{
	player_list_.update_sort_icons();

	preferences::set_playerlist_sort_relation(player_list_.sort_by_relation->get_value_bool());
	preferences::set_playerlist_sort_name(player_list_.sort_by_name->get_value_bool());

	player_list_dirty_ = true;
	// window_->invalidate_layout();
}

void mp_lobby::user_dialog_callback(mp::user_info* info)
{
	lobby_player_info dlg(*chatbox_, *info, lobby_info_);

	lobby_delay_gamelist_update_guard g(*this);

	dlg.show(window_->video());

	delay_playerlist_update_ = true;

	if(dlg.result_open_whisper()) {
		lobby_chat_window* t = chatbox_->whisper_window_open(info->name, true);
		chatbox_->switch_to_window(t);
	}

	selected_game_id_ = info->game_id;
	// the commented out code below should be enough, but that'd delete the
	// playerlist and the widget calling this callback, so instead the game
	// will be selected on the next gamelist update.
	/*
	if (info->game_id != 0) {
		for (unsigned i = 0; i < lobby_info_.games_filtered().size(); ++i) {
		game_info& g = *lobby_info_.games_filtered()[i];
			if (info->game_id == g.id) {
			gamelistbox_->select_row(i);
				update_selected_game();
				break;
			}
		}
	}
	*/
	// do not update here as it can cause issues with removing the widget
	// from within it's event handler. Should get updated as soon as possible
	// update_gamelist();
	delay_playerlist_update_ = false;
	player_list_dirty_ = true;
	wesnothd_connection_.send_data(config("refresh_lobby"));
}

void mp_lobby::skip_replay_changed_callback(window& window)
{
	// TODO: this prefence should probably be controlled with an enum
	const int value = find_widget<menu_button>(&window, "replay_options", false).get_value();
	preferences::set_skip_mp_replay(value == 1);
	preferences::set_blindfold_replay(value == 2);
}

int mp_lobby::get_game_index_from_id(const int game_id) const
{
	if(mp::game_info* game = lobby_info_.get_game_by_id(game_id)) {
		return std::find(lobby_info_.games().begin(), lobby_info_.games().end(), game) - lobby_info_.games().begin();
	}

	return -1;
}

} // namespace dialogs
} // namespace gui2
