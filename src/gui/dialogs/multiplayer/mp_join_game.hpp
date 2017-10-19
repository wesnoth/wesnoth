/*
   Copyright (C) 2008 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "gui/dialogs/modal_dialog.hpp"
#include "game_initialization/lobby_info.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"

#include "game_initialization/connect_engine.hpp"
#include "game_initialization/multiplayer.hpp"
#include "mp_game_settings.hpp"

class config;
class CVideo;

namespace gui2
{

class tree_view_node;

namespace dialogs
{

class mp_join_game : public modal_dialog, private plugin_executor
{
public:
	mp_join_game(saved_game& state, mp::lobby_info& lobby_info, wesnothd_connection& connection,
		const bool first_scenario = true, const bool observe_game = false);

	~mp_join_game();

	/**
	 * FIXME: We shouldn't need to pass a CVideo argument here. Optimally, this would be done in
	 * post_build or pre_show, but there's a bug where the Faction Select dialog does not display
	 * there. This should be changed to either of those functions once that's fixed.
	 */
	bool fetch_game_config(CVideo& video);
	bool started() const { return level_["started"].to_bool(); }
private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	void generate_side_list(window& window);

	void update_player_list(window& window, const config::const_child_itors& users);

	void network_handler(window& window);

	config& get_scenario();

	config level_;

	saved_game& state_;

	mp::lobby_info& lobby_info_;

	wesnothd_connection& network_connection_;

	size_t update_timer_;

	const bool first_scenario_;
	const bool observe_game_;

	bool stop_updates_;

	std::map<std::string, tree_view_node*> team_tree_map_;
};

} // namespace dialogs
} // namespace gui2
