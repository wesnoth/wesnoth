/*
   Copyright (C) 2006 - 2014 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAY_CONTROLLER_H_INCLUDED
#define PLAY_CONTROLLER_H_INCLUDED

#include "controller_base.hpp"
#include "game_end_exceptions.hpp"
#include "game_state.hpp"
#include "help.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "persist_manager.hpp"
#include "statistics.hpp"
#include "tod_manager.hpp"
#include "unit_types.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class game_display;
class saved_game;
class game_data;
class team;
class unit;
class wmi_pager;

namespace actions {
	class undo_list;
}

namespace game_events {
	class  manager;
	class  wml_menu_item;
} // namespace game_events

namespace preferences {
	struct display_manager;
}

namespace soundsource {
	class manager;
} // namespace soundsource

namespace pathfind {
	class manager;
}

namespace tooltips {
	struct manager;
} // namespace tooltips

namespace wb {
	class manager; // whiteboard manager
} // namespace wb

// Holds gamestate related objects
class game_state;

class play_controller : public controller_base, public events::observer, public savegame::savegame_config
{
public:
	play_controller(const config& level, saved_game& state_of_game,
		const int ticks, const config& game_config,
		CVideo& video, bool skip_replay, const std::string& local_client_id);
	virtual ~play_controller();

	//event handler, overridden from observer
	//there is nothing to handle in this class actually but that might change in the future
	virtual void handle_generic_event(const std::string& /*name*/) {}

	//event handlers, overridden from command_executor
	virtual void objectives();
	virtual void show_statistics();
	virtual void unit_list();
	virtual void left_mouse_click();
	virtual void move_action();
	virtual void select_and_action();
	virtual void select_hex();
	virtual void deselect_hex();
	virtual void right_mouse_click();
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
	virtual void terrain_description();
	virtual void toggle_ellipses();
	virtual void toggle_grid();
	virtual void search();
	virtual void toggle_accelerated_speed();

	void maybe_do_init_side(bool is_replay = false, bool only_visual = false);
	void do_init_side(bool is_replay = false, bool only_visual = false);

	virtual void force_end_turn() = 0;
	virtual void force_end_level(LEVEL_RESULT res) = 0;
	virtual void check_end_level() = 0;

	virtual void on_not_observer() = 0;
	/**
	 * Asks the user whether to continue on an OOS error.
	 * @throw end_level_exception If the user wants to abort.
	 */
	virtual void process_oos(const std::string& msg) const;

	void set_victory_when_enemies_defeated(bool e)
		{ victory_when_enemies_defeated_ = e; }
	void set_remove_from_carryover_on_defeat(bool e)
		{ remove_from_carryover_on_defeat_= e; }
	end_level_data& get_end_level_data() {
		return end_level_data_;
	}
	const end_level_data& get_end_level_data_const() const {
		return end_level_data_;
	}
	const std::vector<team>& get_teams_const() const {
		return gamestate_.board_.teams_;
	}
	const gamemap& get_map_const() const{
		return gamestate_.board_.map();
	}
	const tod_manager& get_tod_manager_const() const{
			return gamestate_.tod_manager_;
		}

	bool is_observer() const {
		return gamestate_.board_.is_observer();
	}
	virtual bool is_local_client_id(const std::string& controller_client_id)
	{ return this->gamestate_.is_local_client_id(controller_client_id); }

	/**
	 * Checks to see if a side has won, and throws an end_level_exception.
	 * Will also remove control of villages from sides with dead leaders.
	 */
	void check_victory();

	size_t turn() const {return gamestate_.tod_manager_.turn();}

	/** Returns the number of the side whose turn it is. Numbering starts at one. */
	int current_side() const { return player_number_; }

	config to_config() const;

	bool is_skipping_replay() const { return skip_replay_;}
	bool is_linger_mode() const { return linger_; }

	void do_autosave();

	void do_consolesave(const std::string& filename);

	events::mouse_handler& get_mouse_handler_base();
	events::menu_handler& get_menu_handler() { return menu_handler_; }

	static const std::string wml_menu_hotkey_prefix;
