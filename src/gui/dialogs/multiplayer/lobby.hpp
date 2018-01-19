/*
   Copyright (C) 2009 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/tree_view.hpp"
#include "chat_events.hpp"
#include "game_initialization/lobby_info.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"
#include "game_initialization/multiplayer.hpp"
#include "quit_confirmation.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#endif
class wesnothd_connection;
namespace gui2
{

class grid;
class label;
#ifndef GUI2_EXPERIMENTAL_LISTBOX
class listbox;
#endif
class text_box;
class window;
class multi_page;
class toggle_button;
class chatbox;

namespace dialogs
{

struct sub_player_list
{
	void init(window& w, const std::string& label, const bool unfolded = false);
	void update_player_count_label();
	tree_view_node* tree;
	label* tree_label;
	label* label_player_count;
};

struct player_list
{
	void init(window& w);
	void update_sort_icons();
	sub_player_list active_game;
	sub_player_list active_room;
	sub_player_list other_rooms;
	sub_player_list other_games;

	toggle_button* sort_by_name;
	toggle_button* sort_by_relation;

	tree_view* tree;
};

class mp_lobby : public modal_dialog, public quit_confirmation, private plugin_executor
{
public:
	mp_lobby(const config& game_config, mp::lobby_info& info, wesnothd_connection &connection);

	~mp_lobby();

	int get_joined_game_id() const { return joined_game_id_; }

	void update_gamelist();

protected:
	void update_gamelist_header();

	void update_gamelist_diff();

	void update_gamelist_filter();

	std::map<std::string, string_map> make_game_row_data(const mp::game_info& game);

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

protected:

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
	 * informing them additonal content needs installing.
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

	void show_preferences_button_callback(window& window);

	void refresh_lobby();

	void game_filter_reload();

	void game_filter_change_callback();

	void game_filter_keypress_callback(const SDL_Keycode key);

	void gamelist_change_callback();

	void player_filter_callback();

	void user_dialog_callback(mp::user_info* info);

	void skip_replay_changed_callback(window& window);

	bool exit_hook(window& window);

	static bool logout_prompt();

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void post_build(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	const config& game_config_;

	listbox* gamelistbox_;

	mp::lobby_info& lobby_info_;

	chatbox* chatbox_;

	toggle_button* filter_friends_;

	toggle_button* filter_ignored_;

	toggle_button* filter_slots_;

	toggle_button* filter_invert_;

	text_box* filter_text_;

	int selected_game_id_;

	player_list player_list_;

	bool player_list_dirty_;

	bool gamelist_dirty_;

	unsigned last_gamelist_update_;

	bool gamelist_diff_update_;

	wesnothd_connection &network_connection_;

	/** Timer for updating the lobby. */
	size_t lobby_update_timer_;

	std::vector<int> gamelist_id_at_row_;

	bool delay_playerlist_update_;

	bool delay_gamelist_update_;

	int joined_game_id_;

	friend struct lobby_delay_gamelist_update_guard;
};

} // namespace dialogs
} // namespace gui2
