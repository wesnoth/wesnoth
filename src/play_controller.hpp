/*
   Copyright (C) 2006 - 2015 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
#include "help/help.hpp"
#include "hotkey/command_executor.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "persist_manager.hpp"
#include "terrain_type_data.hpp"
#include "tod_manager.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class game_display;
class game_data;
class team;
class unit;
class wmi_pager;
class replay;
class saved_game;
struct mp_game_settings;
class game_classification;
struct unit_experience_accelerator;

namespace actions {
	class undo_list;
}

namespace game_events {
	class t_pump;
	class manager;
	class wml_menu_item;
} // namespace game_events

namespace preferences {
	struct display_manager;
}

namespace soundsource {
	class manager;
} // namespace soundsource

namespace statistics {
	struct scenario_context;
} // namespace statistics

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
		const tdata_cache & tdata,
		CVideo& video, bool skip_replay);
	virtual ~play_controller();

	//event handler, overridden from observer
	//there is nothing to handle in this class actually but that might change in the future
	virtual void handle_generic_event(const std::string& /*name*/) {}

	bool can_undo() const;
	bool can_redo() const;

	void undo();
	void redo();

	void load_game();

	void save_game();
	void save_game_auto(const std::string & filename);
	void save_replay();
	void save_replay_auto(const std::string & filename);
	void save_map();

	void init_side_begin(bool is_replay);
	void maybe_do_init_side();
	void do_init_side();
	void init_side_end();

	virtual void force_end_turn() = 0;
	virtual void check_objectives() = 0;

	virtual void on_not_observer() = 0;
	/**
	 * Asks the user whether to continue on an OOS error.
	 * @throw quit_game_exception If the user wants to abort.
	 */
	virtual void process_oos(const std::string& msg) const;

	void set_victory_when_enemies_defeated(bool e)
		{ victory_when_enemies_defeated_ = e; }
	void set_remove_from_carryover_on_defeat(bool e)
		{ remove_from_carryover_on_defeat_= e; }
	
	void set_end_level_data(const end_level_data& data) {
		end_level_data_ = data;
	}
	void reset_end_level_data() {
		end_level_data_ = boost::none_t();
	}
	bool is_regular_game_end() const { 
		return end_level_data_.get_ptr() != NULL;
	}
	const end_level_data& get_end_level_data_const() const {
		return *end_level_data_;
	}
	const std::vector<team>& get_teams_const() const {
		return gamestate_.board_.teams_;
	}

	const unit_map & get_units_const() const {
		return gamestate_.board_.units();
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

	game_state & gamestate() {
		return gamestate_;
	}

	/**
	 * Checks to see if a side has won.
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

	boost::shared_ptr<wb::manager> get_whiteboard();
	const mp_game_settings& get_mp_settings();
	const game_classification & get_classification();
	int get_server_request_number() const { return server_request_number_; }
	void increase_server_request_number() { ++server_request_number_; }

	game_events::t_pump & pump();

	int get_ticks();

	virtual soundsource::manager * get_soundsource_man();
	virtual plugins_context * get_plugins_context();
	hotkey::command_executor * get_hotkey_command_executor();

	actions::undo_list & get_undo_stack() { return *undo_stack_; }

	bool is_browsing() const OVERRIDE;
	bool is_lingering() { return linger_; }

	class hotkey_handler;
	virtual bool is_replay() { return false; }
	t_string get_scenario_name()
	{ return level_["name"].t_str(); }
	bool get_disallow_recall()
	{ return level_["disallow_recall"].to_bool(); }
	void update_savegame_snapshot() const;
	virtual bool should_return_to_play_side()
	{ return is_regular_game_end(); }
	void maybe_throw_return_to_play_side()
	{ if(should_return_to_play_side()) { throw return_to_play_side_exception(); } }

protected:
	void play_slice_catch();
	game_display& get_display();
	bool have_keyboard_focus();
	void process_focus_keydown_event(const SDL_Event& event);
	void process_keydown_event(const SDL_Event& event);
	void process_keyup_event(const SDL_Event& event);

	void init_managers();
	///preload events cannot be synced
	void fire_preload();
	void fire_prestart();
	void fire_start();
	virtual void init_gui();
	virtual void finish_side_turn();
	void finish_turn(); //this should not throw an end turn or end level exception
	bool enemies_visible() const;

	void enter_textbox();
	void tab();

	team& current_team();
	const team& current_team() const;

	bool is_team_visible(int team_num, bool observer) const;
	/// returns 0 if no such team was found.
	int find_last_visible_team() const;

	//gamestate
	game_state gamestate_;
	const config & level_;
	saved_game & saved_game_;

	//managers
	boost::scoped_ptr<preferences::display_manager> prefs_disp_manager_;
	boost::scoped_ptr<tooltips::manager> tooltips_manager_;

	//whiteboard manager
	boost::shared_ptr<wb::manager> whiteboard_manager_;

	//plugins context
	boost::scoped_ptr<plugins_context> plugins_context_;

	//more managers
	font::floating_label_context labels_manager_;
	help::help_manager help_manager_;
	events::mouse_handler mouse_handler_;
	events::menu_handler menu_handler_;
	boost::scoped_ptr<hotkey_handler> hotkey_handler_;
	boost::scoped_ptr<soundsource::manager> soundsources_manager_;
	persist_manager persist_;

	//other objects
	boost::scoped_ptr<game_display> gui_;
	boost::scoped_ptr<unit_experience_accelerator> xp_mod_;
	boost::scoped_ptr<const statistics::scenario_context> statistics_context_;
	/// undo_stack_ is never NULL. It is implemented as a pointer so that
	/// undo_list can be an incomplete type at this point (which reduces the
	/// number of files that depend on actions/undo.hpp).
	boost::scoped_ptr<actions::undo_list> undo_stack_;
	boost::scoped_ptr<replay> replay_;

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

	const int ticks_;

	const std::string& select_victory_music() const;
	const std::string& select_defeat_music()  const;
	void set_victory_music_list(const std::string& list);
	void set_defeat_music_list(const std::string& list);

	/*
	 * Changes the UI for this client to the passed side index.
	 */
	void update_gui_to_player(const int team_index, const bool observe = false);

private:

	void init(CVideo &video);

	bool victory_when_enemies_defeated_;
	bool remove_from_carryover_on_defeat_;
	typedef boost::optional<end_level_data> t_possible_end_level_data;
	t_possible_end_level_data end_level_data_;
	std::vector<std::string> victory_music_;
	std::vector<std::string> defeat_music_;

	hotkey::scope_changer scope_;
	// used to sync with the mpserver, not persistent in savefiles.
	int server_request_number_;
};


#endif
