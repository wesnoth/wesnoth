/* $Id: replay_controller.hpp 7396 2005-07-02 21:37:20Z ott $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef REPLAY_CONTROLLER_H_INCLUDED
#define REPLAY_CONTROLLER_H_INCLUDED

#include "display.hpp"
#include "font.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "halo.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "playlevel.hpp"
#include "preferences_display.hpp"
#include "tooltips.hpp"
#include "wml_separators.hpp"

#include <vector>

class replay_controller : public hotkey::command_executor, public events::handler
{
public:
	replay_controller(const config& level, const game_data& gameinfo, game_state& state_of_game,
		const int ticks, const int num_turns, const config& game_config, CVideo& video,
		const std::vector<config*>& story);
	~replay_controller();

	void handle_event(const SDL_Event& event);
	void replay_slice();
	bool can_execute_command(hotkey::HOTKEY_COMMAND command) const;

	std::vector<team>& get_teams();
	unit_map get_units();
	display& get_gui();
	gamemap& get_map();
	const gamestatus& get_status();
	const int get_player_number();
	const bool is_loading_game();

	//event handlers
	void objectives();
	void show_statistics();
	void unit_list();
	void status_table();
	void save_game();
	void load_game();
	void preferences();
	void show_chat_log();
	void show_help();
	void play_replay();
	void reset_replay();
	void stop_replay();
	void replay_next_turn();
	void replay_next_side();
	void replay_switch_fog();
	void replay_switch_shroud();
	void replay_skip_animation();

	std::vector<team> teams_, teams_start_;
private:
	void init(CVideo& video, const std::vector<config*>& story);
	void init_managers();
	void play_turn();
	void play_side(const int team_index);
	void update_teams();
	void update_gui();

	//managers
	const verification_manager verify_manager_;
	teams_manager team_manager_;
	halo::manager* halo_manager_;
	font::floating_label_context labels_manager_;
	preferences::display_manager* prefs_disp_manager_;
	tooltips::manager* tooltips_manager_;
	game_events::manager* events_manager_;
	help::help_manager help_manager_;

	const config& level_;
	const config& game_config_;
	const game_data& gameinfo_;
	game_state& gamestate_, gamestate_start_;
	display* gui_;
	gamestatus status_, status_start_;
	gamemap map_;
	unit_map units_, units_start_;
	events::mouse_handler mouse_handler_;
	events::menu_handler menu_handler_;

	const int ticks_;
	int player_number_;
	int first_player_;
	bool loading_game_;
	int delay_;
	bool is_playing_;
	int current_turn_;
	const int xp_modifier_;
};


LEVEL_RESULT play_replay_level(const game_data& gameinfo, const config& terrain_config,
		const config* level, CVideo& video,
		game_state& state_of_game,
		const std::vector<config*>& story);

#endif
