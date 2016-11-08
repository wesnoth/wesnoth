/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CREATE_GAME_HPP_INCLUDED
#define GUI_DIALOGS_MP_CREATE_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"
#include "gui/dialogs/multiplayer/mp_options_helper.hpp"

#include "game_initialization/create_engine.hpp"
#include "game_initialization/configure_engine.hpp"
#include "game_initialization/multiplayer.hpp"
#include "mp_game_settings.hpp"

class config;

namespace gui2
{

class toggle_button;
class toggle_panel;
class widget;

class tmp_create_game : public tdialog, private plugin_executor
{
	typedef std::pair<ng::level::TYPE, std::string> level_type_info;

public:
	tmp_create_game(const config& cfg, ng::create_engine& create_eng);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(window& window);

	/** Inherited from tdialog. */
	void post_show(window& window);

	const config& cfg_;

	ng::create_engine& create_engine_;
	std::unique_ptr<ng::configure_engine> config_engine_;
	std::unique_ptr<tmp_options_helper> options_manager_;

	int selected_game_index_;
	int selected_rfm_index_;

	std::vector<level_type_info> level_types_;

	/* We keep and work with a vector of the RFM types since it's the easiest way to get a value for the
	 * config_engine and preferences setters, since menu_buttons aren't supported by tfield. Even if they
	 * were, the above functions take a RANDOM_FACTION_MODE value, not an index. Even if we try to keep a
	 * copy of the selected RFM type index in a int value and update it every time you perform a selection,
	 * there's still the problem of getting an initial value from preferences, which again is provided as a
	 * RANDOM_FACTION_MODE value. Comparing strings between the (translated) menu_button values in the WML and
	 * the hardcoded (non-translated) RANDOM_FACTION_MODE string values stored in preferences is a horrible
	 * way to do it and would break in any language other than English. Instead, we'll keep a vector and use
	 * std::find to get the initial index. This method should also allow the values to eventually be translated,
	 * since the string values don't come into consideration at all, save for populating the menu_button.
	 *
	 * -- vultraz, 8/21/2016
	 */
	std::vector<mp_game_settings::RANDOM_FACTION_MODE> rfm_types_;

	void update_games_list(window& window);
	void display_games_of_type(window& window, ng::level::TYPE type, const std::string& level);

	void show_generator_settings(window& window);
	void regenerate_random_map(window& window);

	/**
	 * All fields are also in the normal field vector, but they need to be
	 * manually controlled as well so add the pointers here as well.
	 */

	tfield_bool* use_map_settings_;
	tfield_bool* fog_;
	tfield_bool* shroud_;
	tfield_bool* start_time_;
	tfield_bool* time_limit_;
	tfield_bool* shuffle_sides_;
	tfield_bool* observers_;
	tfield_bool* registered_users_;
	tfield_bool* strict_sync_;

	tfield_integer* turns_;
	tfield_integer* gold_;
	tfield_integer* support_;
	tfield_integer* experience_;
	tfield_integer* init_turn_limit_;
	tfield_integer* turn_bonus_;
	tfield_integer* reservoir_;
	tfield_integer* action_bonus_;

	template<typename widget>
	void on_filter_change(window& window, const std::string& id);

	void on_game_select(window& window);
	void on_tab_select(window& window);
	void on_mod_select(window& window);
	void on_era_select(window& window);
	void on_mod_toggle(const int index);
	void on_random_faction_mode_select(window& window);

	void show_description(window& window, const std::string& new_description);

	void update_details(window& window);
	void update_map_settings(window& window);

	/**
	 * Dialog exit hook to bring up the difficulty dialog when starting a campaign.
	 * This only fires when the retval is OK (ie, creating a game), meaning it does not fire
	 * when loading a saved game.
	 */
	bool dialog_exit_hook(window&);

	int convert_to_game_filtered_index(const int initial_index);

	void load_game_callback(window& window);

	enum tab { TAB_GENERAL, TAB_OPTIONS, TAB_SETTINGS };
};

} // namespace gui2

#endif
