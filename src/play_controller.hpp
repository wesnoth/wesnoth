/*
	Copyright (C) 2006 - 2024
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "controller_base.hpp"
#include "game_end_exceptions.hpp"
#include "help/help.hpp"
#include "hotkey/command_executor.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "persist_manager.hpp"
#include "tod_manager.hpp"
#include "game_state.hpp"
#include "utils/optimer.hpp"
#include "utils/optional_fwd.hpp"

#include <set>

class team;
class replay;
class replay_controller;
class saved_game;
struct mp_game_settings;
class game_classification;
struct unit_experience_accelerator;

namespace actions {
	class undo_list;
}

namespace font {
	struct floating_label_context;
}

namespace game_events {
	class wml_event_pump;
} // namespace game_events

class statistics_t;

namespace wb {
	class manager; // whiteboard manager
} // namespace wb

// Holds gamestate related objects

class play_controller : public controller_base, public events::observer, public quit_confirmation
{
public:
	play_controller(const config& level,
			saved_game& state_of_game);
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
	replay& get_replay();

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
	virtual void require_end_turn() = 0;
	virtual void check_objectives() = 0;

	virtual void on_not_observer() = 0;

	/**
	 * Asks the user whether to continue on an OOS error.
	 *
	 * @throw quit_game_exception If the user wants to abort.
	 */
	virtual void process_oos(const std::string& msg) const;

	bool reveal_map_default() const;

	void set_end_level_data(const end_level_data& data)
	{
		gamestate().end_level_data_ = data;
	}

	void reset_end_level_data()
	{
		gamestate().end_level_data_.reset();
	}

	bool is_regular_game_end() const
	{
		return gamestate().end_level_data_.has_value();
	}

	const end_level_data& get_end_level_data() const
	{
		return *gamestate().end_level_data_;
	}

	std::vector<team>& get_teams()
	{
		return gamestate().board_.teams();
	}

	const std::vector<team>& get_teams() const
	{
		return gamestate().board_.teams();
	}

	const unit_map& get_units() const
	{
		return gamestate().board_.units();
	}

	unit_map& get_units()
	{
		return gamestate().board_.units();
	}

	const gamemap& get_map() const
	{
		return gamestate().board_.map();
	}

	const tod_manager& get_tod_manager() const
	{
		return gamestate().tod_manager_;
	}

	bool is_observer() const
	{
		return gamestate().board_.is_observer();
	}

	bool do_healing() const
	{
		return gamestate().do_healing_;
	}

	void set_do_healing(bool do_healing)
	{
		gamestate().do_healing_ = do_healing;
	}

	game_state& gamestate()
	{
		return *gamestate_;
	}

	const game_state& gamestate() const
	{
		return *gamestate_;
	}

	/**
	 * Checks to see if a side has won.
	 *
	 * This will also remove control of villages from sides with dead leaders.
	 */
	void check_victory();

	std::size_t turn() const {return gamestate().tod_manager_.turn();}

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
	bool is_skipping_actions() const;
	void toggle_skipping_replay();
	void do_autosave();

	bool is_skipping_story() const { return skip_story_; }

	void do_consolesave(const std::string& filename);

	events::mouse_handler& get_mouse_handler_base() override;
	events::menu_handler& get_menu_handler() { return menu_handler_; }

	std::shared_ptr<wb::manager> get_whiteboard() const;
	const mp_game_settings& get_mp_settings();
	game_classification& get_classification();
	int get_server_request_number() const { return gamestate().server_request_number_; }
	void increase_server_request_number() { ++gamestate().server_request_number_; }

	game_events::wml_event_pump& pump();

	virtual soundsource::manager* get_soundsource_man() override;
	virtual plugins_context* get_plugins_context() override;
	hotkey::command_executor* get_hotkey_command_executor() override;

	actions::undo_list& get_undo_stack() { return undo_stack(); }

	bool is_browsing() const override;

	class hotkey_handler;

	virtual replay_controller * get_replay_controller() const { return nullptr; }
	bool is_replay() const { return get_replay_controller() != nullptr; }

	replay& recorder() const { return *replay_; }

	t_string get_scenario_name() const
	{
		return level_["name"].t_str();
	}

	bool get_disallow_recall() const
	{
		return level_["disallow_recall"].to_bool();
	}

	std::string get_loaded_resources() const
	{
		return level_["loaded_resources"].str();
	}

	std::string theme() const
	{
		return gamestate_->get_game_data()->get_theme();
	}

	virtual bool should_return_to_play_side() const
	{
		return is_regular_game_end();
	}

	void maybe_throw_return_to_play_side() const;

	team& current_team();
	const team& current_team() const;

	bool can_use_synced_wml_menu() const;
	std::set<std::string> all_players() const;
	const auto& timer() const { return timer_; }
	game_display& get_display() override;

	void update_savegame_snapshot() const;
	/**
	 * Changes the UI for this client to the passed side index.
	 */
	void update_gui_to_player(const int team_index, const bool observe = false);

	/// Sends replay [command]s to the server
	virtual void send_actions() { }
	/// Reads and executes replay [command]s from the server
	virtual void receive_actions() { }

	virtual bool is_networked_mp() const { return false; }
	virtual void send_to_wesnothd(const config&, const std::string& = "unknown") const { }
	virtual bool receive_from_wesnothd(config&) const { return false; }
	/** Reevaluate [show_if] conditions and build a new objectives string. */
	void refresh_objectives() const;
	void show_objectives() const;

	struct scoped_savegame_snapshot
	{
		scoped_savegame_snapshot(const play_controller& controller);
		~scoped_savegame_snapshot();
		const play_controller& controller_;
	};

	saved_game& get_saved_game() { return saved_game_; }

	statistics_t& statistics() { return *statistics_context_; }
	bool is_during_turn() const;
	bool is_linger_mode() const;

