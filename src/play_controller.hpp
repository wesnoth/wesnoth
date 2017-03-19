/*
   Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
#include "floating_label.hpp"
#include "game_end_exceptions.hpp"
#include "help/help.hpp"
#include "hotkey/command_executor.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "persist_manager.hpp"
#include "terrain/type_data.hpp"
#include "tod_manager.hpp"
#include "game_state.hpp"

#include <set>

class game_display;
class game_data;
class team;
class unit;
class replay;
class saved_game;
struct mp_game_settings;
class game_classification;
struct unit_experience_accelerator;

namespace actions {
	class undo_list;
}

namespace game_events {
	class manager;
	class wml_event_pump;
	class wml_menu_item;
} // namespace game_events

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

class play_controller : public controller_base, public events::observer, public quit_confirmation
{
public:
	play_controller(const config& level, saved_game& state_of_game,
		const config& game_config,
		const ter_data_cache& tdata,
		CVideo& video, bool skip_replay);
	virtual ~play_controller();

	//event handler, overridden from observer
	//there is nothing to handle in this class actually but that might change in the future
	virtual void handle_generic_event(const std::string& /*name*/) override {}

	bool can_undo() const;
	bool can_redo() const;

	void undo();
	void redo();

	void load_game();

	void save_game();
	void save_game_auto(const std::string& filename);
	void save_replay();
	void save_replay_auto(const std::string& filename);
	void save_map();

	void init_side_begin();

	/**
	 * Called by turn_info::process_network_data() or init_side() to call do_init_side() if necessary.
	 */
	void maybe_do_init_side();

	/**
	 * Called by replay handler or init_side() to do actual work for turn change.
	 */
	void do_init_side();

	void init_side_end();

	virtual void force_end_turn() = 0;
	virtual void check_objectives() = 0;

	virtual void on_not_observer() = 0;

	/**
	 * Asks the user whether to continue on an OOS error.
	 *
	 * @throw quit_game_exception If the user wants to abort.
	 */
	virtual void process_oos(const std::string& msg) const;

	void set_end_level_data(const end_level_data& data) {
		gamestate().end_level_data_ = data;
	}
	void reset_end_level_data() {
		gamestate().end_level_data_ = boost::none;
	}
	bool is_regular_game_end() const {
		return gamestate().end_level_data_.get_ptr() != nullptr;
	}
	const end_level_data& get_end_level_data_const() const {
		return *gamestate().end_level_data_;
	}
	const std::vector<team>& get_teams_const() const {
		return gamestate().board_.teams_;
	}

	const unit_map& get_units_const() const {
		return gamestate().board_.units();
	}

	const gamemap& get_map_const() const{
		return gamestate().board_.map();
	}
	const tod_manager& get_tod_manager_const() const{
			return gamestate().tod_manager_;
		}

	bool is_observer() const {
		return gamestate().board_.is_observer();
	}

	game_state& gamestate() {
		return *gamestate_;
	}
	const game_state& gamestate() const {
		return *gamestate_;
	}

	/**
	 * Checks to see if a side has won.
	 *
	 * This will also remove control of villages from sides with dead leaders.
	 */
	void check_victory();

	size_t turn() const {return gamestate().tod_manager_.turn();}

	/**
	 * Returns the number of the side whose turn it is.
	 *
	 * Numbering starts at one.
	 */
	int current_side() const { return gamestate_->player_number_; }

	/**
	 * Builds the snapshot config from members and their respective configs.
	 */
	config to_config() const;

	bool is_skipping_replay() const { return skip_replay_; }
	void toggle_skipping_replay() { skip_replay_ = !skip_replay_; }
	bool is_linger_mode() const { return linger_; }
	void do_autosave();

	void do_consolesave(const std::string& filename);

	events::mouse_handler& get_mouse_handler_base() override;
	events::menu_handler& get_menu_handler() { return menu_handler_; }

	std::shared_ptr<wb::manager> get_whiteboard() const;
	const mp_game_settings& get_mp_settings();
	game_classification& get_classification();
	int get_server_request_number() const { return gamestate().server_request_number_; }
	void increase_server_request_number() { ++gamestate().server_request_number_; }

	game_events::wml_event_pump& pump();

	int get_ticks() const;

	virtual soundsource::manager* get_soundsource_man() override;
	virtual plugins_context* get_plugins_context() override;
	hotkey::command_executor* get_hotkey_command_executor() override;

	actions::undo_list& get_undo_stack() { return undo_stack(); }

	bool is_browsing() const override;
	bool is_lingering() const { return linger_; }

	class hotkey_handler;

	virtual bool is_replay() { return false; }

	t_string get_scenario_name() const
	{
		return level_["name"].t_str();
	}

	bool get_disallow_recall() const
	{
		return level_["disallow_recall"].to_bool();
	}

	std::string theme() const
	{
		return level_["theme"].str();
	}

	virtual bool should_return_to_play_side() const
	{
		return is_regular_game_end();
	}

	void maybe_throw_return_to_play_side()
	{
		if(should_return_to_play_side() && !linger_ ) {
			throw return_to_play_side_exception();
		}
	}

	virtual void play_side_impl() {}

	void play_side();

	team& current_team();
	const team& current_team() const;

	bool can_use_synced_wml_menu() const;
	std::set<std::string> all_players() const;
	int ticks() const { return ticks_; }
	game_display& get_display() override;

	void update_savegame_snapshot() const;
	/**
	 * Changes the UI for this client to the passed side index.
	 */
	void update_gui_to_player(const int team_index, const bool observe = false);

	virtual bool is_networked_mp() const { return false; }
	virtual void send_to_wesnothd(const config&, const std::string& = "unknown") const { }
	virtual bool recieve_from_wesnothd(config&) const { return false; }
	void show_objectives() const;
