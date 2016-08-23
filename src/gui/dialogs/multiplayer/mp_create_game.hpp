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

#include "game_initialization/create_engine.hpp"
#include "game_initialization/configure_engine.hpp"
#include "mp_game_settings.hpp"
#include "scripting/plugins/context.hpp"

class config;

namespace gui2
{

class ttoggle_button;
class ttoggle_panel;
class ttree_view;
class twidget;

class tmp_create_game : public tdialog
{
	typedef std::pair<ng::level::TYPE, std::string> level_type_info;

public:
	tmp_create_game(const config& cfg, ng::create_engine& create_eng);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	const config& cfg_;

	struct option_source {
		std::string level_type;
		std::string id;
		friend bool operator<(const option_source& a, const option_source& b) {
			return a.level_type < b.level_type && a.id < b.id;
		}
	};

	using option_map = std::map<std::string, config::attribute_value>;

	std::vector<option_source> visible_options_;
	std::map<option_source, option_map> options_data_;

	void display_custom_options(twindow& window, ttree_view& options_tree, std::string&& type, const config& data);

	template<typename T>
	void update_options_data_map(T* widget, const option_source& source);
	void update_options_data_map(ttoggle_button* widget, const option_source& source);

	void reset_options_data(twindow& window, const option_source& source, bool& handled, bool& halt);

	ng::create_engine& create_engine_;
	std::unique_ptr<ng::configure_engine> config_engine_;

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

	void update_games_list(twindow& window);
	void display_games_of_type(twindow& window, ng::level::TYPE type, const std::string& level);

	void show_generator_settings(twindow& window);
	void regenerate_random_map(twindow& window);

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
	tfield_integer* init_turn_limit;
	tfield_integer* turn_bonus_;
	tfield_integer* reservior_;
	tfield_integer* action_bonus_;

	std::unique_ptr<plugins_context> plugins_context_;

	template<typename widget>
	void on_filter_change(twindow& window, const std::string& id);

	void on_game_select(twindow& window);
	void on_tab_select(twindow& window);
	void on_mod_select(twindow& window);
	void on_era_select(twindow& window);
	void on_mod_toggle(const int index);
	void on_random_faction_mode_select(twindow& window);

	void show_description(twindow& window, const std::string& new_description);

	void update_options_list(twindow& window);
	void update_details(twindow& window);
	void update_map_settings(twindow& window);

	void dialog_exit_hook(twindow& window);

	int convert_to_game_filtered_index(const int initial_index);

	void load_game_callback(twindow& window);

	enum tab { TAB_GENERAL, TAB_OPTIONS, TAB_SETTINGS };
};

} // namespace gui2

#endif