protected:
	void slice_before_scroll();

	game_display& get_display();
	bool have_keyboard_focus();
	void process_focus_keydown_event(const SDL_Event& event);
	void process_keydown_event(const SDL_Event& event);
	void process_keyup_event(const SDL_Event& event);

	virtual std::string get_action_image(hotkey::HOTKEY_COMMAND, int index) const;
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command, int index) const;
	/** Check if a command can be executed. */
	virtual bool can_execute_command(const hotkey::hotkey_command& command, int index=-1) const;
	virtual bool execute_command(const hotkey::hotkey_command& command, int index=-1);
	void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& disp);

	/**
	 *  Determines whether the command should be in the context menu or not.
	 *  Independent of whether or not we can actually execute the command.
	 */
	bool in_context_menu(hotkey::HOTKEY_COMMAND command) const;

	void init_managers();
	///preload events cannot be synced
	void fire_preload();
	void fire_prestart();
	void fire_start(bool execute);
	virtual void init_gui();
	possible_end_play_signal init_side(bool is_replay = false);
	virtual void finish_side_turn();
	void finish_turn(); //this should not throw an end turn or end level exception
	bool enemies_visible() const;

	void enter_textbox();
	void tab();

	team& current_team();
	const team& current_team() const;

	/** Find a human team (ie one we own) starting backwards from current player. */
	int find_human_team_before_current_player() const;

	//gamestate
	game_state gamestate_;
	const config & level_;
	saved_game & saved_game_;

	//managers
	boost::scoped_ptr<preferences::display_manager> prefs_disp_manager_;
	boost::scoped_ptr<tooltips::manager> tooltips_manager_;
	boost::scoped_ptr<game_events::manager> events_manager_;
	font::floating_label_context labels_manager_;
	help::help_manager help_manager_;

	//more managers
	events::mouse_handler mouse_handler_;
	events::menu_handler menu_handler_;
	boost::scoped_ptr<soundsource::manager> soundsources_manager_;
	persist_manager persist_;

	//other objects
	boost::scoped_ptr<game_display> gui_;
	const statistics::scenario_context statistics_context_;
	/// undo_stack_ is never NULL. It is implemented as a pointer so that
	/// undo_list can be an incomplete type at this point (which reduces the
	/// number of files that depend on actions/undo.hpp).
	boost::scoped_ptr<actions::undo_list> undo_stack_;

	//whiteboard manager
	boost::shared_ptr<wb::manager> whiteboard_manager_;

	//if a team is specified whose turn it is, it means we're loading a game
	//instead of starting a fresh one. Gets reset to false after init_side
	bool loading_game_;

	int player_number_;
	int first_player_;
	unsigned int start_turn_;
	bool skip_replay_;
	bool linger_;
	bool it_is_a_new_turn_;
	bool init_side_done_;

	const std::string& select_victory_music() const;
	const std::string& select_defeat_music()  const;

	void set_victory_music_list(const std::string& list);
	void set_defeat_music_list(const std::string& list);

	/*
	 * Changes the UI for this client to the passed side index.
	 */
	void update_gui_to_player(const int team_index, const bool observe = false);

private:
	/// A smart pointer used when retrieving menu items.
	typedef boost::shared_ptr<const game_events::wml_menu_item> const_item_ptr;

	void init(CVideo &video);
	// Expand AUTOSAVES in the menu items, setting the real savenames.
	void expand_autosaves(std::vector<std::string>& items);

	std::vector<std::string> savenames_;

	void expand_wml_commands(std::vector<std::string>& items);
	std::vector<const_item_ptr> wml_commands_;
	boost::scoped_ptr<wmi_pager> wml_command_pager_;
	int last_context_menu_x_;
	int last_context_menu_y_;

	bool victory_when_enemies_defeated_;
	bool remove_from_carryover_on_defeat_;
	end_level_data end_level_data_;
	std::vector<std::string> victory_music_;
	std::vector<std::string> defeat_music_;

	hotkey::scope_changer scope_;

};


#endif
