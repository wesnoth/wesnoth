/*
   Copyright (C) 2008 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_JOIN_GAME_HPP_INCLUDED
#define GUI_DIALOGS_MP_JOIN_GAME_HPP_INCLUDED

#include "ai/configuration.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/dialogs/lobby/info.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"

#include "game_initialization/connect_engine.hpp"
#include "game_initialization/multiplayer.hpp"
#include "mp_game_settings.hpp"

class config;
class CVideo;

namespace gui2
{

class ttree_view_node;

class tmp_join_game : public tdialog, private plugin_executor
{
public:
	tmp_join_game(saved_game& state, lobby_info& lobby_info, wesnothd_connection& connection,
		const bool first_scenario = true, const bool observe_game = false);

	~tmp_join_game();

	/**
	 * FIXME: We shouldn't need to pass a CVideo argument here. Optimally, this would be done in
	 * post_build or pre_show, but there's a bug where the Faction Select dialog does not display
	 * there. This should be changed to either of those functions once that's fixed.
	 */
	bool fetch_game_config(CVideo& video);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void generate_side_list(twindow& window);

	void update_player_list(twindow& window);

	void network_handler(twindow& window);

	config& get_scenario();

	config level_;

	saved_game& state_;

	lobby_info& lobby_info_;

	wesnothd_connection& wesnothd_connection_;

	size_t update_timer_;

	const bool first_scenario_;
	const bool observe_game_;

	bool stop_updates_;

	std::map<std::string, ttree_view_node*> team_tree_map_;
};

} // namespace gui2

#endif
