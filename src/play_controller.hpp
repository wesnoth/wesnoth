/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAY_CONTROLLER_H_INCLUDED
#define PLAY_CONTROLLER_H_INCLUDED

#include "global.hpp"

#include "game_events.hpp"
#include "gamestatus.hpp"
#include "generic_event.hpp"
#include "halo.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "soundsource.hpp"
#include "statistics.hpp"
#include "tooltips.hpp"

#include <vector>

class game_display;
class team;

class play_controller : public hotkey::command_executor, public events::handler, public events::observer
{
public:
	play_controller(const config& level, const game_data& gameinfo, game_state& state_of_game,
		int ticks, int num_turns, const config& game_config, CVideo& video, bool skip_replay, bool is_replay);
	~play_controller();

	virtual void play_slice();

	//event handler, overriden from observer
	//there is nothing to handle in this class actually but that might change in the future
	virtual void handle_generic_event(const std::string& /*name*/) {}

	//event handlers, overriden from command_executor
	virtual void objectives();
	virtual void show_statistics();
	virtual void unit_list();
	virtual void status_table();
	virtual void save_game();
	virtual void save_map();
	virtual void load_game();
	virtual void preferences();
	virtual void show_chat_log();
	virtual void show_help();
	virtual void cycle_units();
	virtual void cycle_back_units();
	virtual void undo();
	virtual void redo();
	virtual void show_enemy_moves(bool ignore_units);
	virtual void goto_leader();
	virtual void unit_description();
	virtual void toggle_grid();
	virtual void search();

	virtual void play_side(const unsigned int team_num, bool save) = 0;

	const int get_ticks();

protected:
	void handle_event(const SDL_Event& event);
	virtual std::string get_action_image(hotkey::HOTKEY_COMMAND, int index) const;
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command) const;
	virtual bool can_execute_command(hotkey::HOTKEY_COMMAND command, int index=-1) const;
	virtual bool execute_command(hotkey::HOTKEY_COMMAND command, int index=-1);
	void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu);
	bool in_context_menu(hotkey::HOTKEY_COMMAND command) const;

	virtual void init(CVideo& video, bool is_replay);
	void init_managers();
	void fire_prestart(bool execute);
	void fire_start(bool execute);
	virtual void init_gui();
	virtual void init_side(const unsigned int team_index, bool is_replay = false);
	void place_sides_in_preferred_locations(gamemap& map, const config::child_list& sides);
	virtual void finish_side_turn();
	void finish_turn();
	bool clear_shroud();
	bool enemies_visible() const;
	void enter_textbox();
	
	team& current_team();
	const team& current_team() const;

	int find_human_team_before(const size_t team) const;

	//managers
	const verification_manager verify_manager_;
	teams_manager team_manager_;
	preferences::display_manager* prefs_disp_manager_;
	tooltips::manager* tooltips_manager_;
	game_events::manager* events_manager_;
	halo::manager* halo_manager_;
	font::floating_label_context labels_manager_;
	help::help_manager help_manager_;
	events::mouse_handler mouse_handler_;
	events::menu_handler menu_handler_;
	soundsource::manager* soundsources_manager_;

	//other objects
	game_display* gui_;
	const set_random_generator generator_setter;
	const statistics::scenario_context statistics_context_;
	const game_data& gameinfo_;
	const config& level_;
	const config& game_config_;
	std::vector<team> teams_;
	game_state& gamestate_;
	gamestatus status_;
	gamemap map_;
	unit_map units_;
	undo_list undo_stack_;
	undo_list redo_stack_;

	const int ticks_;
	const unit_type::experience_accelerator xp_mod_;
	//if a team is specified whose turn it is, it means we're loading a game
	//instead of starting a fresh one
	const bool loading_game_;

	CKey key_;
	int first_human_team_;
	unsigned int player_number_;
	unsigned int first_player_;
	unsigned int start_turn_;
	bool is_host_;
	bool skip_replay_;
	bool browse_;
	bool linger_;
	bool scrolling_;
	bool first_turn_;

private:
	// Expand AUTOSAVES in the menu items, setting the real savenames.
	void expand_autosaves(std::vector<std::string>& items);
	std::vector<std::string> savenames_;

	void expand_wml_commands(std::vector<std::string>& items);
	std::vector<wml_menu_item *> wml_commands_;
#define MAX_WML_COMMANDS 7

};


#endif
