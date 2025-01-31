/*
	Copyright (C) 2009 - 2024
	by Tomasz Sniatowski <kailoran@gmail.com>
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

#include "gui/dialogs/multiplayer/lobby.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_join_game_password_prompt.hpp"
#include "gui/dialogs/multiplayer/player_info.hpp"
#include "gui/dialogs/preferences_dialog.hpp"

#include "gui/core/timer.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/chatbox.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/dialogs/server_info_dialog.hpp"
#include "gui/dialogs/multiplayer/match_history.hpp"

#include "addon/client.hpp"
#include "addon/manager_ui.hpp"
#include "chat_log.hpp"
#include "desktop/open.hpp"
#include "serialization/markup.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "preferences/preferences.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "wesnothd_connection.hpp"

#include <functional>

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(debug, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)
#define SCOPE_LB log_scope2(log_lobby, __func__)

namespace gui2::dialogs
{
REGISTER_DIALOG(mp_lobby)

bool mp_lobby::logout_prompt()
{
	return show_prompt(_("Do you really want to log out?"));
}

mp_lobby::mp_lobby(mp::lobby_info& info, wesnothd_connection& connection, int& joined_game)
	: modal_dialog(window_id())
	, quit_confirmation(&mp_lobby::logout_prompt)
	, gamelistbox_(nullptr)
	, lobby_info_(info)
	, chatbox_(nullptr)
	, filter_friends_(register_bool("filter_with_friends",
		  true,
		  []() {return prefs::get().fi_friends_in_game();},
		  [](bool v) {prefs::get().set_fi_friends_in_game(v);},
		  std::bind(&mp_lobby::update_gamelist_filter, this)))
	, filter_ignored_(register_bool("filter_with_ignored",
		  true,
		  []() {return prefs::get().fi_blocked_in_game();},
		  [](bool v) {prefs::get().set_fi_blocked_in_game(v);},
		  std::bind(&mp_lobby::update_gamelist_filter, this)))
	, filter_slots_(register_bool("filter_vacant_slots",
		  true,
		  []() {return prefs::get().fi_vacant_slots();},
		  [](bool v) {prefs::get().set_fi_vacant_slots(v);},
		  std::bind(&mp_lobby::update_gamelist_filter, this)))
	, filter_invert_(register_bool("filter_invert",
		  true,
		  []() {return prefs::get().fi_invert();},
		  [](bool v) {prefs::get().set_fi_invert(v);},
		  std::bind(&mp_lobby::update_gamelist_filter, this)))
	, filter_auto_hosted_(false)
	, filter_text_(nullptr)
	, selected_game_id_()
	, player_list_(std::bind(&mp_lobby::user_dialog_callback, this, std::placeholders::_1))
	, player_list_dirty_(true)
	, gamelist_dirty_(true)
	, last_lobby_update_()
	, gamelist_diff_update_(true)
	, network_connection_(connection)
	, lobby_update_timer_(0)
	, gamelist_id_at_row_()
	, delay_playerlist_update_(false)
	, delay_gamelist_update_(false)
	, joined_game_id_(joined_game)
{
	set_show_even_without_video(true);
	set_allow_plugin_skip(false);
	set_always_save_fields(true);

	/*** Local hotkeys. ***/
	window::register_hotkey(hotkey::HOTKEY_HELP,
		std::bind(&mp_lobby::show_help_callback, this));

	window::register_hotkey(hotkey::HOTKEY_PREFERENCES,
		std::bind(&mp_lobby::show_preferences_button_callback, this));
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

