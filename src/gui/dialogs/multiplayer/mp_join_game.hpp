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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"

class saved_game;
class wesnothd_connection;

namespace gui2
{
class tree_view_node;
class player_list_helper;

namespace dialogs
{
class faction_select;

class mp_join_game : public modal_dialog, private plugin_executor
{
public:
	mp_join_game(saved_game& state, wesnothd_connection& connection,
		const bool first_scenario = true, const bool observe_game = false);

	~mp_join_game();

	bool fetch_game_config();
	bool started() const { return level_["started"].to_bool(); }
private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	/** @returns false if an error ocurred. */
	bool show_flg_select(int side_num, bool first_time = false);

	void generate_side_list();

	/**
	 * Will close the Faction Select dialog if it's open.
	 *
	 * This is used in @ref network_handler to dismiss the dialog if certain actions
	 * occur, such as the game starting.
	 *
	 * @todo maybe move this to a general-purpose close() function in @ref modal_dialog
	 * and @ref modeless_dialog? It could be useful.
	 */
	void close_faction_select_dialog_if_open();

	void network_handler();

	config& get_scenario();

	config level_;

	saved_game& state_;

	wesnothd_connection& network_connection_;

	std::size_t update_timer_;

	const bool first_scenario_;

	bool observe_game_;
	bool stop_updates_;

	std::map<std::string, tree_view_node*> team_tree_map_;

	std::unique_ptr<player_list_helper> player_list_;

	faction_select* flg_dialog_;
};

} // namespace dialogs
} // namespace gui2
