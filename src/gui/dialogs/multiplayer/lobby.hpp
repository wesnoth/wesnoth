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

#pragma once

#include "game_initialization/lobby_info.hpp"
#include "game_initialization/multiplayer.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/multiplayer/lobby_player_list_helper.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"
#include "quit_confirmation.hpp"

class wesnothd_connection;

namespace gui2
{
class grid;
class listbox;
class text_box;
class window;
class chatbox;

namespace dialogs
{
class mp_lobby : public modal_dialog, public quit_confirmation, private plugin_executor
{
public:
	mp_lobby(mp::lobby_info& info, wesnothd_connection& connection, int& joined_game);

	~mp_lobby();

	void update_gamelist();

protected:
	void update_visible_games();

	void update_gamelist_diff();

	void update_gamelist_filter();

	widget_data make_game_row_data(const mp::game_info& game);

	void adjust_game_row_contents(const mp::game_info& game, grid* grid, bool add_callbacks = true);

public:
	void update_playerlist();

	enum lobby_result {
		QUIT,
		JOIN,
		OBSERVE,
		CREATE,
		RELOAD_CONFIG
	};

private:
	void update_selected_game();

	/**
	 * Network polling callback
	 */
	void network_handler();

	void process_network_data(const config& data);

	void process_gamelist(const config& data);

	void process_gamelist_diff(const config& data);

	enum JOIN_MODE { DO_JOIN, DO_OBSERVE, DO_EITHER };

	/**
	 * Exits the lobby and enters the given game.
	 *
	 * This assembles the game request for the server and handles any applicable
	 * pre-join actions, such as prompting the player for the game's password or
	 * informing them additional content needs installing.
	 *
	 * The lobby window will be closed on completion, assuming an error wasn't
	 * encountered.
	 *
	 * @param game            Info on the game we're attempting to join.
	 * @param mode            Whether joining as player, observer or whichever works.
	 */
	void enter_game(const mp::game_info& game, JOIN_MODE mode);

	/** Entry wrapper for @ref enter_game, where game is located by index. */
	void enter_game_by_index(const int index, JOIN_MODE mode);

	/** Entry wrapper for @ref enter_game, where game is located by game id. */
	void enter_game_by_id(const int game_id, JOIN_MODE mode);

	/** Enter game by index, where index is the selected game listbox row. */
	void enter_selected_game(JOIN_MODE mode);

	void show_help_callback();

	void show_preferences_button_callback();

	void show_server_info();

	void open_profile_url();

	void open_match_history();

	void tab_switch_callback();

	void refresh_lobby();

	void game_filter_init();

	void game_filter_keypress_callback(const SDL_Keycode key);

	void user_dialog_callback(const mp::user_info* info);

	void skip_replay_changed_callback();

	static bool logout_prompt();

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	listbox* gamelistbox_;

	mp::lobby_info& lobby_info_;

	chatbox* chatbox_;

	field_bool* filter_friends_;
	field_bool* filter_ignored_;
	field_bool* filter_slots_;
	field_bool* filter_invert_;
	bool filter_auto_hosted_;

	text_box* filter_text_;

	int selected_game_id_;

	lobby_player_list_helper player_list_;

	bool player_list_dirty_;

	bool gamelist_dirty_;

	std::chrono::steady_clock::time_point last_lobby_update_;

	bool gamelist_diff_update_;

	wesnothd_connection &network_connection_;

	/** Timer for updating the lobby. */
	std::size_t lobby_update_timer_;

	std::vector<int> gamelist_id_at_row_;

	bool delay_playerlist_update_;

	bool delay_gamelist_update_;

	int& joined_game_id_;

	friend struct lobby_delay_gamelist_update_guard;

	static inline std::string server_information_ = "";
	static inline std::string announcements_ = "";
};

} // namespace dialogs
} // namespace gui2
