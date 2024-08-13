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

#include "game_initialization/connect_engine.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"

class wesnothd_connection;

namespace ai
{
	struct description;
}

namespace gui2
{
class menu_button;
class slider;
class tree_view_node;
class player_list_helper;

namespace dialogs
{

class mp_staging : public modal_dialog, private plugin_executor
{
public:
	mp_staging(ng::connect_engine& connect_engine, wesnothd_connection* connection = nullptr);

	~mp_staging();

	DEFINE_SIMPLE_EXECUTE_WRAPPER(mp_staging)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

	template<typename... T>
	tree_view_node& add_side_to_team_node(ng::side_engine_ptr side, T&&... params);

	void add_side_node(ng::side_engine_ptr side);

	/**
	 * Find an appropriate position to insert a side node.
	 *
	 * This ensures the side nodes are always arranged by descending index order
	 * in each team group.
	 */
	int get_side_node_position(ng::side_engine_ptr side) const;

	void on_controller_select(ng::side_engine_ptr side, grid& row_grid);
	void on_ai_select(ng::side_engine_ptr side, menu_button& ai_menu, const bool saved_game);
	void on_color_select(ng::side_engine_ptr side, grid& row_grid);
	void on_team_select(ng::side_engine_ptr side, menu_button& team_menu);

	template<void(ng::side_engine::*fptr)(int)>
	void on_side_slider_change(ng::side_engine_ptr side, slider& slider);

	void select_leader_callback(ng::side_engine_ptr side, grid& row_grid);

	void update_leader_display(ng::side_engine_ptr side, grid& row_grid);
	void update_status_label_and_buttons();

	void network_handler();

	void set_state_changed()
	{
		state_changed_ = true;
	}

	void start_game()
	{
		get_window()->set_retval(retval::OK);
	}

	ng::connect_engine& connect_engine_;

	std::vector<ai::description*> ai_algorithms_;

	wesnothd_connection* network_connection_;

	std::size_t update_timer_;

	bool state_changed_;

	std::map<std::string, tree_view_node*> team_tree_map_;
	std::map<ng::side_engine_ptr, tree_view_node*> side_tree_map_;

	std::unique_ptr<player_list_helper> player_list_;
};

} // namespace dialogs
} // namespace gui2
