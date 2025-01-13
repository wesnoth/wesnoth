/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"
#include "gui/dialogs/multiplayer/mp_options_helper.hpp"

#include "game_initialization/create_engine.hpp"
#include "game_initialization/configure_engine.hpp"


namespace gui2
{
class toggle_button;
class listbox;
class menu_button;

namespace dialogs
{

class mp_create_game : public modal_dialog, private plugin_executor
{
	typedef std::pair<level_type::type, std::string> level_type_info;

public:
	mp_create_game(saved_game& state, bool local_mode);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(mp_create_game);

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	ng::create_engine create_engine_;
	std::unique_ptr<ng::configure_engine> config_engine_;
	std::unique_ptr<mp_options_helper> options_manager_;

	int selected_game_index_;
	int selected_rfm_index_;

	std::vector<level_type_info> level_types_;

	void update_games_list();
	void display_games_of_type(level_type::type type, const std::string& level);

	void show_generator_settings();
	void regenerate_random_map();

	/**
	 * All fields are also in the normal field vector, but they need to be
	 * manually controlled as well so add the pointers here as well.
	 */

	field_bool* use_map_settings_;
	field_bool* fog_;
	field_bool* shroud_;
	field_bool* start_time_;
	field_bool* time_limit_;
	field_bool* shuffle_sides_;
	field_bool* observers_;
	field_bool* strict_sync_;
	field_bool* private_replay_;

	field_integer* turns_;
	field_integer* gold_;
	field_integer* support_;
	field_integer* experience_;
	field_integer* init_turn_limit_;
	field_integer* turn_bonus_;
	field_integer* reservoir_;
	field_integer* action_bonus_;

	listbox* mod_list_;
	menu_button* eras_menu_button_;

	bool local_mode_;

	template<typename widget>
	void on_filter_change(const std::string& id, bool do_select);

	void on_game_select();
	void on_tab_select();
	void on_era_select();
	void on_mod_toggle(const std::string& id, toggle_button* sender);
	void on_random_faction_mode_select();

	std::vector<std::string> get_active_mods();
	void set_active_mods(const std::vector<std::string>& val);

	void sync_with_depcheck();

	void show_description(const std::string& new_description);

	void update_details();
	void update_map_settings();

	void reset_timer_settings();

	/**
	 * Dialog exit hook to bring up the difficulty dialog when starting a campaign.
	 * This only fires when the retval is OK (ie, creating a game), meaning it does not fire
	 * when loading a saved game.
	 */
	bool dialog_exit_hook();

	int convert_to_game_filtered_index(const unsigned int initial_index);

	void load_game_callback();

	enum tab { TAB_GENERAL, TAB_OPTIONS, TAB_SETTINGS };
};

} // namespace dialogs
} // namespace gui2
