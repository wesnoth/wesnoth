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

#include <deque>
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
	bool in_context_menu(hotkey::HOTKEY_COMMAND command) const;

	void save_game(const std::string& message);

	enum PROCESS_DATA_RESULT { PROCESS_CONTINUE, PROCESS_RESTART_TURN, PROCESS_END_TURN };

	//function which will process incoming network data, and act on it. If there is
	//more data than a single turn's worth, excess data will be placed into 'backlog'.
	//No more than one turn's worth of data will be placed into a single backlog item,
	//so it is safe to assume that backlog won't be touched if cfg is a member of a previous
	//backlog.
	//data will be forwarded to all peers other than 'from', unless 'from' is null, in
	//which case data will not be forwarded
	PROCESS_DATA_RESULT process_network_data(const config& cfg,network::connection from,std::deque<config>& backlog);

private:
	//convenience functions
	team& current_team() { return teams_[team_num_-1]; }
	const team& current_team() const { return teams_[team_num_-1]; }

	void write_game_snapshot(config& cfg) const;
	
	//overridden from command_executor
	virtual void cycle_units();
	virtual void end_turn();
	virtual void goto_leader();
	virtual void end_unit_turn();
	virtual void undo();
	virtual void redo();
	virtual void terrain_table();
	virtual void attack_resistance();
	virtual void unit_description();
	virtual void rename_unit();
	virtual void save_game();
	virtual void toggle_grid();
	virtual void status_table();
	virtual void recruit();
	virtual void repeat_recruit();
	virtual void recall();
	virtual void speak();
	virtual void create_unit();
	virtual void preferences();
	virtual void objectives();
	virtual void unit_list();
	virtual void show_statistics();
	virtual void label_terrain();
	virtual void show_enemy_moves(bool ignore_units);
	virtual void toggle_shroud_updates();
	virtual void update_shroud_now();
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command) const;
	
	void do_recruit(const std::string& name);

	void handle_event(const SDL_Event& event);
	void mouse_motion(const SDL_MouseMotionEvent& event);
	void mouse_press(const SDL_MouseButtonEvent& event);

	void left_click(const SDL_MouseButtonEvent& event);
	void show_menu(const std::vector<std::string>& items, int xloc, int yloc, bool context_menu);

	void show_attack_options(unit_map::const_iterator u);

	bool clear_shroud();
	void clear_undo_stack();

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
	paths current_paths_, all_paths_;
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