protected:
	struct scoped_savegame_snapshot
	{
		scoped_savegame_snapshot(const play_controller& controller);
		~scoped_savegame_snapshot();
		const play_controller& controller_;
	};
	friend struct scoped_savegame_snapshot;
	void play_slice_catch();
	bool have_keyboard_focus() override;
	void process_focus_keydown_event(const SDL_Event& event) override;
	void process_keydown_event(const SDL_Event& event) override;
	void process_keyup_event(const SDL_Event& event) override;

	void init_managers();
	///preload events cannot be synced
	void fire_preload();
	void fire_prestart();
	void fire_start();
	void start_game();
	virtual void init_gui();
	void finish_side_turn();
	void finish_turn(); //this should not throw an end turn or end level exception
	bool enemies_visible() const;

	void enter_textbox();
	void tab();


	bool is_team_visible(int team_num, bool observer) const;
	/// returns 0 if no such team was found.
	int find_last_visible_team() const;

private:
	const int ticks_;

protected:
	//gamestate
	const ter_data_cache& tdata_;
	std::unique_ptr<game_state> gamestate_;
	config level_;
	saved_game& saved_game_;

	//managers
	std::unique_ptr<tooltips::manager> tooltips_manager_;

	//whiteboard manager
	std::shared_ptr<wb::manager> whiteboard_manager_;

	//plugins context
	std::unique_ptr<plugins_context> plugins_context_;

	//more managers
	font::floating_label_context labels_manager_;
	help::help_manager help_manager_;
	events::mouse_handler mouse_handler_;
	events::menu_handler menu_handler_;
	std::unique_ptr<hotkey_handler> hotkey_handler_;
	std::unique_ptr<soundsource::manager> soundsources_manager_;
	persist_manager persist_;

	//other objects
	std::unique_ptr<game_display> gui_;
	const std::unique_ptr<unit_experience_accelerator> xp_mod_;
	const std::unique_ptr<const statistics::scenario_context> statistics_context_;
	actions::undo_list& undo_stack() { return *gamestate().undo_stack_; };
	const actions::undo_list& undo_stack() const { return *gamestate().undo_stack_; };
	std::unique_ptr<replay> replay_;

	bool skip_replay_;
	bool linger_;
	/**
	 * Whether we did init sides in this session
	 * (false = we did init sides before we reloaded the game).
	 */
	bool init_side_done_now_;
	const std::string& select_victory_music() const;
	const std::string& select_defeat_music()  const;
	void set_victory_music_list(const std::string& list);
	void set_defeat_music_list(const std::string& list);

	void reset_gamestate(const config& level, int replay_pos);

private:

	void init(CVideo& video, const config& level);

	bool victory_when_enemies_defeated_;
	bool remove_from_carryover_on_defeat_;
	std::vector<std::string> victory_music_;
	std::vector<std::string> defeat_music_;

	hotkey::scope_changer scope_;

protected:
	mutable bool ignore_replay_errors_;
	bool player_type_changed_;
	virtual void sync_end_turn() {};
	virtual void check_time_over();
	virtual void update_viewing_player() = 0;
	void play_turn();
};


#endif
