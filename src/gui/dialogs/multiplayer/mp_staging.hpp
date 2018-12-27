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

#pragma once

#include "ai/configuration.hpp"
#include "game_initialization/connect_engine.hpp"
#include "game_initialization/lobby_info.hpp"
#include "game_initialization/multiplayer.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/multiplayer/player_list_helper.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"
#include "mp_game_settings.hpp"

class config;

namespace gui2
{

class menu_button;
class slider;
class tree_view_node;

namespace dialogs
{

class mp_staging : public modal_dialog, private plugin_executor
{
public:
	mp_staging(ng::connect_engine& connect_engine, mp::lobby_info& lobby_info, wesnothd_connection* connection = nullptr);

	~mp_staging();

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	void add_side_node(window& window, ng::side_engine_ptr side);

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
	void on_team_select(window& window, ng::side_engine_ptr side, menu_button& team_menu);

	template<void(ng::side_engine::*fptr)(int)>
	void on_side_slider_change(ng::side_engine_ptr side, slider& slider);

	void select_leader_callback(ng::side_engine_ptr side, grid& row_grid);

	void update_leader_display(ng::side_engine_ptr side, grid& row_grid);
	void update_status_label_and_buttons(window& window);

	void network_handler(window& window);

	void set_state_changed()
	{
		state_changed_ = true;
	}

	ng::connect_engine& connect_engine_;

	std::vector<ai::description*> ai_algorithms_;

	mp::lobby_info& lobby_info_;

	wesnothd_connection* network_connection_;

	std::size_t update_timer_;

	bool state_changed_;

	std::map<std::string, tree_view_node*> team_tree_map_;
	std::map<ng::side_engine_ptr, tree_view_node*> side_tree_map_;

	std::unique_ptr<player_list_helper> player_list_;
};

} // namespace dialogs
} // namespace gui2
