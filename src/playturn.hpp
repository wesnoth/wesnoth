/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef PLAYTURN_HPP_INCLUDED
#define PLAYTURN_HPP_INCLUDED

#include "actions.hpp"
#include "ai.hpp"
#include "config.hpp"
#include "dialogs.hpp"
#include "display.hpp"
#include "events.hpp"
#include "game_config.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "hotkeys.hpp"
#include "key.hpp"
#include "pathfind.hpp"
#include "show_dialog.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "unit_types.hpp"
#include "unit.hpp"
#include "video.hpp"

#include <map>
#include <vector>

struct paths_wiper
{
	paths_wiper(display& gui) : gui_(gui)
	{}

	~paths_wiper() { gui_.set_paths(NULL); }

private:
	display& gui_;
};

struct command_disabler : private hotkey::basic_handler
{
	command_disabler(display* disp);
	~command_disabler();
};

class turn_info : public hotkey::command_executor, public events::handler,
                  private paths_wiper
{
public:
	turn_info(game_data& gameinfo, game_state& state_of_game,
	          gamestatus& status, const config& terrain_config, config* level,
	          CKey& key, display& gui, gamemap& map,
	          std::vector<team>& teams, int team_num,
	          unit_map& units, bool browse_only);

	void turn_slice();

	bool turn_over() const;

	int send_data(int first_command);

	undo_list& undos() { return undo_stack_; }

	bool can_execute_command(hotkey::HOTKEY_COMMAND command) const;

	void save_game(const std::string& message);

private:

	void write_game_snapshot(config& cfg) const;

	void cycle_units();
	void end_turn();
	void goto_leader();
	void end_unit_turn();
	void undo();
	void redo();
	void terrain_table();
	void attack_resistance();
	void unit_description();
	void rename_unit();
	void save_game();
	void toggle_grid();
	void status_table();
	void recruit();
	void repeat_recruit();
	void recall();
	void speak();
	void create_unit();
	void preferences();
	void objectives();
	void unit_list();
	void show_statistics();
	void label_terrain();

	void do_recruit(const std::string& name);

	void handle_event(const SDL_Event& event);
	void mouse_motion(const SDL_MouseMotionEvent& event);
	void mouse_press(const SDL_MouseButtonEvent& event);

	void left_click(const SDL_MouseButtonEvent& event);
	void show_menu(const std::vector<std::string>& items, int xloc, int yloc);

	void show_attack_options(unit_map::const_iterator u);

	unit_map::iterator current_unit();
	unit_map::const_iterator current_unit() const;

	std::vector<std::string> create_unit_table(const statistics::stats::str_int_map& m);

	game_data& gameinfo_;
	game_state& state_of_game_;
	gamestatus& status_;
	const config& terrain_config_;
	config* level_;
	CKey key_;
	display& gui_;
	gamemap& map_;
	std::vector<team>& teams_;
	int team_num_;
	unit_map& units_;
	bool browse_;

	bool left_button_, right_button_, middle_button_;
	bool minimap_scrolling_;
	gamemap::location next_unit_;
	paths current_paths_;
	paths::route current_route_;
	bool enemy_paths_;
	gamemap::location last_hex_;
	gamemap::location selected_hex_;
	undo_list undo_stack_;
	undo_list redo_stack_;
	int path_turns_;

	bool end_turn_;

	std::string last_recruit_;
};

void play_turn(game_data& gameinfo, game_state& state_of_game,
               gamestatus& status, const config& terrain_config, config* level,
			   CVideo& video, CKey& key, display& gui,
               game_events::manager& events_manager, gamemap& map,
			   std::vector<team>& teams, int team_num, unit_map& units);                

#endif