namespace
{
void modify_grid_with_data(grid* grid, const widget_data& map)
{
	for(const auto& v : map) {
		const std::string& key = v.first;
		const widget_item& strmap = v.second;

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

bool handle_addon_requirements_gui(const std::vector<mp::game_info::required_addon>& reqs, mp::game_info::addon_req addon_outcome)
{
	if(addon_outcome == mp::game_info::addon_req::CANNOT_SATISFY) {
		std::string e_title = _("Incompatible User-made Content");
		std::string err_msg = _("This game cannot be joined because the host has out-of-date add-ons that are incompatible with your version. You might wish to suggest that the host’s add-ons be updated.");

		err_msg +="\n\n";
		err_msg += _("Details:");
		err_msg += "\n";

		for(const mp::game_info::required_addon & a : reqs) {
			if (a.outcome == mp::game_info::addon_req::CANNOT_SATISFY) {
				err_msg += font::unicode_bullet + " " + a.message + "\n";
			}
		}
		gui2::show_message(e_title, err_msg, message::auto_close, true);

		return false;
	} else if(addon_outcome == mp::game_info::addon_req::NEED_DOWNLOAD) {
		std::string e_title = _("Missing User-made Content");
		std::string err_msg = _("This game requires one or more user-made addons to be installed or updated in order to join.\nDo you want to try to install them?");

		err_msg +="\n\n";
		err_msg += _("Details:");
		err_msg += "\n";

		std::vector<std::string> needs_download;
		for(const mp::game_info::required_addon & a : reqs) {
			if(a.outcome == mp::game_info::addon_req::NEED_DOWNLOAD) {
				err_msg += font::unicode_bullet + " " + a.message + "\n";

				needs_download.push_back(a.addon_id);
			}
		}

		assert(needs_download.size() > 0);

		if(gui2::show_message(e_title, err_msg, message::yes_no_buttons, true) == gui2::retval::OK) {
			// Begin download session
			try {
				return ad_hoc_addon_fetch_session(needs_download);
			} catch (const addons_client::user_exit&) {
			} catch (const addons_client::user_disconnect&) {
			}
		}
	}

	return false;
}

} // end anonymous namespace

void mp_lobby::update_gamelist()
{
	if(delay_gamelist_update_) return;

	SCOPE_LB;
	gamelistbox_->clear();
	gamelist_id_at_row_.clear();

	const auto finish_state_sync = lobby_info_.begin_state_sync();

	int select_row = -1;
	for(unsigned i = 0; i < lobby_info_.games().size(); ++i) {
		const mp::game_info& game = *lobby_info_.games()[i];

		if(game.id == selected_game_id_) {
			select_row = i;
		}

		gamelist_id_at_row_.push_back(game.id);
		LOG_LB << "Adding game to listbox (1)" << game.id;
		grid* grid = &gamelistbox_->add_row(make_game_row_data(game));

		adjust_game_row_contents(game, grid);
	}

	if(select_row >= 0 && select_row != gamelistbox_->get_selected_row()) {
		gamelistbox_->select_row(select_row);
	}

	update_selected_game();
	gamelist_dirty_ = false;
	last_lobby_update_ = std::chrono::steady_clock::now();
	finish_state_sync();
	update_visible_games();
}

void mp_lobby::update_gamelist_diff()
{
	if(delay_gamelist_update_) return;

	SCOPE_LB;
	int select_row = -1;
	unsigned list_i = 0;
	int list_rows_deleted = 0;

	const auto finish_state_sync = lobby_info_.begin_state_sync();

	std::vector<int> next_gamelist_id_at_row;
	for(unsigned i = 0; i < lobby_info_.games().size(); ++i) {
		const mp::game_info& game = *lobby_info_.games()[i];

		if(game.display_status == mp::game_info::disp_status::NEW) {
			// call void do_notify(notify_mode mode, const std::string& sender, const std::string& message)
			// sender will be the game_info.scenario (std::string) and message will be game_info.name (std::string)
			if (lobby_info_.is_game_visible(game)) {
				do_notify(mp::notify_mode::game_created, game.scenario, game.name);
			}

			LOG_LB << "Adding game to listbox " << game.id;

			if(list_i != gamelistbox_->get_item_count()) {
				gamelistbox_->add_row(make_game_row_data(game), list_i);
				DBG_LB << "Added a game listbox row not at the end" << list_i
					   << " " << gamelistbox_->get_item_count();
				list_rows_deleted--;
			} else {
				gamelistbox_->add_row(make_game_row_data(game));
			}

			grid* grid = gamelistbox_->get_row_grid(gamelistbox_->get_item_count() - 1);
			adjust_game_row_contents(game, grid);

			list_i++;
			next_gamelist_id_at_row.push_back(game.id);
		} else {
			if(list_i >= gamelistbox_->get_item_count()) {
				ERR_LB << "Ran out of listbox items -- triggering a full refresh";
				refresh_lobby();
				return;
			}

			if(list_i + list_rows_deleted >= gamelist_id_at_row_.size()) {
				ERR_LB << "gamelist_id_at_row_ overflow! " << list_i << " + "
					   << list_rows_deleted
					   << " >= " << gamelist_id_at_row_.size()
					   << " -- triggering a full refresh";
				refresh_lobby();
				return;
			}

			int listbox_game_id = gamelist_id_at_row_[list_i + list_rows_deleted];
			if(game.id != listbox_game_id) {
				ERR_LB << "Listbox game id does not match expected id "
					   << listbox_game_id << " " << game.id << " (row " << list_i << ")";
				refresh_lobby();
				return;
			}

			if(game.display_status == mp::game_info::disp_status::UPDATED) {
				LOG_LB << "Modifying game in listbox " << game.id << " (row " << list_i << ")";
				grid* grid = gamelistbox_->get_row_grid(list_i);
				modify_grid_with_data(grid, make_game_row_data(game));
				adjust_game_row_contents(game, grid, false);
				++list_i;
				next_gamelist_id_at_row.push_back(game.id);
			} else if(game.display_status == mp::game_info::disp_status::DELETED) {
				LOG_LB << "Deleting game from listbox " << game.id << " (row "
					   << list_i << ")";
				gamelistbox_->remove_row(list_i);
				++list_rows_deleted;
			} else {
				// clean
				LOG_LB << "Clean game in listbox " << game.id << " (row " << list_i << ")";
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
			   << gamelistbox_->get_item_count();
		select_row = gamelistbox_->get_item_count() - 1;
	}

	if(select_row >= 0 && select_row != gamelistbox_->get_selected_row()) {
		gamelistbox_->select_row(select_row);
	}

	update_selected_game();
	gamelist_dirty_ = false;
	last_lobby_update_ = std::chrono::steady_clock::now();
	finish_state_sync();
	update_visible_games();
}

void mp_lobby::update_visible_games()
{
	const std::string games_string = VGETTEXT("Games: showing $num_shown out of $num_total", {
		{"num_shown", std::to_string(lobby_info_.games_visibility().count())},
		{"num_total", std::to_string(lobby_info_.games().size())}
	});

	gamelistbox_->find_widget<label>("map").set_label(games_string);

	gamelistbox_->set_row_shown(lobby_info_.games_visibility());
}

widget_data mp_lobby::make_game_row_data(const mp::game_info& game)
{
	widget_data data;
	widget_item item;

	item["use_markup"] = "true";

	color_t color_string;
	if(game.vacant_slots > 0) {
		color_string = (game.reloaded || game.started) ? font::YELLOW_COLOR : font::GOOD_COLOR;
	}

	const std::string scenario_text = VGETTEXT("$game_name (Era: $era_name)", {
		{"game_name", game.scenario},
		{"era_name", game.era}
	});

	item["label"] = game.vacant_slots > 0 ? markup::span_color(color_string, game.name) : game.name;
	data.emplace("name", item);

	item["label"] = markup::span_color(font::GRAY_COLOR, game.type_marker, markup::italic(scenario_text));
	data.emplace("scenario", item);

	item["label"] = markup::span_color(color_string, game.status);
	data.emplace("status", item);

	return data;
}

void mp_lobby::adjust_game_row_contents(const mp::game_info& game, grid* grid, bool add_callbacks)
{
	grid->find_widget<styled_widget>("name").set_use_markup(true);
	grid->find_widget<styled_widget>("status").set_use_markup(true);

	toggle_panel& row_panel = grid->find_widget<toggle_panel>("panel");

	//
	// Game info
	//
	std::ostringstream ss;

	const auto mark_missing = [&ss]() {
		ss << ' ' << markup::span_color(font::BAD_COLOR, "(", _("era_or_mod^not installed"), ")");
	};

	ss << markup::tag("big", markup::span_color(font::TITLE_COLOR, _("Era"))) << "\n" << game.era;

	if(!game.have_era) {
		// NOTE: not using colorize() here deliberately to avoid awkward string concatenation.
		mark_missing();
	}

	ss << "\n\n" << markup::tag("big", markup::span_color(font::TITLE_COLOR, _("Modifications"))) << "\n";

	auto mods = game.mod_info;

	if(mods.empty()) {
		ss << _("active_modifications^None") << "\n";
	} else {
		for(const auto& mod : mods) {
			ss << mod.first;

			if(!mod.second) {
				mark_missing();
			}

			ss << '\n';
		}
	}

	// TODO: move to some general area of the code.
	const auto yes_or_no = [](bool val) { return val ? _("yes") : _("no"); };

	ss << "\n" << markup::tag("big", markup::span_color(font::TITLE_COLOR, _("Settings"))) << "\n";
	ss << _("Experience modifier:")   << " " << game.xp << "\n";
	ss << _("Gold per village:")      << " " << game.gold << "\n";
	ss << _("Map size:")              << " " << game.map_size_info << "\n";
	ss << _("Reloaded:")              << " " << yes_or_no(game.reloaded) << "\n";
	ss << _("Shared vision:")         << " " << game.vision << "\n";
	ss << _("Shuffle sides:")         << " " << yes_or_no(game.shuffle_sides) << "\n";
	ss << _("Time limit:")            << " " << game.time_limit << "\n";
	ss << _("Use map settings:")      << " " << yes_or_no(game.use_map_settings);

	image& info_icon = grid->find_widget<image>("game_info");

	if(!game.have_era || !game.have_all_mods || !game.required_addons.empty()) {
		info_icon.set_label("icons/icon-info-error.png");

		ss << "\n\n" << markup::span_color("#f00", markup::span_size("x-large", "! "));
		ss << _("One or more add-ons need to be installed\nin order to join this game.");
	} else {
		info_icon.set_label("icons/icon-info.png");
	}

	info_icon.set_tooltip(ss.str());

	//
	// Password icon
	//
	image& password_icon = grid->find_widget<image>("needs_password");

	if(game.password_required) {
		password_icon.set_visible(widget::visibility::visible);
	} else {
		password_icon.set_visible(widget::visibility::hidden);
	}

	//
	// Observer icon
	//
	image& observer_icon = grid->find_widget<image>("observer_icon");

	if(game.observers) {
		observer_icon.set_label("misc/eye.png");
		observer_icon.set_tooltip( _("Observers allowed"));
	} else {
		observer_icon.set_label("misc/no_observer.png");
		observer_icon.set_tooltip( _("Observers not allowed"));
	}

	//
	// Minimap
	//
	minimap& map = grid->find_widget<minimap>("minimap");

	map.set_map_data(game.map_data);

	if(!add_callbacks) {
		return;
	}

	connect_signal_mouse_left_double_click(row_panel,
		std::bind(&mp_lobby::enter_game_by_id, this, game.id, DO_EITHER));
}

void mp_lobby::update_gamelist_filter()
{
	DBG_LB << "mp_lobby::update_gamelist_filter";
	lobby_info_.apply_game_filter();
	DBG_LB << "Games in lobby_info: " << lobby_info_.games().size()
		   << ", games in listbox: " << gamelistbox_->get_item_count();
	assert(lobby_info_.games().size() == gamelistbox_->get_item_count());

	update_visible_games();
}

void mp_lobby::update_playerlist()
{
	if(delay_playerlist_update_) return;

	SCOPE_LB;
	DBG_LB << "Playerlist update: " << lobby_info_.users().size();

	player_list_.update(lobby_info_.users(), selected_game_id_);

	player_list_dirty_ = false;
	last_lobby_update_ = std::chrono::steady_clock::now();
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

	find_widget<button>("observe_global").set_active(can_observe);
	find_widget<button>("join_global").set_active(can_join);

	player_list_dirty_ = true;
}

void mp_lobby::pre_show()
{
	SCOPE_LB;

	gamelistbox_ = find_widget<listbox>("game_list", false, true);

	connect_signal_notify_modified(*gamelistbox_,
			std::bind(&mp_lobby::update_selected_game, this));

	player_list_.init(*this);

	set_enter_disabled(true);

	// Exit hook to add a confirmation when quitting the Lobby.
	set_exit_hook(window::exit_hook::always, [this] { return get_retval() == retval::CANCEL ? quit() : true; });

	chatbox_ = find_widget<chatbox>("chat", false, true);

	keyboard_capture(chatbox_);

	chatbox_->set_active_window_changed_callback([this]() { player_list_dirty_ = true; });
	chatbox_->load_log(default_chat_log, true);

	find_widget<button>("create").set_retval(CREATE);

	connect_signal_mouse_left_click(
		find_widget<button>("show_preferences"),
		std::bind(&mp_lobby::show_preferences_button_callback, this));

	connect_signal_mouse_left_click(
		find_widget<button>("join_global"),
		std::bind(&mp_lobby::enter_selected_game, this, DO_JOIN));

	find_widget<button>("join_global").set_active(false);

	connect_signal_mouse_left_click(
		find_widget<button>("observe_global"),
		std::bind(&mp_lobby::enter_selected_game, this, DO_OBSERVE));

	connect_signal_mouse_left_click(
		find_widget<button>("server_info"),
		std::bind(&mp_lobby::show_server_info, this));

	find_widget<button>("observe_global").set_active(false);

	menu_button& replay_options = find_widget<menu_button>("replay_options");

	if(prefs::get().skip_mp_replay()) {
		replay_options.set_selected(1);
	}

	if(prefs::get().blindfold_replay()) {
		replay_options.set_selected(2);
	}

	connect_signal_notify_modified(replay_options,
		std::bind(&mp_lobby::skip_replay_changed_callback, this));

	filter_text_    = find_widget<text_box>("filter_text", false, true);

	connect_signal_pre_key_press(
			*filter_text_,
			std::bind(&mp_lobby::game_filter_keypress_callback, this, std::placeholders::_5));

	chatbox_->room_window_open(N_("lobby"), true, false);
	chatbox_->active_window_changed();

	game_filter_init();

	// Force first update to be directly.
	update_gamelist();
	update_playerlist();

	// TODO: currently getting a crash in the chatbox if we use
	// -- vultraz, 2017-11-10
	//mp_lobby::network_handler();

	lobby_update_timer_ = add_timer(
		game_config::lobby_network_timer, std::bind(&mp_lobby::network_handler, this), true);

	//
	// Profile box
	//
	if(panel* profile_panel = find_widget<panel>("profile", false, false)) {
		auto your_info = std::find_if(lobby_info_.users().begin(), lobby_info_.users().end(),
			[](const auto& u) { return u.get_relation() == mp::user_info::user_relation::ME; });

		if(your_info != lobby_info_.users().end()) {
			profile_panel->find_widget<label>("username").set_label(your_info->name);

			auto& profile_button = profile_panel->find_widget<button>("view_profile");
			connect_signal_mouse_left_click(profile_button, std::bind(&mp_lobby::open_profile_url, this));

			auto& history_button = profile_panel->find_widget<button>("view_match_history");
			connect_signal_mouse_left_click(history_button, std::bind(&mp_lobby::open_match_history, this));
		}
	}

	listbox& tab_bar = find_widget<listbox>("games_list_tab_bar");
	connect_signal_notify_modified(tab_bar, std::bind(&mp_lobby::tab_switch_callback, this));

	// Set up Lua plugin context
	plugins_context_.reset(new plugins_context("Multiplayer Lobby"));

	plugins_context_->set_callback("join",    [&, this](const config&) {
		enter_game_by_id(selected_game_id_, DO_JOIN);
	}, true);

	plugins_context_->set_callback("observe", [&, this](const config&) {
		enter_game_by_id(selected_game_id_, DO_OBSERVE);
	}, true);

	plugins_context_->set_callback("create", [this](const config&) { set_retval(CREATE); }, true);
	plugins_context_->set_callback("quit", [this](const config&) { set_retval(retval::CANCEL); }, false);

	plugins_context_->set_callback("chat", [this](const config& cfg) { chatbox_->send_chat_message(cfg["message"], false); }, true);
	plugins_context_->set_callback("select_game", [this](const config& cfg) {
		selected_game_id_ = cfg.has_attribute("id") ? cfg["id"].to_int() : lobby_info_.games()[cfg["index"].to_int()]->id;
	}, true);

	plugins_context_->set_accessor("game_list",   [this](const config&) { return lobby_info_.gamelist(); });
}

void mp_lobby::tab_switch_callback()
{
	filter_auto_hosted_ = find_widget<listbox>("games_list_tab_bar").get_selected_row() == 1;
	update_gamelist_filter();
}

void mp_lobby::open_profile_url()
{
	const mp::user_info* info = player_list_.get_selected_info();
	if(info && info->forum_id != 0) {
		desktop::open_object(mp::get_profile_link(info->forum_id));
	}
}

void mp_lobby::post_show()
{
	remove_timer(lobby_update_timer_);
	lobby_update_timer_ = 0;
	plugins_context_.reset();
}

void mp_lobby::open_match_history()
{
	const mp::user_info* info = player_list_.get_selected_info();
	if(info) {
		mp_match_history::display(info->name, network_connection_);
	}
}

void mp_lobby::network_handler()
{
	try {
		config data;
		if (network_connection_.receive_data(data)) {
			process_network_data(data);
		}
	} catch (const wesnothd_error& e) {
		LOG_LB << "caught wesnothd_error in network_handler: " << e.message;
		throw;
	}

	if(std::chrono::steady_clock::now() - last_lobby_update_ < game_config::lobby_refresh) {
		return;
	}

	if(gamelist_diff_update_ && !lobby_info_.gamelist_initialized()) {
		//don't process a corrupted gamelist further to prevent crashes later.
		return;
	}

	if(gamelist_dirty_ && !delay_gamelist_update_) {
		if(gamelist_diff_update_) {
			update_gamelist_diff();
		} else {
			update_gamelist();
			gamelist_diff_update_ = true;
		}
	}

	if(player_list_dirty_ && !delay_playerlist_update_) {
		update_gamelist_filter();
		update_playerlist();
	}
}

void mp_lobby::process_network_data(const config& data)
{
	if(auto error = data.optional_child("error")) {
		throw wesnothd_error(error["message"]);
	} else if(data.has_child("gamelist")) {
		process_gamelist(data);
	} else if(auto gamelist_diff = data.optional_child("gamelist_diff")) {
		process_gamelist_diff(*gamelist_diff);
	} else if(auto info = data.optional_child("message")) {
		if(info["type"] == "server_info") {
			server_information_ = info["message"].str();
			return;
		} else if(info["type"] == "announcements") {
			announcements_ = info["message"].str();
			return;
		}
	}

	chatbox_->process_network_data(data);
}

void mp_lobby::process_gamelist(const config& data)
{
	if(delay_gamelist_update_ || delay_playerlist_update_) return;

	lobby_info_.process_gamelist(data);
	DBG_LB << "Received gamelist";
	gamelist_dirty_ = true;
	gamelist_diff_update_ = false;
}

void mp_lobby::process_gamelist_diff(const config& data)
{
	if(delay_gamelist_update_ || delay_playerlist_update_) return;

	if(lobby_info_.process_gamelist_diff(data)) {
		DBG_LB << "Received gamelist diff";
		gamelist_dirty_ = true;
	} else {
		ERR_LB << "process_gamelist_diff failed!";
		refresh_lobby();
	}
	const int joined = data.child_count("insert_child");
	const int left = data.child_count("remove_child");
	if(joined > 0 || left > 0) {
		if(left > joined) {
			do_notify(mp::notify_mode::lobby_quit);
		} else {
			do_notify(mp::notify_mode::lobby_join);
		}
	}
}

void mp_lobby::enter_game(const mp::game_info& game, JOIN_MODE mode)
{
	switch(mode) {
	case DO_JOIN:
		if(!game.can_join()) {
			ERR_LB << "Attempted to join a game with no vacant slots";
			return;
		}

		break;
	case DO_OBSERVE:
		if(!game.can_observe()) {
			ERR_LB << "Attempted to observe a game with observers disabled";
			return;
		}

		break;
	case DO_EITHER:
		if(game.can_join()) {
			mode = DO_JOIN;
		} else if(game.can_observe()) {
			mode = DO_OBSERVE;
		} else {
			DBG_LB << "Cannot join or observe a game.";
			return;
		}

		break;
	}

	// prompt moderators for whether they want to join a game with observers disabled
	if(!game.observers && mp::logged_in_as_moderator()) {
		if(gui2::show_message(_("Observe"), _("This game doesn’t allow observers. Observe using moderator rights anyway?"), gui2::dialogs::message::yes_no_buttons) != gui2::retval::OK) {
			return;
		}
	}

	const bool try_join = mode == DO_JOIN;
	const bool try_obsv = mode == DO_OBSERVE;

	// Prompt user to download this game's required addons if its requirements have not been met
	if(game.addons_outcome != mp::game_info::addon_req::SATISFIED) {
		if(game.required_addons.empty()) {
			gui2::show_error_message(_("Something is wrong with the addon version check database supporting the multiplayer lobby. Please report this at https://bugs.wesnoth.org."));
			return;
		}

		if(!handle_addon_requirements_gui(game.required_addons, game.addons_outcome)) {
			return;
		}

		// Addons have been downloaded, so the game_config and installed addons list need to be reloaded.
		// The lobby is closed and reopened.
		set_retval(RELOAD_CONFIG);
		return;
	}

	config response;

	config& join_data = response.add_child("join");
	join_data["id"] = std::to_string(game.id);
	join_data["observe"] = try_obsv;

	if(mp::logged_in_as_moderator() && game.password_required) {
		if(gui2::show_message(_("Join"), _("This game is password protected. Join using moderator rights anyway?"), gui2::dialogs::message::yes_no_buttons) != gui2::retval::OK) {
			return;
		}
	} else if(!join_data.empty() && game.password_required) {
		std::string password;

		if(!gui2::dialogs::mp_join_game_password_prompt::execute(password)) {
			return;
		}

		join_data["password"] = password;
	}

	mp::send_to_server(response);
	joined_game_id_ = game.id;

	// We're all good. Close lobby and proceed to game!
	set_retval(try_join ? JOIN : OBSERVE);
}

void mp_lobby::enter_game_by_index(const int index, JOIN_MODE mode)
{
	try {
		enter_game(*lobby_info_.games().at(index), mode);
	} catch(const std::out_of_range&) {
		// Game index was invalid!
		ERR_LB << "Attempted to join/observe a game with index out of range: " << index << ". "
		       << "Games vector size is " << lobby_info_.games().size();
	}
}

void mp_lobby::enter_game_by_id(const int game_id, JOIN_MODE mode)
{
	mp::game_info* game_ptr = lobby_info_.get_game_by_id(game_id);

	if(!game_ptr) {
		ERR_LB << "Attempted to join/observe a game with an invalid id: " << game_id;
		return;
	}

	enter_game(*game_ptr, mode);
}

void mp_lobby::enter_selected_game(JOIN_MODE mode)
{
	enter_game_by_index(gamelistbox_->get_selected_row(), mode);
}

void mp_lobby::refresh_lobby()
{
	mp::send_to_server(config("refresh_lobby"));
}

void mp_lobby::show_help_callback()
{
	help::show_help();
}

void mp_lobby::show_preferences_button_callback()
{
	gui2::dialogs::preferences_dialog::display();

	refresh_lobby();
}

void mp_lobby::show_server_info()
{
	server_info::display(server_information_, announcements_);
}

void mp_lobby::game_filter_init()
{
	lobby_info_.clear_game_filters();

	lobby_info_.add_game_filter([this](const mp::game_info& info) {
		for(const auto& s : utils::split(filter_text_->get_value(), ' ')) {
			if(!info.match_string_filter(s)) {
				return false;
			}
		}

		return true;
	});

	lobby_info_.add_game_filter([this](const mp::game_info& info) {
		return filter_friends_->get_widget_value() ? info.has_friends == true : true;
	});

	// Unlike the friends filter, this is an inclusion filter (do we want to also show
	// games with blocked players) rather than an exclusion filter (do we want to show
	// only games with friends).
	lobby_info_.add_game_filter([this](const mp::game_info& info) {
		return filter_ignored_->get_widget_value() == false ? info.has_ignored == false : true;
	});

	lobby_info_.add_game_filter([this](const mp::game_info& info) {
		return filter_slots_->get_widget_value() ? info.vacant_slots > 0 : true;
	});

	lobby_info_.add_game_filter([this](const mp::game_info& info) {
		return info.auto_hosted == filter_auto_hosted_;
	});

	lobby_info_.set_game_filter_invert(
		[this](bool val) { return filter_invert_->get_widget_value() ? !val : val; });
}

void mp_lobby::game_filter_keypress_callback(const SDL_Keycode key)
{
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		update_gamelist_filter();
	}
}

void mp_lobby::user_dialog_callback(const mp::user_info* info)
{
	delay_playerlist_update_ = true;
	lobby_delay_gamelist_update_guard g(*this);

	lobby_player_info dlg(*chatbox_, *info, lobby_info_);
	dlg.show();

	if(dlg.result_open_whisper()) {
		lobby_chat_window* t = chatbox_->whisper_window_open(info->name, true);
		chatbox_->switch_to_window(t);
	}

	selected_game_id_ = info->game_id;

	// do not update here as it can cause issues with removing the widget
	// from within it's event handler. Should get updated as soon as possible
	// update_gamelist();
	delay_playerlist_update_ = false;
	player_list_dirty_ = true;
	refresh_lobby();
}

void mp_lobby::skip_replay_changed_callback()
{
	// TODO: this prefence should probably be controlled with an enum
	const int value = find_widget<menu_button>("replay_options").get_value();
	prefs::get().set_skip_mp_replay(value == 1);
	prefs::get().set_blindfold_replay(value == 2);
}

} // namespace dialogs
