/* $Id$ */
/*
   Copyright (C) 2006 - 2010 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "actions.hpp"
#include "controller_base.hpp"
#include "game_end_exceptions.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "map.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "statistics.hpp"
#include "tod_manager.hpp"
#include "savegame_config.hpp"

#include <vector>

#include <boost/scoped_ptr.hpp>

class game_display;
class game_state;
class team;
struct wml_menu_item;

namespace game_events {
	struct manager;
} // namespace game_events

namespace halo {
	struct manager;
} // namespace halo

namespace preferences {
	struct display_manager;
}

namespace soundsource {
	class manager;
} // namespace soundsource


namespace tooltips {
	struct manager;
} // namespace tooltips

namespace wb {
	class manager; // whiteboard manager
} // namespace wb

class play_controller : public controller_base, public events::observer, public savegame::savegame_config
{
public:
	play_controller(const config& level, game_state& state_of_game,
		int ticks, int num_turns, const config& game_config, CVideo& video, bool skip_replay);
	virtual ~play_controller();

	//event handler, overriden from observer
	//there is nothing to handle in this class actually but that might change in the future
	virtual void handle_generic_event(const std::string& /*name*/) {}

	//event handlers, overriden from command_executor
	virtual void objectives();
	virtual void show_statistics();
	virtual void unit_list();
	virtual void status_table();
	virtual void save_game();
	virtual void save_replay();
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
	virtual void toggle_ellipses();
	virtual void toggle_grid();
	virtual void search();

	// Whiteboard hotkeys
	virtual void whiteboard_execute_action();
	virtual void whiteboard_delete_action();
	virtual void whiteboard_bump_up_action();
	virtual void whiteboard_bump_down_action();

	virtual void do_init_side(const unsigned int team_index);
	virtual void play_side(const unsigned int team_num, bool save) = 0;

	virtual void force_end_turn() = 0;
	virtual void force_end_level(LEVEL_RESULT res) = 0;
	virtual void check_end_level() = 0;
	/**
	 * Asks the user whether to continue on an OOS error.
	 * @throw end_level_exception If the user wants to abort.
	 */
	virtual void process_oos(const std::string& msg) const;

	void set_victory_when_enemies_defeated(bool e)
	{ victory_when_enemies_defeated_ = e; }
	end_level_data &get_end_level_data()
	{ return end_level_data_; }

	/**
	 * Checks to see if a side has won, and throws an end_level_exception.
	 * Will also remove control of villages from sides with dead leaders.
	 */
	void check_victory();

	//turn functions
	size_t turn() const {return tod_manager_.turn();}
	int number_of_turns() const {return tod_manager_.number_of_turns();}
	void modify_turns(const std::string& mod)  {tod_manager_.modify_turns(mod);}
	void add_turns(int num) {tod_manager_.add_turns(num);}

	/** Dynamically change the current turn number. */
	void set_turn(unsigned int num) {tod_manager_.set_turn(num);}

	/**
	 * Function to move to the next turn.
	 *
	 * @returns                   True if time has not expired.
	 */
	bool next_turn() {return tod_manager_.next_turn();}

	int current_side() const { return player_number_; }

	void add_time_area(const config& cfg) {tod_manager_.add_time_area(cfg);}
	void add_time_area(const std::string& id, const std::set<map_location>& locs,
				const config& time_cfg) {tod_manager_.add_time_area(id, locs, time_cfg);}
	void remove_time_area(const std::string& id) {tod_manager_.remove_time_area(id);}

	config to_config() const;

	bool is_skipping_replay() const { return skip_replay_;};
protected:
	void slice_before_scroll();
	void slice_end();

	events::mouse_handler& get_mouse_handler_base();
	game_display& get_display();
	bool have_keyboard_focus();
	void process_keydown_event(const SDL_Event& event);
	void process_keyup_event(const SDL_Event& event);
	void post_mouse_press(const SDL_Event& event);

	virtual std::string get_action_image(hotkey::HOTKEY_COMMAND, int index) const;
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command, int index) const;
	/** Check if a command can be executed. */
	virtual bool can_execute_command(hotkey::HOTKEY_COMMAND command, int index=-1) const;
	virtual bool execute_command(hotkey::HOTKEY_COMMAND command, int index=-1);
	void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu);

	/**
	 *  Determines whether the command should be in the context menu or not.
	 *  Independant of whether or not we can actually execute the command.
	 */
	bool in_context_menu(hotkey::HOTKEY_COMMAND command) const;

	void init_managers();
	void fire_prestart(bool execute);
	void fire_start(bool execute);
	virtual void init_gui();
	virtual void init_side(const unsigned int team_index, bool is_replay = false);
	void place_sides_in_preferred_locations();
	virtual void finish_side_turn();
	void finish_turn();
	bool clear_shroud();
	bool enemies_visible() const;
	void enter_textbox();
	std::string get_unique_saveid(const config& cfg, std::set<std::string>& seen_save_ids);

	team& current_team();
	const team& current_team() const;

	/** Find a human team (ie one we own) starting backwards from 'team_num'. */
	int find_human_team_before(const size_t team) const;

	//managers
	boost::scoped_ptr<preferences::display_manager> prefs_disp_manager_;
	boost::scoped_ptr<tooltips::manager> tooltips_manager_;
	boost::scoped_ptr<game_events::manager> events_manager_;
	boost::scoped_ptr<halo::manager> halo_manager_;
	font::floating_label_context labels_manager_;
	help::help_manager help_manager_;
	events::mouse_handler mouse_handler_;
	events::menu_handler menu_handler_;
	boost::scoped_ptr<soundsource::manager> soundsources_manager_;
	tod_manager tod_manager_;

	//other objects
	boost::scoped_ptr<game_display> gui_;
	const statistics::scenario_context statistics_context_;
	const config& level_;
	std::vector<team> teams_;
	game_state& gamestate_;
	gamemap map_;
	unit_map units_;
	undo_list undo_stack_;
	undo_list redo_stack_;

	//whiteboard manager
	boost::scoped_ptr<wb::manager> whiteboard_manager_;

	const unit_type::experience_accelerator xp_mod_;
	//if a team is specified whose turn it is, it means we're loading a game
	//instead of starting a fresh one. Gets reset to false after init_side
	bool loading_game_;

	int first_human_team_;
	int player_number_;
	int first_player_;
	unsigned int start_turn_;
	bool is_host_;
	bool skip_replay_;
	bool linger_;
	unsigned int previous_turn_;

	const std::string& select_victory_music() const;
	const std::string& select_defeat_music()  const;

	void set_victory_music_list(const std::string& list);
	void set_defeat_music_list(const std::string& list);

private:
	void init(CVideo &video);
	// Expand AUTOSAVES in the menu items, setting the real savenames.
	void expand_autosaves(std::vector<std::string>& items);
	std::vector<std::string> savenames_;

	void expand_wml_commands(std::vector<std::string>& items);
	std::vector<wml_menu_item *> wml_commands_;
	static const size_t MAX_WML_COMMANDS = 7;

	bool victory_when_enemies_defeated_;
	end_level_data end_level_data_;
	std::vector<std::string> victory_music_;
	std::vector<std::string> defeat_music_;
};


#endif