protected:
	friend struct scoped_savegame_snapshot;
	void play_slice_catch();
	bool have_keyboard_focus() override;
	void process_focus_keydown_event(const SDL_Event& event) override;
	void process_keydown_event(const SDL_Event& event) override;
	void process_keyup_event(const SDL_Event& event) override;

	void init_managers();
	/** preload events cannot be synced */
	void fire_preload();
	void fire_prestart();
	void fire_start();
	void start_game();
	virtual void init_gui();
	void finish_side_turn_events();
	void finish_turn(); //this should not throw an end turn or end level exception
	bool enemies_visible() const;

	void enter_textbox();
	void textbox_move_vertically(bool up);
	void tab();

public:
	/** returns 0 if no such team was found. */
	virtual int find_viewing_side() const = 0;
private:
	utils::ms_optimer timer_;

protected:
	//gamestate
	std::unique_ptr<game_state> gamestate_;
	config level_;
	saved_game& saved_game_;

	//managers
	tooltips::manager tooltips_manager_;

	//whiteboard manager
	std::shared_ptr<wb::manager> whiteboard_manager_;

	//plugins context
	std::unique_ptr<plugins_context> plugins_context_;

	//more managers
	std::unique_ptr<font::floating_label_context> labels_manager_;
	help::help_manager help_manager_;
	events::mouse_handler mouse_handler_;
	events::menu_handler menu_handler_;
	std::unique_ptr<hotkey_handler> hotkey_handler_;
	std::unique_ptr<soundsource::manager> soundsources_manager_;
	persist_manager persist_;

	//other objects
	std::unique_ptr<game_display> gui_;
	const std::unique_ptr<unit_experience_accelerator> xp_mod_;
	const std::unique_ptr<statistics_t> statistics_context_;
	actions::undo_list& undo_stack() { return *gamestate().undo_stack_; }
	const actions::undo_list& undo_stack() const { return *gamestate().undo_stack_; }
	std::unique_ptr<replay> replay_;

	bool skip_replay_;
	bool skip_story_;
	/**
	 * Whether we did init sides in this session
	 * (false = we did init sides before we reloaded the game).
	 */
	bool did_autosave_this_turn_;
	bool did_tod_sound_this_turn_;
	//the displayed location when we load a game.
	map_location map_start_;
	// Whether to start with the display faded to black
	bool start_faded_;

	const std::string& select_music(bool victory) const;

	void reset_gamestate(const config& level, int replay_pos);

private:

	void init(const config& level);

	/**
	 * This shows a warning dialog if either [scenario]next_scenario or any [endlevel]next_scenario would lead to an "Unknown Scenario" dialog.
	 */
	void check_next_scenario_is_known();

	std::vector<std::string> victory_music_;
	std::vector<std::string> defeat_music_;

	const hotkey::scope_changer scope_;

protected:
	mutable bool ignore_replay_errors_;
	/// true when the controller of the currently playing side has changed.
	/// this can mean for example:
	/// - The currently active side was reassigned from/to another player in a mp game
	/// - The replay controller was disabled ('continue play' button)
	/// - The currently active side was droided / undroided.
	/// - A side was set to idle.
	bool player_type_changed_;
	virtual void sync_end_turn() {}
	virtual void check_time_over();
	virtual void update_viewing_player() = 0;
};
